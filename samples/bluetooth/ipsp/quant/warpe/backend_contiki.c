// SPDX-License-Identifier: BSD-2-Clause
//
// Copyright (c) 2014-2022
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
// #include <sys/param.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>

#ifndef NDEBUG
#include <string.h>
#endif

#include "warpcore.h"

#ifndef PARTICLE
// #include <sys/uio.h>
#endif

#include "backend.h"

// #include <fmt.h>
#include <stdint.h>
// #include <sys/select.h>
// #include "contiki.h"
// #include "net/ipv6/uip-udp-packet.h"
// #include "net/ipv6/uiplib.h"
// #include "quic-endpoint.h"
// #include "quic-transport.h"

/* Log configuration */
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(quic_backend);
extern struct sockaddr myAddr;
// waiting for udp pkt after sent pkt
struct k_sem waitpkt_lock;
extern uint8_t udp_connected;
extern char *net_sprint_addr(sa_family_t af, const void *addr);

void w_set_sockopt(struct w_sock *const s, const struct w_sockopt *const opt)
{
	s->opt = *opt;
}

uint16_t backend_addr_cnt(void)
{
	// default only 1 ipv6 connection
	return 1; // quic_udp_active();
}

/// Initialize the warpcore socket backend for engine @p w. Sets up the extra
/// buffers.
///
/// @param      w      Backend engine.
/// @param[in]  nbufs  Number of packet buffers to allocate.
///
void backend_init(struct w_engine *const w, const uint32_t nbufs)
{
	k_sem_init(&waitpkt_lock, 0, K_SEM_MAX_LIMIT);
	w->have_ip6 = true;
	w->mtu = NET_IPV6_MTU; // UIP_LINK_MTU;
	w->mbps = UINT32_MAX;
	w->b->n = 0;
	w->b->udp_cn = &myAddr; // quic_udp_con();

	struct w_ifaddr *ia = &w->ifaddr[0]; // just dummy
	ia->addr.af = AF_INET6;		     // value 10
	memcpy(ia->addr.ip6, (uint8_t *)&net_sin6(&myAddr)->sin6_addr, sizeof(ia->addr.ip6));
	ia->scope_id = 0;

	// todo: Use contiki Memalloca
	ensure((w->mem = calloc(nbufs, max_buf_len(w))) != 0,
	       "cannot alloc %" PRIu32 " * %u buf mem", nbufs, max_buf_len(w));
	ensure((w->bufs = calloc(nbufs, sizeof(*w->bufs))) != 0, "cannot alloc bufs");
	w->backend_name = "zephyr-os";

	for (uint32_t i = 0; i < nbufs; i++) {
		init_iov(w, &w->bufs[i], i);
		sq_insert_head(&w->iov, &w->bufs[i], next);
		ASAN_POISON_MEMORY_REGION(w->bufs[i].buf, max_buf_len(w));
	}

	// w->b->ep = epoll_create1(0);
	w->backend_variant = "uIp-UDP";

	LOG_DBG( "%s backend using %s", w->backend_name, w->backend_variant);
}

/// Shut a warpcore socket engine down cleanly. Does nothing, at the moment.
///
/// @param      w     Backend engine.
///
void backend_cleanup(struct w_engine *const w)
{
	struct w_sock *s;
	sl_foreach(s, &w->b->socks, __next) w_close(s);

	free(w->mem);
	free(w->bufs);
	w->b->n = 0;
}
/// Close a RIOT socket.
///
/// @param      s     The w_sock to close.
///
void backend_close(struct w_sock *const s)
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
int backend_bind(struct w_sock *const s, const struct w_sockopt *const opt)
{
	// todo: check if the local and remote is set
	s->af_tp = AF_INET6; // IP_v6 only
	s->fd = 197;	     // contiki only
	sl_insert_head(&s->w->b->socks, s, __next);
	return 0;
}

void backend_preconnect(struct w_sock *const s __attribute__((unused)))
{
}

static inline void pkt_sent(struct net_context *context,
			    int status,
			    void *user_data)
{
	if (status >= 0) {
		LOG_DBG("Sent %d bytes", status);
	}
}

/// The socket backend performs no operation here.
///
/// @param      s     The w_sock to connect.
///
/// @return     Zero on success, @p errno otherwise.
///
int backend_connect(struct w_sock *const s)
{

	return !udp_connected; //! quic_udp_active();
}

/// Loops over the w_iov structures in the w_iov_sq @p o, sending them all
/// over w_sock @p s.
///
/// @param      s     w_sock socket to transmit over.
/// @param      o     w_iov_sq to send.
///
extern struct qmcon tranx_conn;
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
void w_tx(struct w_sock *const s, struct w_iov_sq *const o)
{
	const bool is_connected = udp_connected;

	struct w_iov *v = sq_first(o);
	int ret;
	while (v) {
		if (is_connected == false) {
			LOG_ERR("uIP-udp not connected ");
			break;
		}

		//static uint8_t buf_tx[NET_IPV6_MTU+20];
		//snprintf(buf_tx, v->len,"%s",v->buf);
		//memset(buf_tx,45, 300);
		LOG_INF("Now sending buffer of size (%d)", v->len);

		ret = net_context_sendto(tranx_conn.w->u6_rec.cntx,  v->buf,  v->len, &v->saddr,
					 sizeof(struct sockaddr_in6),
					 pkt_sent, K_NO_WAIT, NULL);

		if (ret < 0) {
			LOG_ERR("Cannot send data to peer (%d) %s", ret, strerror(ret));
		}

		/*if (unlikely(quic_sendto(&s->w->u6_rec, &v->saddr, v->buf, v->len) < 0))
			LOG_ERR("Cannot send data to peer (%d)", ret);*/
		v = sq_next(v, next);
	};
}

int
quic_sendto(struct net_rec *u6rec, struct sockaddr *dst_addr, uint8_t *buf_tx, uint16_t len)
{
	int ret;
	ret = net_context_sendto(tranx_conn.w->u6_rec.cntx, buf_tx, len, dst_addr,
						    sizeof(struct sockaddr_in6),
				 pkt_sent, K_NO_WAIT, u6rec->user_data);
	LOG_INF("UDP ret: %d dest: %s port: %d",
		ret, net_sprint_addr(AF_INET6,(uint8_t *)&net_sin6(dst_addr)->sin6_addr),
		ntohs(net_sin6(dst_addr)->sin6_port) );
	if (ret < 0) {
		LOG_ERR("Cannot send data to peer (%d)", ret);
	}

	return ret;
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
void w_rx(struct w_sock *const s, struct w_iov_sq *const i)
{
	do {
		struct w_iov *v = w_alloc_iov(s->w, s->af_tp, 0, 0);
		if (unlikely(v == 0))
			return;

		v->len = recvfr(&s->w->u6_rec, v->buf );

		if (likely(v->len > 0)) {
			// v->renpt = sa_port(&sa);
			//set_endpoint(&v->saddr);
			v->saddr = s->w->u6_rec.src_addr;
			//v->sport = s->w->u6_rec->src_port;
			v->ttl = s->w->u6_rec.pkt->ipv6_hop_limit; // ttl or pkt->ipv6_hop_limit
			// w_to_waddr(&v->wv_addr, (struct sockaddr *)&sa); // server address
			// wv_addr
			sq_insert_tail(i, v, next);
		} else {
			w_free_iov(v);
		}
	} while (0); //(syncNewData != 0); // at the moment only 1 pkt AAT
}

uint16_t recvfr(struct net_rec *u6_rec, uint8_t *buf)
{
	// get pkt length
	int ret;
	int reply_len = net_pkt_remaining_data(u6_rec->pkt);
	ret = net_pkt_read(u6_rec->pkt, buf, reply_len);
	if (ret < 0) {
		LOG_ERR("cannot read packet: %d", ret);
		syncNewData = 0;
		return 0;
	}

	syncNewData = (syncNewData > 0) ? syncNewData - 1 : 0;
	net_pkt_unref(u6_rec->pkt);
	return reply_len;
}

/// The sock backend performs no operation here.
///
/// @param[in]  w     Backend engine.
///
void w_nic_tx(struct w_engine *const w __attribute__((unused)))
{
}

/// Check/wait until any data has been received.
///
/// @param[in]  w     Backend engine.
/// @param[in]  nsec  Timeout in nanoseconds. Pass zero for immediate return, -1
///                   for infinite wait.
///
/// @return     Whether any data is ready for reading.
///
bool w_nic_rx(struct w_engine *const w, const int64_t nsec)
{
	// wait here for a specific time using the semaphore
	//  rx callback will release the semaphore before timeout
	k_sem_take(&waitpkt_lock, K_NSEC(nsec));
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
uint32_t w_rx_ready(struct w_engine *const w, struct w_sock_slist *const sl)
{
	uint32_t i = 0;
	struct w_sock *s;
	sl_foreach(s, &w->b->socks, __next)
	    if (s->fd == 197){
		 sl_insert_head(sl, s, next);
		 i++;
	    }

	return i;
}
