/*
 * This file is part of the Nice GLib ICE library.
 *
 * (C) 2008-2009 Collabora Ltd.
 *  Contact: Youness Alaoui
 * (C) 2007-2009 Nokia Corporation. All rights reserved.
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

#ifndef _NICE_DISCOVERY_H
#define _NICE_DISCOVERY_H

/* note: this is a private header to libnice */

#include "agent.h"
#include "candidate-priv.h"
#include "stream.h"

typedef struct
{
    NiceCandidateType type; /* candidate type STUN or TURN */
    NiceSocket *nicesock;   /* XXX: should be taken from local cand: existing socket to use */
    NiceAddress server;     /* STUN/TURN server address */
    gint64 next_tick;       /* next tick timestamp */
    gboolean pending;       /* is discovery in progress? */
    gboolean done;          /* is discovery complete? */
    guint stream_id;
    guint component_id;
    TurnServer *turn;
    StunAgent stun_agent;
    StunTimer timer;
    uint8_t stun_buffer[STUN_MAX_MESSAGE_SIZE_IPV6];
    StunMessage stun_message;
    uint8_t stun_resp_buffer[STUN_MAX_MESSAGE_SIZE];
    StunMessage stun_resp_msg;
} CandidateDiscovery;

typedef struct
{
    NiceSocket *nicesock;         /* existing socket to use */
    NiceAddress server;           /* STUN/TURN server address */
    NiceCandidateImpl *candidate; /* candidate to refresh */
    guint stream_id;
    guint component_id;
    StunAgent stun_agent;
    GSource *timer_source;
    GSource *tick_source;
    StunTimer timer;
    uint8_t stun_buffer[STUN_MAX_MESSAGE_SIZE_IPV6];
    StunMessage stun_message;
    uint8_t stun_resp_buffer[STUN_MAX_MESSAGE_SIZE];
    StunMessage stun_resp_msg;

    gboolean disposing;
    GDestroyNotify destroy_cb;
    gpointer destroy_cb_data;
    GSource *destroy_source;
} CandidateRefresh;

void refresh_free(NiceAgent *agent, CandidateRefresh *refresh);
void refresh_prune_agent_async(NiceAgent *agent,
                               NiceTimeoutLockedCallback function, gpointer user_data);
void refresh_prune_stream_async(NiceAgent *agent, NiceStream *stream,
                                NiceTimeoutLockedCallback function);
void refresh_prune_candidate(NiceAgent *agent, NiceCandidateImpl *candidate);
void refresh_prune_candidate_async(NiceAgent *agent, NiceCandidateImpl *cand,
                                   NiceTimeoutLockedCallback function);
void refresh_prune_socket(NiceAgent *agent, NiceSocket *nicesock);


void discovery_free(NiceAgent *agent);
void discovery_prune_stream(NiceAgent *agent, guint stream_id);
void discovery_prune_socket(NiceAgent *agent, NiceSocket *sock);
void discovery_schedule(NiceAgent *agent);

typedef enum {
    HOST_CANDIDATE_SUCCESS,
    HOST_CANDIDATE_FAILED,
    HOST_CANDIDATE_CANT_CREATE_SOCKET,
    HOST_CANDIDATE_REDUNDANT,
    HOST_CANDIDATE_DUPLICATE_PORT
} HostCandidateResult;

HostCandidateResult
discovery_add_local_host_candidate(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceAddress *address,
        NiceCandidateTransport transport,
        gboolean accept_duplicate,
        NiceCandidateImpl **candidate);

NiceCandidateImpl *
discovery_add_relay_candidate(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceAddress *address,
        NiceCandidateTransport transport,
        NiceSocket *base_socket,
        TurnServer *turn,
        uint32_t *lifetime);

void discovery_add_server_reflexive_candidate(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceAddress *address,
        NiceCandidateTransport transport,
        NiceSocket *base_socket,
        const NiceAddress *server_address,
        gboolean nat_assisted);

void discovery_discover_tcp_server_reflexive_candidates(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        NiceAddress *address,
        NiceSocket *base_socket,
        const NiceAddress *server_address);

NiceCandidate *
discovery_add_peer_reflexive_candidate(
        NiceAgent *agent,
        guint stream_id,
        guint component_id,
        guint32 priority,
        NiceAddress *address,
        NiceSocket *base_socket,
        NiceCandidate *local,
        NiceCandidate *remote);

NiceCandidate *
discovery_learn_remote_peer_reflexive_candidate(
        NiceAgent *agent,
        NiceStream *stream,
        NiceComponent *component,
        guint32 priority,
        const NiceAddress *remote_address,
        NiceSocket *udp_socket,
        NiceCandidate *local,
        NiceCandidate *remote);

#endif /*_NICE_CONNCHECK_H */
