#include <string.h>
#include <stdint.h>

#include "jerry-api.h"

#include "os.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "timers.h"
#include "os.h"
#include "httpclient.h"
#include "mcs.h"
#include "microlattice.h"

#define TRX_PACKET_COUNT 3000

char rcv_buf_old[100] = {0};
int tcpClientSocket;
int ret;

DELCARE_HANDLER(__tcpClient) {
  /* ip */
  jerry_size_t ip_req_sz = jerry_get_string_size (args_p[0]);
  jerry_char_t ip_buffer[ip_req_sz];
  jerry_string_to_char_buffer (args_p[0], ip_buffer, ip_req_sz);
  ip_buffer[ip_req_sz] = '\0';

  struct sockaddr_in addr;
  int count = 0;
  int rcv_len, rlen;
  // struct netif *sta_if = netif_find_by_type(NETIF_TYPE_STA);

  os_memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = lwip_htons((int) jerry_get_number_value (args_p[1]));
  addr.sin_addr.s_addr = inet_addr(ip_buffer);
  // inet_addr_from_ipaddr(&addr.sin_addr, ip);


  tcpClientSocket = lwip_socket(AF_INET, SOCK_STREAM, 0);

  if (tcpClientSocket < 0) {
    printf("tcp client create fail\n");
    goto idle;
  }
  ret = lwip_connect(tcpClientSocket, (struct sockaddr *)&addr, sizeof(addr));

  if (ret < 0) {
    lwip_close(tcpClientSocket);
    printf("tcp client connect fail\n");
    goto idle;
  }

  char script [] = "global.eventStatus.emit('tcpConnect', true);";
  jerry_value_t eval_ret;
  eval_ret = jerry_eval(script, strlen (script), false);
  jerry_release_value (eval_ret);

  for (;;) {
    char rcv_buf[100] = {0};

    rcv_len = 0;

    rlen = lwip_recv(tcpClientSocket, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
    rcv_len += rlen;

    printf("rcv_buf: %s\n", rcv_buf);

    if (strcmp(rcv_buf_old, rcv_buf) != 0) {
      jerry_value_t params[0];
      params[0] = jerry_create_string(rcv_buf);

      jerry_value_t this_val = jerry_create_undefined();
      jerry_value_t ret_val = jerry_call_function (args_p[5], this_val, &params, 1);

      *rcv_buf_old = "";
      strcpy(*rcv_buf_old, rcv_buf);

      jerry_release_value(params);
      jerry_release_value(this_val);
      jerry_release_value(ret_val);
    }

    count++;
    vTaskDelay(1000);
  }
  /* close */
  // ret = lwip_close(s);
  // printf("tcp client s close:ret = %d\n", ret);

idle:
  return jerry_create_boolean(true);

  return jerry_create_boolean(true);
}

DELCARE_HANDLER(__tcpServer) {
  int s;
  int c;
  int ret;
  int rlen;
  struct sockaddr_in addr;
  char srv_buf[32] = {0};

  printf("tcp_server_test start\n");

  os_memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = lwip_htons((int) jerry_get_number_value (args_p[0]));
  addr.sin_addr.s_addr = lwip_htonl(IPADDR_ANY);

  /* create the socket */
  s = lwip_socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
      printf("tcp server create fail\n");
      goto done;
  }

  ret = lwip_bind(s, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
      printf("tcp server bind fail\n");
      goto clean;
  }

  ret = lwip_listen(s, 0);
  if (ret < 0) {
      printf("tcp server listen fail\n");
      goto clean;
  }

  while ( 1 ) {
      socklen_t sockaddr_len = sizeof(addr);
      c = lwip_accept(s, (struct sockaddr *)&addr, &sockaddr_len);
      if (c < 0) {
          printf("tcp server accept error\n");
          break;   // or go on waiting for connect requestion?
      }

      printf("tcp server waiting for data...\n");
      while (rlen = lwip_read(c, srv_buf, sizeof(srv_buf) - 1)) {
          if (rlen < 0) {
              printf("read error.\n");
              break;
          }
          srv_buf[rlen] = 0; //for the next statement - printf string.
          printf("tcp server received data:%s", srv_buf);

          lwip_write(c, srv_buf, rlen);      // sonar server
      }

      lwip_close(c);
  }

clean:
    lwip_close(s);

done:
    printf("tcp server test done\n");

  return jerry_create_boolean(true);
}

DELCARE_HANDLER(__tcpClose) {
  lwip_close(tcpClientSocket);
  return jerry_create_boolean(true);
}

DELCARE_HANDLER(__tcpSend) {

  jerry_size_t data_req_sz = jerry_get_string_size (args_p[0]);
  jerry_char_t data_buffer[data_req_sz];
  jerry_string_to_char_buffer (args_p[0], data_buffer, data_req_sz);
  data_buffer[data_req_sz] = '\0';

  lwip_write(tcpClientSocket, data_buffer, data_req_sz);

  return jerry_create_boolean(true);
}

void ml_tcp_init(void) {
  REGISTER_HANDLER(__tcpServer);
  REGISTER_HANDLER(__tcpClient);
  REGISTER_HANDLER(__tcpClose);
  REGISTER_HANDLER(__tcpSend);
}