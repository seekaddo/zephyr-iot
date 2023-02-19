// SPDX-License-Identifier: BSD-2-Clause
//
// Copyright (c) 2014-2022, NetApp, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#ifdef NDEBUG
#undef NDEBUG
#endif
//#include <netinet/in.h>
//#include <stdint.h>
//#include <sys/param.h>
//#include <sys/time.h>

#define klib_unused
#undef RIOT_VERSION
#include "configw.h" // IWYU pragma: export
#include "khash.h" // IWYU pragma: export
#include "plat.h" // IWYU pragma: export
#include "queue.h" // IWYU pragma: export
#include "util.h"// IWYU pragma: export

#if !defined(PARTICLE) && !defined(RIOT_VERSION)
//#include <ifaddrs.h>
//#include <net/if.h>
//#include <sys/socket.h>
#endif

#ifdef RIOT_VERSION
#include "net/netif.h"
#include "net/sock/udp.h"
#endif

#ifdef CONTIKI_NG_LE
//#include "net/ipv6/uiplib.h"
//#include "net/app-layer/quic-en/quic-endpoint.h"
#include <net/net_pkt.h>
#include <net/net_if.h>
#include <net/socket.h>
#include <posix/time.h>
#include <posix/sys/time.h>
#include <zephyr/types.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 5 // zephyr only
#endif
#endif


/// A tail queue of w_iov I/O vectors. Also contains a counter that (on TX)
/// tracks how many w_iovs have not yet been transmitted by the NIC.
///
sq_head(w_iov_sq, w_iov); ///< Head of the w_iov tail queue.


/// Initializer for struct w_iov_sq.
///
/// @param      q     A struct w_iov_sq.
///
/// @return     Empty w_iov_sq, to be assigned to @p q.
///
#define w_iov_sq_initializer(q) sq_head_initializer(q)


#define IP6_LEN 16 ///< Length of an IPv4 address in bytes. Sixteen.
#define IP6_STRLEN 40 //INET6_ADDRSTRLEN

/// Initializer for temporary string for holding an IPv6 address.
#define ip6_tmp                                                                \
    (char[IP6_STRLEN])                                                         \
    {                                                                          \
        ""                                                                     \
    }

#define IP4_LEN 4 ///< Length of an IPv4 address in bytes. Four.
#define IP4_STRLEN 16 //INET_ADDRSTRLEN

/// Initializer for temporary string for holding an IPv4 address.
#define ip4_tmp                                                                \
    (char[IP4_STRLEN])                                                         \
    {                                                                          \
        ""                                                                     \
    }

#define IP_STRLEN MAX(IP4_STRLEN, IP6_STRLEN)

/// Initializer for temporary string for holding an IPv4 or IPv6 address.
#define ip_tmp                                                                 \
    (char[IP_STRLEN])                                                          \
    {                                                                          \
        ""                                                                     \
    }


/// Return the length of the IP address of address family @p af.
///
/// @param      af    AF_INET or AF_INET6.
///
/// @return     Length of an IP address of the given family.
///
#define af_len(af) (uint8_t)((af) == AF_INET ? IP4_LEN : IP6_LEN)


/// Return the length of an IP header (without options) of address family @p af.
///
/// @param      af    AF_INET or AF_INET6.
///
/// @return     Length of an IP header of the given family.
///
#define ip_hdr_len(af) (uint8_t)((af) == 2/*AF_INET*/ ? 20 : 40)


/// Compare two IPv6 addresses for equality.
///
/// @param      a     An IPv6 address.
/// @param      b     Another IPv6 address.
///
/// @return     True if equal, false otherwise.
///
#define ip6_eql(a, b) (memcmp((a), (b), IP6_LEN) == 0)


struct w_addr {
    sa_family_t af; ///< Address family.
    union {
        uint32_t ip4;         ///< IPv4 address (when w_addr::af is AF_INET).
        uint8_t ip6[IP6_LEN]; ///< IPv6 address (when w_addr::af is AF_INET6).
    };
};


struct w_ifaddr {
    struct w_addr addr; ///< Interface address.

    union {
        /// IPv4 broadcast address (when w_ifaddr::addr::af is AF_INET).
        uint32_t bcast4;

        /// IPv6 broadcast address (when w_ifaddr::addr::af is AF_INET6).
        uint8_t bcast6[IP6_LEN];
    };

    union {
        /// IPv4 solicited-node multicast address (when w_ifaddr::addr::af is
        /// AF_INET). (Not currently in use.)
        uint32_t snma4;

        /// IPv6 solicited-node multicast address (when w_ifaddr::addr::af is
        /// AF_INET6).
        uint8_t snma6[IP6_LEN];
    };

    uint32_t scope_id; ///< IPv6 scope ID.
    uint8_t prefix;    ///< Prefix length.
};


struct w_sockaddr {
    struct w_addr addr; ///< IP address.
    uint16_t port;      ///< Port number.
};


struct w_socktuple {
    //struct w_sockaddr local;  ///< Local address and port. NOADD
    //struct w_sockaddr remote; ///< Remote address and port. NOADD
#ifdef CONTIKI_NG_LE
    struct uip_udp_conn *locdp; // current udp_connection
    struct sockaddr *remt; // endpoint to send
#endif
    uint32_t scope_id;        ///< IPv6 scope ID.
};

char *wi_ntop(const struct sockaddr *addr);


//extern khint_t __attribute__((nonnull))
//w_socktuple_hash(const struct w_socktuple * const tup);

#if 0
extern khint_t __attribute__((nonnull))
w_socktuple_cmp(const struct w_socktuple * const a,
                const struct w_socktuple * const b);
#endif

#define ETH_LEN 6
#define ETH_STRLEN (ETH_LEN * 3 + 1)

#define eth_tmp                                                                \
    (char[ETH_STRLEN])                                                         \
    {                                                                          \
        ""                                                                     \
    }

#if 0
struct eth_addr {
    uint8_t addr[ETH_LEN];
};
#endif

#ifdef RIOT_VERSION
#ifdef CONFIG_NETIF_NAMELENMAX
#define IFNAMSIZ CONFIG_NETIF_NAMELENMAX
#else
#define IFNAMSIZ NETIF_NAMELENMAX
#endif
#endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 48
#endif

struct w_socktup
{
  struct sockaddr laddr;
  struct sockaddr raddr;
  uint16_t lport;
  uint16_t rport;
  uint16_t af;
  uint16_t scope_id;
};

struct qmcon {

  struct w_engine *w;
  const char *  req;
  const char *  peer;
  struct q_conn *  c;
  struct q_stream * s ;
  struct sockaddr  p_addr;
  uint16_t qrcnt;
  //uint16_t clean;
  //quic_udp_callback cl_callback;
};

struct net_rec {
  struct sockaddr src_addr;
  //uint16_t src_port;
  struct net_context *cntx;
  struct net_pkt *pkt;
  void *user_data;
};

/// A warpcore backend engine.
///
struct w_engine {
    void * mem;           ///< Pointer to netmap or socket buffer memory region.
    struct w_iov * bufs;  ///< Pointer to w_iov buffers.
    struct w_backend * b; ///< Backend.
    uint16_t mtu;         ///< MTU of this interface.
    uint32_t mbps;        ///< Link speed of this interface in Mb/s.
    //struct eth_addr mac;  ///< Local Ethernet MAC address of the interface.
    // struct eth_addr rip;  ///< Ethernet MAC address of the next-hop router.

    struct w_iov_sq iov; ///< Tail queue of w_iov buffers available.

    sl_entry(w_engine) next;      ///< Pointer to next engine.
    char ifname[IFNAMSIZ];        ///< Name of the interface of this engine.
    char drvname[IFNAMSIZ];       ///< Name of the driver of this interface.
    const char * backend_name;    ///< Name of the backend in @p b.
    const char * backend_variant; ///< Name of the backend variant in @p b.
    //struct net_context *u6_contxt; // udp context used for receiving and sending net pkt
    struct net_rec u6_rec;

    /// Pointer to generic user data (not used by warpcore.)
    void * data;

    uint16_t addr_cnt;
    uint16_t addr4_pos;
    uint8_t have_ip4 : 1;
    uint8_t have_ip6 : 1;
    uint8_t is_loopback : 1;
    uint8_t is_right_pipe : 1;
    uint8_t : 4;
    struct w_ifaddr ifaddr[];
};


/// A chain of w_sock socket.
///
sl_head(w_sock_slist, w_sock);


/// Initializer for struct w_sock_slist.
///
/// @param      l     A struct w_sock_slist.
///
/// @return     Empty w_sock_slist, to be assigned to @p l.
///
#define w_sock_slist_initializer(l) sl_head_initializer(l)


/// Socket options.
///
struct w_sockopt {
    /// Do not compute a UDP checksum for outgoing packets.
    uint32_t enable_udp_zero_checksums : 1;
    /// Enable ECN, by setting ECT(0) on all packets.
    uint32_t enable_ecn : 1;
    uint32_t : 27;
    uint32_t user_1 : 1; ///< User flag 1 (not used by warpcore.)
    uint32_t user_2 : 1; ///< User flag 2 (not used by warpcore.)
    uint32_t user_3 : 1; ///< User flag 3 (not used by warpcore.)
};


/// A warpcore socket.
///
struct w_sock {
    /// Pointer back to the warpcore instance associated with this w_sock.
    struct w_engine * w;

    /// Pointer to generic user data (not used by warpcore.)
    void * data;

    //struct w_socktuple tup; ///< Socket four-tuple.
#ifdef CONTIKI_NG_LE
    struct w_socktup itup; // current udp_connection
#endif
    uint16_t af_tp;
    //struct eth_addr dmac;   ///< Destination MAC address.  NOADD
    struct w_sockopt opt;   ///< Socket options.
    intptr_t fd;            ///< Socket descriptor underlying the engine.  NOADD
    struct w_iov_sq iv;     ///< Tail queue containing incoming unread data.

    sl_entry(w_sock) next; ///< Next socket.

#if !defined(HAVE_KQUEUE) && !defined(HAVE_EPOLL)
    sl_entry(w_sock) __next; ///< Internal use.
#endif
};


#define ws_af af_tp
//#define ws_loc tup.local
#define ws_laddr itup.laddr
#define ws_lport itup.laddr
#define ws_laf   ws_af
//#define ws_rem tup.remote
#define ws_raddr itup.raddr
#define ws_rport itup.raddr
#define ws_str(ad) net_sin6(&ad)->sin6_port
#define ws_scope itup.scope_id

uint16_t backend_addr_cnt(void);

/// The I/O vector structure that warpcore uses at the center of its API. It is
/// mostly a pointer to the first UDP payload byte contained in a netmap packet
/// buffer, together with some associated meta data.
///
/// The meta data consists of the length of the payload data, the sender IP
/// address and port number, the DSCP and ECN bits associated with the IP
/// packet in which the payload arrived, and the netmap arrival timestamp.
///
/// The w_iov structure also contains a pointer to the next I/O vector, which
/// can be used to chain together longer data items for use with w_rx() and
/// w_tx().
///
struct w_iov {
    /// Pointer back to the warpcore instance associated with this w_iov.
    struct w_engine * w;

    /// Sender IP address and port on RX. Destination IP address and port on TX
    /// on a disconnected w_sock. Ignored on TX on a connected w_sock.
    //struct w_sockaddr saddr;
    struct sockaddr saddr; // endpoint to rx (server address -- remote addrss)
    //uint16_t sport; // source/rx port

    uint8_t * base;       ///< Absolute start of buffer.
    uint8_t * buf;        ///< Start of payload data.
    sq_entry(w_iov) next; ///< Next w_iov in a w_iov_sq.
    uint32_t idx;         ///< Index of netmap buffer.
    uint16_t len;         ///< Length of payload data.

    /// DSCP + ECN of the received IP packet on RX, DSCP + ECN to use for the
    /// to-be-transmitted IP packet on TX.
    uint8_t flags;

    /// TTL of received IP packets.
    uint8_t ttl;

    /// Can be used by application to maintain arbitrary data. Not used by
    /// warpcore.
    uint16_t user_data;
};

#if 0
#define wv_port saddr.port
#define wv_af saddr.addr.af
//#define wv_ip4 saddr.addr.ip4
//#define wv_ip6 saddr.addr.ip6
#define wv_addr saddr.addr
#endif
//#define wv_port saddr
#define wv_af   saddr
//#define wv_addr saddr
#define ws_strprt(ad) net_sin6(&ad)->sin6_port
#define ws_straf(ad) net_sin6(&ad)->sin6_family
#define ws_stradr(ad) net_sin6(&ad)->sin6_addr

#define get_ttl(pkt) pkt->ipv6_hop_limit

#define to_endpoint(a,b) \
  memcpy((uint8_t *)&net_sin6(&a)->sin6_addr, (b).addr.ip6, sizeof((b).addr.ip6)); \
  net_sin6(&a)->sin6_port = (b).port;

/// Return the index of w_iov @p v.
///
/// @param      v     A w_iov.
///
/// @return     Index between 0-nfbus.
///

static inline uint32_t __attribute__((nonnull, no_instrument_function))
w_iov_idx(const struct w_iov * const v)
{
    return (uint32_t)(v - v->w->bufs);
}


/// Return a pointer to the w_iov with index @p i.
///
/// @param      w     Warpcore engine.
/// @param      i     Index.
///
/// @return     Pointer to w_iov.
///
static inline struct w_iov * __attribute__((nonnull, no_instrument_function))
w_iov(const struct w_engine * const w, const uint32_t i)
{
    return &w->bufs[i];
}


/// Return warpcore engine serving w_sock @p s.
///
/// @param[in]  s     A w_sock.
///
/// @return     The warpcore engine for w_sock @p s.
///
static inline struct w_engine * __attribute__((nonnull, no_instrument_function))
w_engine(const struct w_sock * const s)
{
    return s->w;
}


/// Return the maximum UDP payload for a given socket.
///
/// @param[in]  s     A w_sock.
///
/// @return     Maximum UDP payload.
///
static inline uint16_t __attribute__((nonnull, no_instrument_function))
w_max_udp_payload(const struct w_sock * const s)
{
    return s->w->mtu - ip_hdr_len(s->af_tp) - 8; // 8 = sizeof(struct udp_hdr)
}


/// Return the number of w_iov structures in the w_iov tail queue @p c.
///
/// @param[in]  q     The w_iov tail queue to compute the payload length of.
///
/// @return     Number of w_iov structs in @p q.
///
static inline uint_t __attribute__((nonnull, no_instrument_function))
w_iov_sq_cnt(const struct w_iov_sq * const q)
{
    return sq_len(q);
}


/// Return the current socket options.
///
/// @param[in]  s     A w_sock.
///
/// @return     The socket options for w_sock @p s.
///
static inline const struct w_sockopt * __attribute__((nonnull,
                                                      no_instrument_function))
w_get_sockopt(const struct w_sock * const s)
{
    return &s->opt;
}


/// Return whether a socket is connected (i.e., w_connect() has been called on
/// it) or not.
///
/// @param[in]  s     Connection.
///
/// @return     True when connected, zero otherwise.
///

/*static inline bool __attribute__((nonnull, no_instrument_function))
w_connected(const struct w_sock * const s)
{
    return udp_connected;
}*/

#if 0
static inline bool __attribute__((nonnull))
w_is_linklocal(const struct w_addr * const a)
{
    if (a->af == AF_INET)
        return (a->ip4 & 0x0000ffff) == 0x0000fea9; // 169.254.0.0/16
    else
        return a->ip6[0] == 0xfe && (a->ip6[1] & 0xc0) == 0x80;
}
#endif

static inline bool __attribute__((nonnull))
w_is_private(const struct w_addr * const a)
{
    if (a->af == AF_INET)
        return (a->ip4 & 0x000000ff) == 0x0000000a || // 10.0.0.0/8
               (a->ip4 & 0x0000f0ff) == 0x000010ac || // 172.16.0.0/12
               (a->ip4 & 0x0000ffff) == 0x0000a8c0;   // 192.168.0.0/16
    else
        return a->ip6[0] == 0xfe && (a->ip6[1] & 0xc0) == 0x80;
}


extern struct w_engine * __attribute__((nonnull))
w_init(const char * const ifname, const uint32_t rip, const uint_t nbufs);

extern void __attribute__((nonnull)) w_cleanup(struct w_engine * const w);

extern struct w_sock * __attribute__((nonnull(1)))
w_bind(struct w_engine * const w,
       const uint16_t addr_idx,
       const uint16_t port,
       const struct w_sockopt * const opt);

extern int __attribute__((nonnull))
w_connect(struct w_sock * const s, const struct sockaddr * const peer);

extern void __attribute__((nonnull)) w_close(struct w_sock * const s);

extern void __attribute__((nonnull)) w_alloc_len(struct w_engine * const w,
                                                 const int af,
                                                 struct w_iov_sq * const q,
                                                 const uint_t qlen,
                                                 const uint16_t len,
                                                 const uint16_t off);

extern void __attribute__((nonnull)) w_alloc_cnt(struct w_engine * const w,
                                                 const int af,
                                                 struct w_iov_sq * const q,
                                                 const uint_t count,
                                                 const uint16_t len,
                                                 const uint16_t off);

extern struct w_iov * __attribute__((nonnull))
w_alloc_iov(struct w_engine * const w,
            const int af,
            const uint16_t len,
            const uint16_t off);

extern void __attribute__((nonnull))
w_tx(struct w_sock * const s, struct w_iov_sq * const o);

extern uint_t w_iov_sq_len(const struct w_iov_sq * const q);

extern void __attribute__((nonnull))
w_rx(struct w_sock * const s, struct w_iov_sq * const i);

extern uint16_t __attribute__((nonnull))
recvfr(struct net_rec *u6_rec, uint8_t *b);

//extern int __attribute__((nonnull(1,2,3)))
//quic_sendto(struct net_rec *u6rec, struct sockaddr *dst_addr, uint8_t *buf, uint16_t len);

extern void __attribute__((nonnull)) w_nic_tx(struct w_engine * const w);

extern bool __attribute__((nonnull))
w_nic_rx(struct w_engine * const w, const int64_t nsec);

extern uint32_t __attribute__((nonnull))
w_rx_ready(struct w_engine * const w, struct w_sock_slist * sl);

extern uint16_t __attribute__((nonnull))
w_max_iov_len(const struct w_iov * const v, const uint16_t af);

extern void __attribute__((nonnull)) w_free(struct w_iov_sq * const q);

extern void __attribute__((nonnull)) w_free_iov(struct w_iov * const v);

//extern const char * __attribute__((nonnull))
//w_ntop(const struct w_addr * const addr, char * const dst);

extern void w_init_rand(void);

extern uint64_t w_rand64(void);

extern uint64_t w_rand_uniform64(const uint64_t upper_bound);

extern uint32_t w_rand32(void);

extern uint32_t w_rand_uniform32(const uint32_t upper_bound);

extern bool __attribute__((nonnull))
w_addr_cmp(const struct w_addr * const a, const struct w_addr * const b);

extern bool __attribute__((nonnull))
w_sockaddr_cmp(const struct w_sockaddr * const a,
               const struct sockaddr * const b);

extern void __attribute__((nonnull))
w_set_sockopt(struct w_sock * const s, const struct w_sockopt * const opt);

extern uint64_t w_now(const clockid_t clock);

extern void w_nanosleep(const uint64_t ns);

//extern bool __attribute__((nonnull))
//w_to_waddr(struct w_addr * const wa, const struct sockaddr * const sa);

#define RIOT_VERSION

#ifdef __cplusplus
}
#endif
