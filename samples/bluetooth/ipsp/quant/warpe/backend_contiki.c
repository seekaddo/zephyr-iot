// SPDX-License-Identifier: BSD-2-Clause
//
// Copyright (c) 2014-2022, todo;
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

#include <errno.h>
#include <inttypes.h>

#if defined(__linux__)
#include <limits.h>
#else
#include <sys/types.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
//#include <sys/param.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>

#ifndef NDEBUG
#include <string.h>
#endif

#include "warpcore.h"

#ifndef PARTICLE
//#include <sys/uio.h>
#endif

#include "backend.h"

//#include <fmt.h>
#include <stdint.h>
//#include <sys/select.h>
//#include "contiki.h"
//#include "net/ipv6/uip-udp-packet.h"
//#include "net/ipv6/uiplib.h"
//#include "quic-endpoint.h"
//#include "quic-transport.h"

/* Log configuration */
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(quic_backend);



void w_set_sockopt(struct w_sock * const s, const struct w_sockopt * const opt)
{
  s->opt = *opt;
}


uint16_t backend_addr_cnt(void)
{
  //default only 1 ipv6 connection
  return quic_udp_active();
}



/// Initialize the warpcore socket backend for engine @p w. Sets up the extra
/// buffers.
///
/// @param      w      Backend engine.
/// @param[in]  nbufs  Number of packet buffers to allocate.
///
void backend_init(struct w_engine * const w, const uint32_t nbufs)
{
    w->have_ip6 = true;
    w->mtu = UIP_LINK_MTU;
    w->mbps = UINT32_MAX;
    w->b->n = 0;
    w->b->udp_cn = quic_udp_con();

    struct w_ifaddr * ia = &w->ifaddr[0]; // just dummy
    ia->addr.af = AF_INET6; // value 10
    memcpy(ia->addr.ip6,uip_hostaddr.u8, sizeof(ia->addr.ip6));
    ia->scope_id = 0;

    //todo: Use contiki Memalloca
    ensure((w->mem = calloc(nbufs, max_buf_len(w))) != 0,
           "cannot alloc %" PRIu32 " * %u buf mem", nbufs, max_buf_len(w));
    ensure((w->bufs = calloc(nbufs, sizeof(*w->bufs))) != 0,
           "cannot alloc bufs");
    w->backend_name = "contiki-ng";

    for (uint32_t i = 0; i < nbufs; i++) {
        init_iov(w, &w->bufs[i], i);
        sq_insert_head(&w->iov, &w->bufs[i], next);
        ASAN_POISON_MEMORY_REGION(w->bufs[i].buf, max_buf_len(w));
    }

    //w->b->ep = epoll_create1(0);
    w->backend_variant = "uIp-UDP";

    warn(DBG, "%s backend using %s", w->backend_name, w->backend_variant);
}


/// Shut a warpcore socket engine down cleanly. Does nothing, at the moment.
///
/// @param      w     Backend engine.
///
void backend_cleanup(struct w_engine * const w)
{
  struct w_sock * s;
  sl_foreach (s, &w->b->socks, __next)
      w_close(s);

  free(w->mem);
  free(w->bufs);
  w->b->n = 0;
}
/// Close a RIOT socket.
///
/// @param      s     The w_sock to close.
///
void backend_close(struct w_sock * const s)
{
  s->fd = 0;
  sl_remove(&s->w->b->socks, s, w_sock, __next);
}



/// Bind a warpcore socket-backend socket. Calls the underlying Socket API.
///
/// @param      s     The w_sock to bind.
/// @param[in]  opt   Socket options for this socket. Can be zero.
///
/// @return     Zero on success, @p errno otherwise.
///
int backend_bind(struct w_sock * const s, const struct w_sockopt * const opt)
{
  //todo: check if the local and remote is set
  s->af_tp = AF_INET6; //IP_v6 only
  s->fd  = 197; // contiki only
  sl_insert_head(&s->w->b->socks, s, __next);
  return 0;
}


void backend_preconnect(struct w_sock * const s __attribute__((unused))) {}


/// The socket backend performs no operation here.
///
/// @param      s     The w_sock to connect.
///
/// @return     Zero on success, @p errno otherwise.
///
int backend_connect(struct w_sock * const s)
{

    return !quic_udp_active();
}


/// Loops over the w_iov structures in the w_iov_sq @p o, sending them all
/// over w_sock @p s.
///
/// @param      s     w_sock socket to transmit over.
/// @param      o     w_iov_sq to send.
///
void w_tx(struct w_sock * const s, struct w_iov_sq * const o)
{
  const bool is_connected = w_connected(s);

  struct w_iov * v = sq_first(o);
  while (v) {
    if(is_connected == false)
    {
      LOG_ERR("uIP-udp not connected ");
      break ;
    }

    if (unlikely(quic_sendto(&v->saddr, v->buf, v->len) < 0) )
      warn(ERR, "sendto returned %d (%s)", errno, strerror(errno));
    v = sq_next(v, next);
  };
}

extern uint32_t syncNewData;

/// Return any new data that has been received on a socket by appending it to
/// the w_iov tail queue @p i. The tail queue must eventually be returned to
/// warpcore via w_free().
///
/// @param      s     w_sock for which the application would like to receive new
///                   data.
/// @param      i     w_iov tail queue to append new data to.
///
void w_rx(struct w_sock * const s, struct w_iov_sq * const i)
{
  do {
    struct w_iov *v = w_alloc_iov(s->w, s->af_tp, 0, 0);
    if (unlikely(v == 0))
      return;

    v->len = recvfr(v->buf, v->len);

    if (likely(v->len > 0)) {
      // v->renpt = sa_port(&sa);
      set_endpoint(&v->saddr);
      v->ttl = get_ttl();
      // w_to_waddr(&v->wv_addr, (struct sockaddr *)&sa); // server address wv_addr
      sq_insert_tail(i, v, next);
    } else {
      w_free_iov(v);
    }
  } while (syncNewData != 0);
}


/// The sock backend performs no operation here.
///
/// @param[in]  w     Backend engine.
///
void w_nic_tx(struct w_engine * const w __attribute__((unused))) {}


/// Check/wait until any data has been received.
///
/// @param[in]  w     Backend engine.
/// @param[in]  nsec  Timeout in nanoseconds. Pass zero for immediate return, -1
///                   for infinite wait.
///
/// @return     Whether any data is ready for reading.
///
bool w_nic_rx(struct w_engine * const w, const int64_t nsec)
{
    return syncNewData > 0;
}


/// Fill a w_sock_slist with pointers to some sockets with pending inbound
/// data. Data can be obtained via w_rx() on each w_sock in the list. Call
/// can optionally block to wait for at least one ready connection. Will
/// return the number of ready connections, or zero if none are ready. When
/// the return value is not zero, a repeated call may return additional
/// ready sockets.
///
/// @param[in]  w     Backend engine.
/// @param      sl    Empty and initialized w_sock_slist.
///
/// @return     Number of connections that are ready for reading.
///
uint32_t w_rx_ready(struct w_engine * const w, struct w_sock_slist * const sl)
{
  uint32_t i = 0;
  struct w_sock * s;
  sl_foreach (s, &w->b->socks, __next)
      if (s->fd == 197) {
           sl_insert_head(sl, s, next);
           i++;
      }

  return i;

}

