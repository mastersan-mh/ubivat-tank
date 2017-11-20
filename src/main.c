/*
 * Ubivat Tank
 * Главное тело
 * by Master San
 */

#include "game.h"

//#include <fonts.h>


void listenForPackets(void)
{

#define PORT_SRC 40000
#define PORT_DEST 40001

    net_addr_t dest;
    //net_addr_set(&dest, PORT_DEST, INADDR_BROADCAST);
    //net_addr_set_fromstring(&dest, PORT_DEST, "127.0.0.1");
    //net_addr_set_fromstring(&dest, PORT_DEST, "192.168.1.5");
    //net_addr_set_fromstring(&dest, PORT_DEST, "192.168.7.255");
    net_addr_set_fromstring(&dest, PORT_DEST, "255.255.255.255");

    net_addr_t sender;
    net_addr_set(&sender, PORT_SRC, INADDR_ANY);

    net_addr_t receiver;
    net_addr_set(&receiver, PORT_DEST, INADDR_ANY);

    int sock_sender = net_socket_open_connectionless(&sender);
    if(sock_sender < 0)
        game_halt("net_socket_open_udp() failed\n");

    int sock_receiver = net_socket_open_connectionless(&receiver);
    if(sock_receiver < 0)
        game_halt("net_socket_open_udp() failed\n");

    char buf[128];
    size_t buf_len;
    net_send(sock_sender, buf, 4, &dest);

    int res = net_recv(sock_receiver, buf, &buf_len, 128, &dest);
    printf("net_recv(): res = %d, buf_len = %d\n", res, (int)buf_len);

    net_socket_close(sock_sender);
    net_socket_close(sock_receiver);

}

int main(int argc, char ** argv)
{
    //      fonts_test();
//#define STANDALONETEST
#ifdef STANDALONETEST
    listenForPackets();
#else
    game_init();
    game_main();
    game_done();
#endif

    return 0;
}
