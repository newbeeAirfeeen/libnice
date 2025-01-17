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

#ifndef _NICE_CONNCHECK_H
#define _NICE_CONNCHECK_H

/* note: this is a private header to libnice */

#include "agent.h"
#include "stream.h"
#include "stun/stunagent.h"
#include "stun/usages/timer.h"

#define NICE_CANDIDATE_PAIR_MAX_FOUNDATION NICE_CANDIDATE_MAX_FOUNDATION * 2

/* A helper macro to test whether connection checks should continue to be
 * performed after a component has successfully connected */
#define NICE_AGENT_DO_KEEPALIVE_CONNCHECKS(obj) \
    ((obj)->consent_freshness || (obj)->keepalive_conncheck || (obj)->compatibility == NICE_COMPATIBILITY_GOOGLE)

/**
 * NiceCheckState:
 * @NICE_CHECK_WAITING: Waiting to be scheduled.
 * @NICE_CHECK_IN_PROGRESS: Connection checks started.
 * @NICE_CHECK_SUCCEEDED: Connection successfully checked.
 * @NICE_CHECK_FAILED: No connectivity; retransmissions ceased.
 * @NICE_CHECK_FROZEN: Waiting to be scheduled to %NICE_CHECK_WAITING.
 * @NICE_CHECK_DISCOVERED: A valid candidate pair not on the check list.
 *
 * States for checking a candidate pair.
 */
typedef enum {
    NICE_CHECK_WAITING = 1,
    NICE_CHECK_IN_PROGRESS,
    NICE_CHECK_SUCCEEDED,
    NICE_CHECK_FAILED,
    NICE_CHECK_FROZEN,
    NICE_CHECK_DISCOVERED,
} NiceCheckState;

typedef struct _CandidateCheckPair CandidateCheckPair;
typedef struct _StunTransaction StunTransaction;

struct _StunTransaction {
    gint64 next_tick; /* next tick timestamp */
    StunTimer timer;
    uint8_t buffer[STUN_MAX_MESSAGE_SIZE_IPV6];
    StunMessage message;
};

struct _CandidateCheckPair {
    guint stream_id;
    guint component_id;
    NiceCandidate *local;
    NiceCandidate *remote;
    struct _NiceSocket *sockptr;
    gchar foundation[NICE_CANDIDATE_PAIR_MAX_FOUNDATION];
    NiceCheckState state;
    gboolean nominated;
    gboolean valid;
    gboolean use_candidate_on_next_check;
    gboolean mark_nominated_on_response_arrival;
    gboolean retransmit; /* if the first stun request must be retransmitted */
    CandidateCheckPair *discovered_pair;
    CandidateCheckPair *succeeded_pair;
    guint64 priority;
    guint32 stun_priority;
    GSList *stun_transactions; /* a list of ongoing stun requests */
};

int conn_check_add_for_candidate(NiceAgent *agent, guint stream_id, NiceComponent *component, NiceCandidate *remote);
int conn_check_add_for_local_candidate(NiceAgent *agent, guint stream_id, NiceComponent *component, NiceCandidate *local);
gboolean conn_check_add_for_candidate_pair(NiceAgent *agent, guint stream_id, NiceComponent *component, NiceCandidate *local, NiceCandidate *remote);
void conn_check_free(NiceAgent *agent);
int conn_check_send(NiceAgent *agent, CandidateCheckPair *pair);
void conn_check_prune_stream(NiceAgent *agent, NiceStream *stream);
gboolean conn_check_handle_inbound_stun(NiceAgent *agent, NiceStream *stream, NiceComponent *component, struct _NiceSocket *udp_socket, const NiceAddress *from, gchar *buf, guint len);
gint conn_check_compare(const CandidateCheckPair *a, const CandidateCheckPair *b);
void conn_check_remote_candidates_set(NiceAgent *agent, NiceStream *stream, NiceComponent *component);
void conn_check_remote_credentials_set(NiceAgent *agent, NiceStream *stream);
NiceCandidateTransport conn_check_match_transport(NiceCandidateTransport transport);
void conn_check_prune_socket(NiceAgent *agent, NiceStream *stream, NiceComponent *component,
                             struct _NiceSocket *sock);

void recalculate_pair_priorities(NiceAgent *agent);
void conn_check_update_selected_pair(NiceAgent *agent,
                                     NiceComponent *component, CandidateCheckPair *pair);
void conn_check_update_check_list_state_for_ready(NiceAgent *agent,
                                                  NiceStream *stream, NiceComponent *component);
void conn_check_unfreeze_related(NiceAgent *agent, CandidateCheckPair *pair);
guint conn_check_stun_transactions_count(NiceAgent *agent);


#endif /*_NICE_CONNCHECK_H */
