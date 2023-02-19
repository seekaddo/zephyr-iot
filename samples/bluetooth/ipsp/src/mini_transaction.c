//
// Created by Dennis Kwame Addo on 12/20/22.
//


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
//#include <sys/param.h>
#include "quant.h"


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(quic_transx);
#define SERVER_IPV6_ADDR "2001:db8::2"



//PROCESS(quic_etranx, "QUIC Transac");

// XXX: change "flash" to 0 to disable 0-RTT:
static const struct q_conf qc = {0, "flash", 0, 0,
                                 0, 0, 0, 20, false};
struct qmcon tranx_conn;
static struct w_iov_sq o = w_iov_sq_initializer(o);


static inline void pkt_sent(struct net_context *context,
			    int status,
			    void *user_data)
{
	if (status >= 0) {
		LOG_DBG("Sent %d bytes", status);
	}
}

//static struct etimer timer;
#define CLIENT_SEND_INTERVAL      (10 * 1)

void quic_transx(const char * const req, const char* peer )
{

  tranx_conn.peer = peer;//SERVER_IPV6_ADDR;
  tranx_conn.qrcnt = 1;
  tranx_conn.req = req; // "/index.html"
  struct in6_addr in6addr_my;

  if (net_addr_pton(AF_INET6,
		    peer,
		    &in6addr_my) < 0) {
	  LOG_ERR("Invalid IPv6 address %s",
		  CONFIG_NET_CONFIG_MY_IPV6_ADDR);
	  return ;
  }

  net_ipaddr_copy(&net_sin6(&tranx_conn.p_addr)->sin6_addr,
			 &in6addr_my);
  net_sin6(&tranx_conn.p_addr)->sin6_family = AF_INET6;
  net_sin6(&tranx_conn.p_addr)->sin6_port = htons(4432); // default server port

  //start the state machine for the first connection or handshake
  LOG_INF("q_connect: connection /handshake handler init \n");
  q_alloc(tranx_conn.w, &o, 0, AF_INET6, 512); // af family is ipv6
  struct w_iov * const v = sq_first(&o);
  v->len = sprintf((char *)v->buf, "GET %s\r\n", tranx_conn.req);
#if 0
  ///############### testing
  static uint8_t buf_tx[NET_IPV6_MTU + 20];
  //snprintf(buf_tx, v->len,"%s",v->buf);
  memset(buf_tx,45, 250);
  int ret = net_context_sendto(tranx_conn.w->u6_rec.cntx, buf_tx, 200 , &tranx_conn.p_addr,
			       sizeof(struct sockaddr_in6) ,
			       pkt_sent, K_NO_WAIT, buf_tx);
  if (ret < 0) {
	  LOG_ERR("Cannot send data to peer (%d)", ret);
  }

  do{
	  k_sleep(K_SECONDS(70));
  } while (1);
#endif

  static const struct q_conn_conf qcc = {
      60, 0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0xff000000 + DRAFT_VERSION};
  LOG_INF("quic_etranx:  new transaction \n");
  tranx_conn.c  = q_connect(tranx_conn.w, &tranx_conn.p_addr, tranx_conn.peer,
                           &o, &tranx_conn.s, true,
                           "hq-" DRAFT_VERSION_STRING, &qcc);

//do{
//	k_sleep(K_SECONDS(70));
//} while (1);

  uint16_t cnt = 0;
  while (1)
  {

	  if (tranx_conn.c) {
		  LOG_INF("======Get Response==========");
		  struct w_iov_sq i = w_iov_sq_initializer(i);
		  q_read_stream(tranx_conn.s, &i, true);
		  const uint16_t len = w_iov_sq_len(&i);
		  LOG_INF("retrieved %" PRIu32 " bytes", len);
		  struct w_iov *const sv = sq_first(&i);
		  LOG_INF("----------Payload %d bytes->\n%s", sv->len, sv->buf);

		  LOG_INF("=====Get Done=======");
		  q_free(&i);
		  q_free_stream(tranx_conn.s);
		  if(cnt > 1)
			  break ;

		  LOG_INF("====Free Done========");
		  //sleep(4); // sleep for some time before new request

		  //todo: here we sent a fresh new request, Using old connection but new stream
		  LOG_INF("==============New Stream Pointer ================================");
		  tranx_conn.s = q_rsv_stream(tranx_conn.c, true); // request a new stream
		  LOG_INF("=====New Stream Pointer Done ===================");
		  q_alloc(tranx_conn.w, &o, 0, AF_INET6, 512); // allocate memory for the stream
		  struct w_iov * const vv = sq_first(&o);
		  vv->len = sprintf((char *)vv->buf, "GET %s\r\n", tranx_conn.req);
		  q_write(tranx_conn.s, &o, true); // send new request on the same stream 0

		  cnt += 1;

	  } else {
		  LOG_ERR( "could not retrieve %s", v->buf);
		  cnt += 1;
		  break ;
	  }
  }
  LOG_INF("====Con Closed ========");
  q_free(&o);
  q_close(tranx_conn.c, 0, "No connection");

  q_cleanup(tranx_conn.w);

}
#if 0
PROCESS_THREAD(quic_etranx, ev, data)
{
  PROCESS_BEGIN();
  LOG_INFO("  QUIC Transx Started  \n");

  do {
    etimer_set(&timer, 2);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    LOG_INFO("  QUIC wait \n");
  } while(!quic_udp_active());

  etimer_set(&timer, 1);

  while(1) {
    PROCESS_YIELD();
    if((ev == PROCESS_EVENT_TIMER) && (data == &timer)) {
      etimer_set(&timer, CLIENT_SEND_INTERVAL);
    }
    static uint8_t qwait = 0;


    if (tranx_conn.c == NULL) { // connection /handshake handler
      LOG_INFO("q_connect: connection /handshake handler init \n");
      q_alloc(tranx_conn.w, &o, 0, AF_INT6, 512); // af family is ipv6
      struct w_iov * const v = sq_first(&o);
      v->len = sprintf((char *)v->buf, "GET %s\r\n", tranx_conn.req);

      static const struct q_conn_conf qcc = {
          30, 0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0xff000000 + DRAFT_VERSION};
      LOG_INFO("quic_etranx:  new transaction \n");
      tranx_conn.c  = q_connect(tranx_conn.w, &tranx_conn.p_addr, tranx_conn.peer,
                               &o, &tranx_conn.s, true,
                    "hq-" DRAFT_VERSION_STRING, &qcc);

      qwait = 1; // wait for response
    }

    if (tranx_conn.c ) { // streams handler
      if(qwait) {
        warn(DBG, "==============Get Response================================");
        struct w_iov_sq i = w_iov_sq_initializer(i);
        q_read_stream(tranx_conn.s, &i, true);
        const uint16_t len = w_iov_sq_len(&i);
        warn(DBG, "retrieved %" PRIu32 " bytes", len);
        struct w_iov *const sv = sq_first(&i);
        tranx_conn.qrcnt = 0;
        tranx_conn.cl_callback(&sv->saddr, sv->buf, sv->len);
        // warn(DBG, "Payload %d bytes->\n%s", sv->len, sv->buf);

        warn(DBG, "==============Get Done================================");
        qwait = 0;
        q_free(&i);
        q_free_stream(tranx_conn.s);
      }

      if(tranx_conn.qrcnt) {
        // todo: here we sent a fresh new request, Using old connection but new stream
        tranx_conn.s = q_rsv_stream(tranx_conn.c, true); // request a new stream
        q_alloc(tranx_conn.w, &o, 0, AF_INT6,
                512); // allocate memory for the stream
        struct w_iov *const vv = sq_first(&o);
        vv->len = sprintf((char *)vv->buf, "GET %s\r\n", tranx_conn.req);
        q_write(tranx_conn.s, &o, true); // send new request on the same stream 0
        qwait = 1; // wait for response
      }


    }
    else {
      struct w_iov * const vb = sq_first(&o);
      warn(DBG, "could not retrieve %s", vb->buf);
    }

    if(tranx_conn.clean) {
      // free the memory here
      q_free(&o);
      q_close(tranx_conn.c, 0, "No connection");
      q_cleanup(tranx_conn.w);
    }

  } /* while (1) */

  PROCESS_END();
}
#endif


void quic_init_Wegine(struct net_context *u6_ctxt)
{
  LOG_INF("q_init: init wengine \n");
  tranx_conn.w = q_init("uIP", &qc);
  tranx_conn.w->u6_rec.cntx = u6_ctxt;

  LOG_INF("quic_etranx:  q_init done and ready for quic conn \n");
  tranx_conn.c = 0;
}
