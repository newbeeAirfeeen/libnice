/*
 * This file is part of the Nice GLib ICE library.
 *
 * (C) 2006-2009 Collabora Ltd.
 *  Contact: Youness Alaoui
 * (C) 2006-2009 Nokia Corporation. All rights reserved.
 *  Contact: Kai Vehmanen
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Nice GLib ICE library.
 *
 * The Initial Developers of the Original Code are Collabora Ltd and Nokia
 * Corporation. All Rights Reserved.
 *
 * Contributors:
 *   Dafydd Harries, Collabora Ltd.
 *   Youness Alaoui, Collabora Ltd.
 *   Kai Vehmanen, Nokia
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
 * case the provisions of LGPL are applicable instead of those above. If you
 * wish to allow use of your version of this file only under the terms of the
 * LGPL and not to allow others to use your version of this file under the
 * MPL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the MPL or the LGPL.
 */

#ifndef _NICE_AGENT_PRIV_H
#define _NICE_AGENT_PRIV_H

/* note: this is a private header part of agent.h */


#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#define NICEAPI_EXPORT
#endif

#include <glib.h>

#include "agent.h"

/**
 * NiceInputMessageIter:
 * @message: index of the message currently being written into
 * @buffer: index of the buffer currently being written into
 * @offset: byte offset into the buffer
 *
 * Iterator for sequentially writing into an array of #NiceInputMessages,
 * tracking the current write position (i.e. the index of the next byte to be
 * written).
 *
 * If @message is equal to the number of messages in the associated
 * #NiceInputMessage array, and @buffer and @offset are zero, the iterator is at
 * the end of the messages array, and the array is (presumably) full.
 *
 * Since: 0.1.5
 */
typedef struct {
    guint message;
    guint buffer;
    gsize offset;
} NiceInputMessageIter;

void nice_input_message_iter_reset(NiceInputMessageIter *iter);
gboolean
nice_input_message_iter_is_at_end(NiceInputMessageIter *iter,
                                  NiceInputMessage *messages, guint n_messages);
guint nice_input_message_iter_get_n_valid_messages(NiceInputMessageIter *iter);
gboolean
nice_input_message_iter_compare(const NiceInputMessageIter *a,
                                const NiceInputMessageIter *b);


#include "candidate.h"
#include "component.h"
#include "conncheck.h"
#include "random/random.h"
#include "socket/socket.h"
#include "stream.h"
#include "stun/stunagent.h"
#include "stun/usages/ice.h"
#include "stun/usages/turn.h"

#ifdef HAVE_GUPNP
#include <libgupnp-igd/gupnp-simple-igd-thread.h>
#endif

/* XXX: starting from ICE ID-18, Ta SHOULD now be set according
 *      to session bandwidth -> this is not yet implemented in NICE */

#define NICE_AGENT_TIMER_TA_DEFAULT 20                 /* timer Ta, msecs (impl. defined) */
#define NICE_AGENT_TIMER_TR_DEFAULT 25000              /* timer Tr, msecs (impl. defined) */
#define NICE_AGENT_TIMER_CONSENT_DEFAULT 5000          /* msec timer consent freshness connchecks (RFC 7675) */
#define NICE_AGENT_TIMER_CONSENT_TIMEOUT 10000         /* msec timer for consent checks to timeout and assume consent lost (RFC 7675) */
#define NICE_AGENT_TIMER_MIN_CONSENT_INTERVAL 4000     /* msec timer minimum for consent lost requests (RFC 7675) */
#define NICE_AGENT_TIMER_KEEPALIVE_TIMEOUT 50000       /* msec timer for keepalive (without consent checks) to timeout and assume conection lost */
#define NICE_AGENT_MAX_CONNECTIVITY_CHECKS_DEFAULT 100 /* see RFC 8445 6.1.2.5 */


/* An upper limit to size of STUN packets handled (based on Ethernet
 * MTU and estimated typical sizes of ICE STUN packet */
#define MAX_STUN_DATAGRAM_PAYLOAD 1300

#define NICE_COMPONENT_MAX_VALID_CANDIDATES 50 /* maximum number of validates remote candidates to keep, the number is arbitrary but hopefully large enough */

/* A convenient macro to test if the agent is compatible with RFC5245
 * or OC2007R2. Specifically these two modes share the support
 * of the regular or aggressive nomination mode */
#define NICE_AGENT_IS_COMPATIBLE_WITH_RFC5245_OR_OC2007R2(obj) \
    ((obj)->compatibility == NICE_COMPATIBILITY_RFC5245 ||     \
     (obj)->compatibility == NICE_COMPATIBILITY_OC2007R2)

struct _NiceAgent {
    GObject parent; /* gobject pointer */

    GMutex agent_mutex; /* Mutex used for thread-safe lib */

    gboolean full_mode;                 /* property: full-mode */
    gchar *stun_server_ip;              /* property: STUN server IP */
    guint stun_server_port;             /* property: STUN server port */
    gchar *proxy_ip;                    /* property: Proxy server IP */
    guint proxy_port;                   /* property: Proxy server port */
    NiceProxyType proxy_type;           /* property: Proxy type */
    gchar *proxy_username;              /* property: Proxy username */
    gchar *proxy_password;              /* property: Proxy password */
    GHashTable *proxy_extra_headers;    /* property: Proxy extra headers */
    gboolean saved_controlling_mode;    /* property: controlling-mode */
    guint timer_ta;                     /* property: timer Ta */
    guint max_conn_checks;              /* property: max connectivity checks */
    gboolean force_relay;               /* property: force relay */
    guint stun_max_retransmissions;     /* property: stun max retransmissions, Rc */
    guint stun_initial_timeout;         /* property: stun initial timeout, RTO */
    guint stun_reliable_timeout;        /* property: stun reliable timeout */
    NiceNominationMode nomination_mode; /* property: Nomination mode */
    gboolean support_renomination;      /* property: support RENOMINATION STUN attribute */
    guint idle_timeout;                 /* property: conncheck timeout before stop */

    GSList *local_addresses;         /* list of NiceAddresses for local
				     interfaces */
    GSList *streams;                 /* list of Stream objects */
    GSList *pruning_streams;         /* list of Streams current being shut down */
    GMainContext *main_context;      /* main context pointer */
    guint next_candidate_id;         /* id of next created candidate */
    guint next_stream_id;            /* id of next created candidate */
    NiceRNG *rng;                    /* random number generator */
    GSList *discovery_list;          /* list of CandidateDiscovery items */
    GSList *triggered_check_queue;   /* pairs in the triggered check list */
    guint discovery_unsched_items;   /* number of discovery items unscheduled */
    GSource *discovery_timer_source; /* source of discovery timer */
    GSource *conncheck_timer_source; /* source of conncheck timer */
    GSource *keepalive_timer_source; /* source of keepalive timer */
    GSList *refresh_list;            /* list of CandidateRefresh items */
    GSList *pruning_refreshes;       /* list of Refreshes current being shut down*/
    guint64 tie_breaker;             /* tie breaker (ICE sect 5.2
				     "Determining Role" ID-19) */
    NiceCompatibility compatibility; /* property: Compatibility mode */
    gboolean media_after_tick;       /* Received media after keepalive tick */
    gboolean upnp_enabled;           /* whether UPnP discovery is enabled */
#ifdef HAVE_GUPNP
    GUPnPSimpleIgdThread *upnp; /* GUPnP Single IGD agent */
    guint upnp_timeout;         /* UPnP discovery timeout */
#endif
    gchar *software_attribute;    /* SOFTWARE attribute */
    gboolean reliable;            /* property: reliable */
    gboolean bytestream_tcp;      /* property: bytestream-tcp */
    gboolean keepalive_conncheck; /* property: keepalive_conncheck */

    GQueue pending_signals;
    gboolean use_ice_udp;
    gboolean use_ice_tcp;
    gboolean use_ice_trickle;

    guint conncheck_ongoing_idle_delay; /* ongoing delay before timer stop */
    gboolean controlling_mode;          /* controlling mode used by the
                                         conncheck */
    gboolean consent_freshness;         /* rfc 7675 consent freshness with
                                         connchecks */
                                        /* XXX: add pointer to internal data struct for ABI-safe extensions */
};

gboolean
agent_find_component(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceStream **stream,
        NiceComponent **component) G_GNUC_WARN_UNUSED_RESULT;

NiceStream *agent_find_stream(NiceAgent *agent, guint stream_id);

void agent_gathering_done(NiceAgent *agent);
void agent_signal_gathering_done(NiceAgent *agent);

void agent_lock(NiceAgent *agent);
void agent_unlock(NiceAgent *agent);
void agent_unlock_and_emit(NiceAgent *agent);

void agent_signal_new_selected_pair(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceCandidate *lcandidate,
        NiceCandidate *rcandidate);

void agent_signal_component_state_change(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceComponentState state);

void agent_signal_new_candidate(
        NiceAgent *agent,
        NiceCandidate *candidate);

void agent_signal_new_remote_candidate(NiceAgent *agent, NiceCandidate *candidate);

void agent_signal_initial_binding_request_received(NiceAgent *agent, NiceStream *stream);

guint64 agent_candidate_pair_priority(NiceAgent *agent, NiceCandidate *local, NiceCandidate *remote);

NiceSocket *agent_create_tcp_turn_socket(NiceAgent *agent,
                                         NiceStream *stream, NiceComponent *component, NiceSocket *nicesock,
                                         NiceAddress *server, NiceRelayType type, gboolean reliable_tcp);

typedef gboolean (*NiceTimeoutLockedCallback)(NiceAgent *agent,
                                              gpointer user_data);
void agent_timeout_add_with_context(NiceAgent *agent, GSource **out,
                                    const gchar *name, guint interval, NiceTimeoutLockedCallback function,
                                    gpointer data);
void agent_timeout_add_seconds_with_context(NiceAgent *agent, GSource **out,
                                            const gchar *name, guint interval, NiceTimeoutLockedCallback function,
                                            gpointer data);

StunUsageIceCompatibility agent_to_ice_compatibility(NiceAgent *agent);
StunUsageTurnCompatibility agent_to_turn_compatibility(NiceAgent *agent);
NiceTurnSocketCompatibility agent_to_turn_socket_compatibility(NiceAgent *agent);

void agent_remove_local_candidate(NiceAgent *agent, NiceStream *stream,
                                  NiceCandidate *candidate);

void nice_agent_init_stun_agent(NiceAgent *agent, StunAgent *stun_agent);

void _priv_set_socket_tos(NiceAgent *agent, NiceSocket *sock, gint tos);

void _tcp_sock_is_writable(NiceSocket *sock, gpointer user_data);

gboolean
component_io_cb(
        GSocket *gsocket,
        GIOCondition condition,
        gpointer data);

gsize memcpy_buffer_to_input_message(NiceInputMessage *message,
                                     const guint8 *buffer, gsize buffer_length);
guint8 *
compact_input_message(const NiceInputMessage *message, gsize *buffer_length);

guint8 *
compact_output_message(const NiceOutputMessage *message, gsize *buffer_length);

gsize output_message_get_size(const NiceOutputMessage *message);

gsize input_message_get_size(const NiceInputMessage *message);

gssize agent_socket_send(NiceSocket *sock, const NiceAddress *addr, gsize len,
                         const gchar *buf);

guint32
nice_candidate_jingle_priority(NiceCandidate *candidate);

guint32
nice_candidate_msn_priority(NiceCandidate *candidate);

guint32
nice_candidate_ice_priority_full(guint type_pref, guint local_pref,
                                 guint component_id);

guint32
nice_candidate_ice_priority(const NiceCandidate *candidate,
                            gboolean reliable, gboolean nat_assisted);

guint32
nice_candidate_ms_ice_priority(const NiceCandidate *candidate,
                               gboolean reliable, gboolean nat_assisted);

guint64
nice_candidate_pair_priority(guint32 o_prio, guint32 a_prio);

#define NICE_CANDIDATE_PAIR_PRIORITY_MAX_SIZE 32

void nice_candidate_pair_priority_to_string(guint64 prio, gchar *string);

/*
 * nice_debug_init:
 *
 * Initialize the debugging system. Uses the NICE_DEBUG environment variable
 * to set the appropriate debugging flags
 */
void nice_debug_init(void);


#ifdef NDEBUG
static inline gboolean nice_debug_is_enabled(void) { return FALSE; }
static inline gboolean nice_debug_is_verbose(void) { return FALSE; }
static inline void nice_debug(const char *fmt, ...) {}
static inline void nice_debug_verbose(const char *fmt, ...) {}
#else
gboolean nice_debug_is_enabled(void);
gboolean nice_debug_is_verbose(void);
void nice_debug(const char *fmt, ...) G_GNUC_PRINTF(1, 2);
void nice_debug_verbose(const char *fmt, ...) G_GNUC_PRINTF(1, 2);
#endif

#if !GLIB_CHECK_VERSION(2, 59, 0)
#if __GNUC__ > 6
#define G_GNUC_FALLTHROUGH __attribute__((fallthrough))
#else
#define G_GNUC_FALLTHROUGH
#endif /* __GNUC__ */
#endif

#endif /*_NICE_AGENT_PRIV_H */
