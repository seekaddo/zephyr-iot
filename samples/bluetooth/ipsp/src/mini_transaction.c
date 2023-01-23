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
typedef struct {

  struct w_engine *w;
  const char *  req;
  const char *  peer;
  struct q_conn *  c;
  struct q_stream * s ;
  struct sockaddr  p_addr;
  uint16_t qrcnt;
  //uint16_t clean;
  //quic_udp_callback cl_callback;
} qmcon;


//PROCESS(quic_etranx, "QUIC Transac");

// XXX: change "flash" to 0 to disable 0-RTT:
static const struct q_conf qc = {0, "flash", 0, 0,
                                 0, 0, 0, 20, false};
static qmcon tranx_conn;
static struct w_iov_sq o = w_iov_sq_initializer(o);


//static struct etimer timer;
#define CLIENT_SEND_INTERVAL      (10 * 1)

void quic_transx(const char * const req, const struct in6_addr  * const peer )
{

  tranx_conn.peer = SERVER_IPV6_ADDR;
  tranx_conn.qrcnt = 1;
  tranx_conn.req = req;

  net_ipv6_addr_copy_raw((uint8_t *)&net_sin6(&tranx_conn.p_addr)->sin6_addr,
			 peer->s6_addr);
  net_sin6(&tranx_conn.p_addr)->sin6_family = AF_INET6;
  net_sin6(&tranx_conn.p_addr)->sin6_port = htons(4432); // default server port

  //start the state machine for the first connection or handshake
  LOG_INF("q_connect: connection /handshake handler init \n");
  q_alloc(tranx_conn.w, &o, 0, AF_INET6, 512); // af family is ipv6
  struct w_iov * const v = sq_first(&o);
  v->len = sprintf((char *)v->buf, "GET %s\r\n", tranx_conn.req);

  static const struct q_conn_conf qcc = {
      30, 0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0xff000000 + DRAFT_VERSION};
  LOG_INF("quic_etranx:  new transaction \n");
  tranx_conn.c  = q_connect(tranx_conn.w, &tranx_conn.p_addr, tranx_conn.peer,
                           &o, &tranx_conn.s, true,
                           "hq-" DRAFT_VERSION_STRING, &qcc);

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


void quic_init_Wegine(void)
{
  LOG_INF("q_init: init wengine \n");
  tranx_conn.w = q_init("uIP", &qc);

  LOG_INF("quic_etranx:  q_init done and ready for quic conn \n");
  tranx_conn.c = 0;
}
