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

#define NET_RECVFROM_FLAGS MSG_DONTWAIT
#define NET_SENDTO_FLAGS 0


/**
 *
 */
int net_socket_open_connectionless(net_addr_t * net_addr)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock < 0)
        return -1;

    if(bind(sock, &net_addr->addr, sizeof(net_addr->addr)))
    {
        game_halt("bind()");
    }

    int res;

    net_addr_t sock_net_addr;
    sock_net_addr.addr_len = sizeof(sock_net_addr.addr);
    getsockname(sock, &sock_net_addr.addr, &sock_net_addr.addr_len);


    int i = 1;
    res = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &i, sizeof(i));
    if(res)
    {
        game_halt("setsockopt() failed");
    }

    return sock;
}

void net_socket_close(int sock)
{
    if(sock < 0)
        return;
    close(sock);
}

/**
 * @param port      0 - any, NET_SERVER_RECIEVER_PORT
 * @param in_addr   INADDR_ANY, INADDR_BROADCAST, <num>
 */
int net_addr_set(net_addr_t * net_addr, in_port_t port, in_addr_t in_addr)
{
    net_addr->addr_len = sizeof(net_addr->addr_in.sin_family)
            + sizeof(net_addr->addr_in.sin_port)
            + sizeof(net_addr->addr_in.sin_addr.s_addr);

    memset(&net_addr->addr, 0, sizeof(net_addr->addr));

    net_addr->addr_in.sin_family = AF_INET;
    //assert(port != 0 && "net_addr_set() port == 0");
    net_addr->addr_in.sin_port = htons(port);
    net_addr->addr_in.sin_addr.s_addr = htonl(in_addr);
    return 0;
}

int net_addr_set_fromstring(net_addr_t * net_addr, in_port_t port, const char * hostname)
{
    struct in_addr in_addr;
    int res = inet_aton( hostname, &in_addr );
    if(res == 0)
        return -1;
    return net_addr_set(net_addr, port, htonl(in_addr.s_addr));
}

/**
 * @param dest_addr     destination address
 */
void net_send(int sock, const void * buf, size_t buf_size, const net_addr_t * dest_addr)
{
    if( sendto(sock, buf, buf_size, NET_SENDTO_FLAGS, &dest_addr->addr, sizeof(dest_addr->addr)) < 0 )
    {
        game_halt("sendto()");
    }
}

/**
 *
 */
int net_recv(int sock, void * buf, size_t * buf_len, size_t buf_size, net_addr_t * src_addr)
{
    struct sockaddr * paddr;
    socklen_t * paddr_len;
    socklen_t addr_len;

    if(!src_addr)
    {
        paddr = NULL;
        paddr_len = NULL;
    }
    else
    {
        /* set addr_len before use */
        src_addr->addr_len = addr_len = sizeof(src_addr->addr);
        paddr = &src_addr->addr;
        paddr_len = &addr_len;
        memset(paddr, 0, addr_len);
    }
    ssize_t size = recvfrom(sock, buf, buf_size, NET_RECVFROM_FLAGS, paddr, paddr_len);
    if(size <= 0)
    {
        return -1;
    }
    if(addr_len > src_addr->addr_len)
    {
        game_console_send("WARNING: recvfrom() return truncated address.");
    }
    *buf_len = size;
    return 0;
}

