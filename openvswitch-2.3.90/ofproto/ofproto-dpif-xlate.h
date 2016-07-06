/* Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#ifndef OFPROTO_DPIF_XLATE_H
#define OFPROTO_DPIF_XLATE_H 1

#include "dp-packet.h"
#include "flow.h"
#include "meta-flow.h"
#include "odp-util.h"
#include "ofpbuf.h"
#include "ofproto-dpif-mirror.h"
#include "ofproto-dpif.h"
#include "ofproto.h"
#include "stp.h"
#include "ovs-lldp.h"

struct bfd;
struct bond;
struct dpif;
struct lacp;
struct dpif_ipfix;
struct dpif_sflow;
struct mac_learning;
struct mcast_snooping;
struct xlate_cache;

struct xlate_recirc {
    uint32_t recirc_id;  /* !0 Use recirculation instead of output. */
    uint8_t  hash_alg;   /* !0 Compute hash for recirc before. */
    uint32_t hash_basis;  /* Compute hash for recirc before. */
};

struct xlate_out {
    /* Wildcards relevant in translation.  Any fields that were used to
     * calculate the action must be set for caching and kernel
     * wildcarding to work.  For example, if the flow lookup involved
     * performing the "normal" action on IPv4 and ARP packets, 'wc'
     * would have the 'in_port' (always set), 'dl_type' (flow match),
     * 'vlan_tci' (normal action), and 'dl_dst' (normal action) fields
     * set. */
    struct flow_wildcards wc;

    enum slow_path_reason slow; /* 0 if fast path may be used. */
    bool fail_open;             /* Initial rule is fail open? */
    bool has_learn;             /* Actions include NXAST_LEARN? */
    bool has_normal;            /* Actions output to OFPP_NORMAL? */
    bool has_fin_timeout;       /* Actions include NXAST_FIN_TIMEOUT? */
    ofp_port_t nf_output_iface; /* Output interface index for NetFlow. */
    mirror_mask_t mirrors;      /* Bitmap of associated mirrors. */

    uint64_t odp_actions_stub[256 / 8];
    struct ofpbuf odp_actions_buf;
    struct ofpbuf *odp_actions;
};

struct xlate_in {
    struct ofproto_dpif *ofproto;

    /* Flow to which the OpenFlow actions apply.  xlate_actions() will modify
     * this flow when actions change header fields. */
    struct flow flow;

    /* The packet corresponding to 'flow', or a null pointer if we are
     * revalidating without a packet to refer to. */
    const struct dp_packet *packet;

    /* Should OFPP_NORMAL update the MAC learning table?  Should "learn"
     * actions update the flow table?
     *
     * We want to update these tables if we are actually processing a packet,
     * or if we are accounting for packets that the datapath has processed, but
     * not if we are just revalidating. */
    bool may_learn;

    /* If the caller of xlate_actions() doesn't need the flow_wildcards
     * contained in struct xlate_out.  'skip_wildcards' can be set to true
     * disabling the expensive wildcard computation.  When true, 'wc' in struct
     * xlate_out is undefined and should not be read. */
    bool skip_wildcards;

    /* The rule initiating translation or NULL. If both 'rule' and 'ofpacts'
     * are NULL, xlate_actions() will do the initial rule lookup itself. */
    struct rule_dpif *rule;

    /* The actions to translate.  If 'rule' is not NULL, these may be NULL. */
    const struct ofpact *ofpacts;
    size_t ofpacts_len;

    /* Union of the set of TCP flags seen so far in this flow.  (Used only by
     * NXAST_FIN_TIMEOUT.  Set to zero to avoid updating updating rules'
     * timeouts.) */
    uint16_t tcp_flags;

    /* If nonnull, flow translation calls this function just before executing a
     * resubmit or OFPP_TABLE action.  In addition, disables logging of traces
     * when the recursion depth is exceeded.
     *
     * 'rule' is the rule being submitted into.  It will be null if the
     * resubmit or OFPP_TABLE action didn't find a matching rule.
     *
     * 'recurse' is the resubmit recursion depth at time of invocation.
     *
     * This is normally null so the client has to set it manually after
     * calling xlate_in_init(). */
    void (*resubmit_hook)(struct xlate_in *, struct rule_dpif *rule,
                          int recurse);

    /* If nonnull, flow translation calls this function to report some
     * significant decision, e.g. to explain why OFPP_NORMAL translation
     * dropped a packet.  'recurse' is the resubmit recursion depth at time of
     * invocation. */
    void (*report_hook)(struct xlate_in *, const char *s, int recurse);

    /* If nonnull, flow translation credits the specified statistics to each
     * rule reached through a resubmit or OFPP_TABLE action.
     *
     * This is normally null so the client has to set it manually after
     * calling xlate_in_init(). */
    const struct dpif_flow_stats *resubmit_stats;

    /* If nonnull, flow translation populates this cache with references to all
     * modules that are affected by translation. This 'xlate_cache' may be
     * passed to xlate_push_stats() to perform the same function as
     * xlate_actions() without the full cost of translation.
     *
     * This is normally null so the client has to set it manually after
     * calling xlate_in_init(). */
    struct xlate_cache *xcache;

    /* Allows callers to optionally supply their own buffer for the resulting
     * odp_actions stored in xlate_out.  If NULL, the default buffer will be
     * used. */
    struct ofpbuf *odp_actions;
};


struct xlate_ctx {
    struct xlate_in *xin;
    struct xlate_out *xout;

    const struct xbridge *xbridge;

    /* Flow at the last commit. */
    struct flow base_flow;

    /* Tunnel IP destination address as received.  This is stored separately
     * as the base_flow.tunnel is cleared on init to reflect the datapath
     * behavior.  Used to make sure not to send tunneled output to ourselves,
     * which might lead to an infinite loop.  This could happen easily
     * if a tunnel is marked as 'ip_remote=flow', and the flow does not
     * actually set the tun_dst field. */
    ovs_be32 orig_tunnel_ip_dst;

    /* Stack for the push and pop actions.  Each stack element is of type
     * "union mf_subvalue". */
    union mf_subvalue init_stack[1024 / sizeof(union mf_subvalue)];
    struct ofpbuf stack;

    /* The rule that we are currently translating, or NULL. */
    struct rule_dpif *rule;

    /* Resubmit statistics, via xlate_table_action(). */
    int recurse;                /* Current resubmit nesting depth. */
    int resubmits;              /* Total number of resubmits. */
    bool in_group;              /* Currently translating ofgroup, if true. */
    bool in_action_set;         /* Currently translating action_set, if true. */

    uint8_t table_id;           /* OpenFlow table ID where flow was found. */
    ovs_be64 rule_cookie;       /* Cookie of the rule being translated. */
    uint32_t orig_skb_priority; /* Priority when packet arrived. */
    uint32_t sflow_n_outputs;   /* Number of output ports. */
    odp_port_t sflow_odp_port;  /* Output port for composing sFlow action. */
    uint16_t user_cookie_offset;/* Used for user_action_cookie fixup. */
    bool exit;                  /* No further actions should be processed. */

    bool use_recirc;            /* Should generate recirc? */
    struct xlate_recirc recirc; /* Information used for generating
                                 * recirculation actions */

    /* True if a packet was but is no longer MPLS (due to an MPLS pop action).
     * This is a trigger for recirculation in cases where translating an action
     * or looking up a flow requires access to the fields of the packet after
     * the MPLS label stack that was originally present. */
    bool was_mpls;

    /* OpenFlow 1.1+ action set.
     *
     * 'action_set' accumulates "struct ofpact"s added by OFPACT_WRITE_ACTIONS.
     * When translation is otherwise complete, ofpacts_execute_action_set()
     * converts it to a set of "struct ofpact"s that can be translated into
     * datapath actions. */
    bool action_set_has_group;  /* Action set contains OFPACT_GROUP? */
    struct ofpbuf action_set;   /* Action set. */
    uint64_t action_set_stub[1024 / 8];
};


void xlate_ofproto_set(struct ofproto_dpif *, const char *name, struct dpif *,
                       const struct mac_learning *, struct stp *,
                       struct rstp *, const struct mcast_snooping *,
                       const struct mbridge *, const struct dpif_sflow *,
                       const struct dpif_ipfix *, const struct netflow *,
                       bool forward_bpdu,
                       bool has_in_band, bool enable_recirc,
                       bool variable_length_userdata,
                       size_t mpls_label_stack_length,
                       bool masked_set_action);
void xlate_remove_ofproto(struct ofproto_dpif *);

void xlate_bundle_set(struct ofproto_dpif *, struct ofbundle *,
                      const char *name, enum port_vlan_mode, int vlan,
                      unsigned long *trunks, bool use_priority_tags,
                      const struct bond *, const struct lacp *,
                      bool floodable);
void xlate_bundle_remove(struct ofbundle *);

void xlate_ofport_set(struct ofproto_dpif *, struct ofbundle *,
                      struct ofport_dpif *, ofp_port_t, odp_port_t,
                      const struct netdev *, const struct cfm *, const struct bfd *,
                      const struct lldp *, struct ofport_dpif *peer,
                      int stp_port_no, const struct rstp_port *rstp_port,
                      const struct ofproto_port_queue *qdscp,
                      size_t n_qdscp, enum ofputil_port_config,
                      enum ofputil_port_state, bool is_tunnel,
                      bool may_enable);
void xlate_ofport_remove(struct ofport_dpif *);

struct ofproto_dpif * xlate_lookup_ofproto(const struct dpif_backer *,
                                           const struct flow *,
                                           ofp_port_t *ofp_in_port);
int xlate_lookup(const struct dpif_backer *, const struct flow *,
                 struct ofproto_dpif **, struct dpif_ipfix **,
                 struct dpif_sflow **, struct netflow **,
                 ofp_port_t *ofp_in_port);

void xlate_actions(struct xlate_in *, struct xlate_out *);
void xlate_in_init(struct xlate_in *, struct ofproto_dpif *,
                   const struct flow *, ofp_port_t in_port, struct rule_dpif *,
                   uint16_t tcp_flags, const struct dp_packet *packet);
void xlate_out_uninit(struct xlate_out *);
void xlate_actions_for_side_effects(struct xlate_in *);
void xlate_out_copy(struct xlate_out *dst, const struct xlate_out *src);

int xlate_send_packet(const struct ofport_dpif *, struct dp_packet *);

struct xlate_cache *xlate_cache_new(void);
void xlate_push_stats(struct xlate_cache *, const struct dpif_flow_stats *);
void xlate_cache_clear(struct xlate_cache *);
void xlate_cache_delete(struct xlate_cache *);
void xlate_txn_start(void);
void xlate_txn_commit(void);

void compose_output_action(struct xlate_ctx *, ofp_port_t ofp_port);

#endif /* ofproto-dpif-xlate.h */
