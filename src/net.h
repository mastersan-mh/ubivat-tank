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

#define NET_PORT 20000

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

void * net_recv(const net_socket_t * net_sock, size_t * size, struct sockaddr * addr, socklen_t * addr_len);

#define net_recv_free(buf) \
		do \
		{  \
			Z_free(buf); \
			buf = NULL;  \
		} while(0)



#endif /* SRC_NET_H_ */
