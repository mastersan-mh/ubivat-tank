/*
 * net.c
 *
 *  Created on: 2 янв. 2017 г.
 *      Author: mastersan
 */

#include "net.h"

#include "Z_mem.h"

#include "game.h"

#include <assert.h>

#include <stddef.h>
#include <fcntl.h>


/*
 * @param hostname = NULL - any address
 */
net_socket_t * net_socket_create(const net_addr_t * net_addr)
{
    assert(net_addr->addr_in.sin_family == AF_INET && "net_socket_create(): invalid sin_family?");

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock < 0)
        return NULL;

    if(fcntl(sock, F_SETFL, O_NONBLOCK))
    {
        game_halt("fcntl() failed");
    }
    net_socket_t * ns = Z_malloc(sizeof(net_socket_t));
    ns->sock = sock;
    ns->net_addr = *net_addr;
    return ns;
}

/*
 * @param hostname = NULL - any address
 */
net_socket_t * net_socket_create_hostname(short port, const char * hostname)
{
    net_addr_t net_addr;

    net_addr.addr_in.sin_family = AF_INET; // домены Internet
    net_addr.addr_in.sin_port = htons(port);
    net_addr.addr_in.sin_addr.s_addr = inet_addr (hostname ? hostname : INADDR_ANY);

    return net_socket_create(&net_addr);
}

int net_socket_bind(const net_socket_t * sock)
{
    return bind(sock->sock, &sock->net_addr.addr, sizeof(sock->net_addr.addr));
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
    ns->net_addr.addr = addr;
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
    if( sendto(net_sock->sock, buf, size, 0, &net_sock->net_addr.addr, sizeof(net_sock->net_addr.addr)) < 0 )
    {
        game_halt("sendto()");
    }
}

void net_send_addr(int sock, const net_addr_t * net_addr, void * buf, size_t size)
{
    if( sendto(sock, buf, size, 0, &net_addr->addr, sizeof(net_addr->addr)) < 0 )
    {
        game_halt("sendto()");
    }
}

#define LOCK()
#define UNLOCK()

/**
 *
 */
int net_recv(const net_socket_t * net_sock, char * buf, size_t * buf_len, size_t buf_size, struct sockaddr * addr, socklen_t * addr_len)
{
    LOCK();
    if(*addr_len <= 0)
        *addr_len = sizeof(struct sockaddr);
    ssize_t size = recvfrom(net_sock->sock, buf, buf_size, 0, addr, addr_len);
    if(size <= 0)
    {
        return -1;
    }
    *buf_len = size;
    UNLOCK();
    return 0;
}



