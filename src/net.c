/*
 * net.c
 *
 *  Created on: 2 янв. 2017 г.
 *      Author: mastersan
 */

#include "net.h"

#include "Z_mem.h"

#include "game.h"

#include <stddef.h>

#include <fcntl.h>

/*
 * @param hostname = NULL - any address
 */
net_socket_t * net_socket_create(short port, const char * hostname)
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0)
		return NULL;

	if(fcntl(sock, F_SETFL, O_NONBLOCK))
	{
		game_halt("fcntl() failed");
	}
	net_socket_t * ns = Z_malloc(sizeof(net_socket_t));
	ns->sock = sock;
	ns->addr_.addr_in.sin_family = AF_INET; // домены Internet
	ns->addr_.addr_in.sin_port = htons(port);
	ns->addr_.addr_in.sin_addr.s_addr = inet_addr (hostname ? hostname : INADDR_ANY);
	return ns;
}

int net_socket_bind(const net_socket_t * sock)
{
	return bind(sock->sock, &sock->addr_.addr, sizeof(sock->addr_.addr));
}

net_socket_t * net_socket_create_sockaddr(struct sockaddr addr)
{

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0)
		return NULL;

	// Указываем параметры сервера
/*
	addr_to.sin_family = AF_INET; // домены Internet
	addr_to.sin_port = htons(port); // или любой другой порт...
	addr_to.sin_addr.s_addr = inet_addr(hostname);
*/
	net_socket_t * ns = Z_malloc(sizeof(net_socket_t));
	ns->addr_.addr = addr;
	ns->sock = sock;
	return ns;
}

void net_socket_close(net_socket_t * net_sock)
{
	if(!net_sock)
		return;
	close(net_sock->sock);
	Z_free(net_sock);
}

/**
 *
 */
void net_send(const net_socket_t * net_sock, void * buf, size_t size)
{
	if( sendto(net_sock->sock, buf, size, 0, &net_sock->addr_.addr, sizeof(net_sock->addr_.addr)) < 0 )
	{
		game_halt("sendto()");
	}
}

#define LOCK()
#define UNLOCK()

#define MAXCONTENTLENGTH (65535)
static char recv_buf[MAXCONTENTLENGTH];
/**
 *
 */
void * net_recv(const net_socket_t * net_sock, size_t * size, struct sockaddr * addr, socklen_t * addr_len)
{
	LOCK();
	if(*addr_len <= 0)
		*addr_len = sizeof(struct sockaddr);
	ssize_t size_ = recvfrom(net_sock->sock, recv_buf, MAXCONTENTLENGTH, 0, addr, addr_len);
	if(size_ <= 0)
	{
		*size = 0;
		return NULL;
	}
	*size = size_;
	void * buf = Z_malloc(size_);
	memcpy(buf, recv_buf, size_);
	UNLOCK();
	return buf;
}



