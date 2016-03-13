#include <string.h>
#include <stdint.h>

#include "jerry.h"

#include "os.h"
#include "net_init.h"
#include "network_init.h"
#include "wifi_api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

#include "microlattice.h"

#define TRX_PACKET_COUNT 3000

char rcv_buf_old[100] = {0};

DELCARE_HANDLER(tcpClient) {
  int s;
  int ret;
  int ip_req_sz = jerry_api_string_to_char_buffer(args_p[0].v_string, NULL, 0);
  ip_req_sz *= -1;
  char ip_buffer [ip_req_sz + 1];
  ip_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, (jerry_api_char_t *) ip_buffer, ip_req_sz);
  ip_buffer[ip_req_sz] = '\0';

  printf("%s\n", ip_buffer);

  char send_data[] = "DovS4gPM,pS7RkWVGCRP1xv95,0";

  struct sockaddr_in addr;
  int count = 0;
  int rcv_len, rlen;

  os_memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons((int)args_p[1].v_float32);
  addr.sin_addr.s_addr =inet_addr(ip_buffer);

  s = lwip_socket(AF_INET, SOCK_STREAM, 0);

  if (s < 0) {
    printf("tcp client create fail\n");
    goto idle;
  }

  ret = lwip_connect(s, (struct sockaddr *)&addr, sizeof(addr));

  if (ret < 0) {
    lwip_close(s);
    printf("tcp client connect fail\n");
    goto idle;
  }

  for (;;) {
    char rcv_buf[100] = {0};
    if (0 == count) {
      ret = lwip_write(s, send_data, sizeof(send_data));
      printf("tcp client write:ret = %s\n", ret);
    }
    printf("tcp client waiting for data...\n");
    rcv_len = 0;
    rlen = lwip_recv(s, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
    rcv_len += rlen;

    printf("rcv_buf: %s\n", rcv_buf);

    if (strcmp(rcv_buf_old, rcv_buf) != 0) {
      jerry_api_value_t params[0];
      params[0].type = JERRY_API_DATA_TYPE_STRING;
      params[0].v_string = jerry_api_create_string (rcv_buf);
      jerry_api_call_function(args_p[2].v_object, NULL, false, &params, 1);
      *rcv_buf_old = "";
      strcpy(*rcv_buf_old, rcv_buf);
      jerry_api_release_value(&params);
    }

    // if (1000 == count) {
    //   ret = lwip_close(s);
    // }

    count++;
    vTaskDelay(1000);
  }

  /* close */
  // ret = lwip_close(s);
  // printf("tcp client s close:ret = %d\n", ret);

idle:
  ret_val_p->type = JERRY_API_DATA_TYPE_BOOLEAN;
  ret_val_p->v_bool = true;

  return true;
}

DELCARE_HANDLER(tcpServer) {
  int s;
  int c;
  int ret;
  int rlen;
  struct sockaddr_in addr;
  char srv_buf[32] = {0};
  printf("tcp_server_test start\n");

  jerry_api_value_t tcp_server_port;

  // port
  jerry_api_get_object_field_value (args_p[0].v_object, (jerry_api_char_t *) "port", &tcp_server_port);

  os_memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = lwip_htons((int)tcp_server_port.v_float32);
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
  return true;
}

DELCARE_HANDLER(tcpClose) {
  // lwip_close(s);
  return true;
}

DELCARE_HANDLER(tcpSend) {
  int data_req_sz = jerry_api_string_to_char_buffer(args_p[0].v_string, NULL, 0);
  data_req_sz *= -1;
  char data_buffer [data_req_sz + 1];
  data_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, (jerry_api_char_t *) data_buffer, data_req_sz);
  data_buffer[data_req_sz] = '\0';

  // char send_data[100];

  // ssize_t data_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, (jerry_api_char_t *) send_data, 128);
  // send_data[data_req_sz] = '\0';

  // lwip_write(s, data_buffer, sizeof(data_buffer));
  return true;
}

void ml_tcp_init(void) {
  REGISTER_HANDLER(tcpServer);
  REGISTER_HANDLER(tcpClient);
  REGISTER_HANDLER(tcpClose);
  REGISTER_HANDLER(tcpSend);
}