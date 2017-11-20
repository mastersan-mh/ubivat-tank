/*
 * net.h
 *
 *  Created on: 2 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_NET_H_
#define SRC_NET_H_

#include "types.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NET_MTU 1500

#define NET_PORT_AUTO 0

#define NET_SERVER_RECIEVER_PORT 40000
//#define NET_CLIENT_SENDER_PORT (NET_SERVER_RECIEVER_PORT+1)
#define NET_CLIENT_SENDER_PORT NET_PORT_AUTO

#define PDU_BUF_SIZE (NET_MTU*10)

#define PRINTF_NETADDR_IPv4_FMT "%d.%d.%d.%d"
#define PRINTF_NETADDR_IPv4_VAL(net_addr) \
        (int)((unsigned char*)(&(net_addr).addr_in.sin_addr))[0], \
        (int)((unsigned char*)(&(net_addr).addr_in.sin_addr))[1], \
        (int)((unsigned char*)(&(net_addr).addr_in.sin_addr))[2], \
        (int)((unsigned char*)(&(net_addr).addr_in.sin_addr))[3]

#define PRINTF_NETADDR_PORT_FMT "%d"
#define PRINTF_NETADDR_PORT_VAL(net_addr) \
        ntohs((net_addr).addr_in.sin_port)

#define PRINTF_NETADDR_FMT \
        PRINTF_NETADDR_IPv4_FMT ":" PRINTF_NETADDR_PORT_FMT
#define PRINTF_NETADDR_VAL(net_addr) \
        PRINTF_NETADDR_IPv4_VAL((net_addr)), PRINTF_NETADDR_PORT_VAL((net_addr))


typedef struct
{
    socklen_t addr_len;
    union {
        struct sockaddr addr;
        struct sockaddr_in addr_in;
    };
} net_addr_t;


extern int net_socket_open_connectionless(net_addr_t * net_addr);
extern void net_socket_close(int sock);
extern int net_addr_set(net_addr_t * net_addr, in_port_t port, in_addr_t in_addr);
extern int net_addr_set_fromstring(net_addr_t * net_addr, in_port_t port, const char * hostname);

extern void net_send(int sock, const void * buf, size_t buf_size, const net_addr_t * dest_addr);
extern int net_recv(int sock, void * buf, size_t * buf_len, size_t buf_size, net_addr_t * src_addr);

#define net_pdu_free(buf) \
        do \
        {  \
            Z_free(buf); \
            buf = NULL;  \
        } while(0)



#endif /* SRC_NET_H_ */
