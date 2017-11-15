/*
 * net.h
 *
 *  Created on: 2 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NET_MTU 1500

#define NET_PORT 20000

#define PDU_BUF_SIZE (NET_MTU*10)

#define PRINTF_NETADDR_IPv4_FMT "%d.%d.%d.%d"
#define PRINTF_NETADDR_IPv4_VAL(net_addr) \
        ((char*)(&(net_addr).addr_in.sin_addr))[0], \
        ((char*)(&(net_addr).addr_in.sin_addr))[1], \
        ((char*)(&(net_addr).addr_in.sin_addr))[2], \
        ((char*)(&(net_addr).addr_in.sin_addr))[3]

#define PRINTF_NETADDR_PORT_FMT "%d"
#define PRINTF_NETADDR_PORT_VAL(net_addr) \
        (net_addr).addr_in.sin_port

typedef union
{
    struct sockaddr addr;
    struct sockaddr_in addr_in;
} net_addr_t;


typedef struct net_socket_s
{
    int sock;
    net_addr_t addr_;
} net_socket_t;

extern net_socket_t * net_socket_create(short port, const char * hostname);
extern int net_socket_bind(const net_socket_t * sock);
extern net_socket_t * net_socket_create_sockaddr(struct sockaddr addr);
void net_socket_close(net_socket_t * ns);

extern void net_send(const net_socket_t * sock, void * buf, size_t size);

extern int net_recv(const net_socket_t * net_sock, char * buf, size_t * buf_len, size_t buf_size, struct sockaddr * addr, socklen_t * addr_len);

#define net_pdu_free(buf) \
        do \
        {  \
            Z_free(buf); \
            buf = NULL;  \
        } while(0)



#endif /* SRC_NET_H_ */
