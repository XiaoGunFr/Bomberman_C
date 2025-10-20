#ifndef CLIENT_IPV6_H
#define CLIENT_IPV6_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include "grid.h"
#include <math.h>
#include <termio.h>


#define PORT 7878        // Numéro de port du serveur
#define ADDR "localhost" // Adresse du serveur
#define SIZE_MESS 100

typedef struct IPV6
{
    struct in6_addr ipv6mr_multiaddr;
    unsigned int ipv6mr_interface;
} ipv6_mreq;

typedef struct IPV4
{ // pour ipv4 a garder au cas ou on enlévera a la fin au pire ok david ? dsl pour l'opti
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
} ip_mreqn;

typedef struct ClientInfo
{
    uint16_t code_req;
    uint16_t id;
    uint16_t eq;
    bool is_team;
    int num;
    int num_action;
    uint8_t len;
    char *msg;
} ClientInfo;

typedef struct ServerResponse
{
    uint16_t code_req;
    uint16_t id;
    uint16_t eq;
    uint16_t port_udp;
    uint16_t port_mdiff;
    bool is_init_addr;
    struct in6_addr addr_mdiff;
    struct sockaddr_in6 diffadr;
    struct sockaddr_in6 tchat_serv;
    int socktchat;
    socklen_t difflen;
    uint16_t num_grid_full;
    uint16_t num_grid_part;
    char *msg;
    bool is_init_board;
    board *board;
    char *modif_case;
} ServerResponse;

extern ClientInfo client;
extern ServerResponse response;

void tchat_tcp(ServerResponse client);
extern bool still_alive(ServerResponse *b, int id);
extern void send_requestTCP(int fdsock);
extern void send_requestUDP(int fdsock, struct sockaddr_in6 adr);
extern void *client_thread(void *arg);
#include "requete.h"

#endif