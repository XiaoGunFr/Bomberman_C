#ifndef SERVEUR_IPV6_H
#define SERVEUR_IPV6_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "grid.h"
#include <signal.h>
#include <sys/select.h>

#define SIZE_MESS 100
#define PORT_UDP_DEFAULT 5000
#define ADDR_MDIFF_DEFAULT "ff12::1"
#define WIDTH 10
#define HEIGHT 10

typedef struct Client
{
    int sock;      // Descripteur de socket du client
    uint16_t req;
    uint16_t id;        // Identifiant du client
    uint16_t equipe;    // Équipe du client
    bool is_ready; // Indique si le client est prêt
    int num;
    int action;
    uint8_t len;
    char *data;
    int sock_msg;
} Client;

typedef struct Game
{
    int id;
    int mode;
    int nbr_players;
    int players_ready;
    Client *players[4];
    pos position[4];
    bool alive[4];
    struct Game *next;
    uint16_t port_udp;
    uint16_t port_mdiff;
    struct sockaddr_in6 addr;
    struct sockaddr_in6 tchat_addr;
    int socktchat;
    int sockfd;
    board *grid;
    bool is_end;
    int fin_partie;
} Game;
typedef struct affichage
{
    board *board;
    line *line;
    pos *pos;
} affichage;

extern Game *games_head;
extern int game_id;

extern int one_alive(Game game);
void tcp_chat(Game *game);
void *tchat_handler(void *arg);
int team_alive(Game game);
extern void id_missing(Game *b);
extern void setup_pos_alive(Game *game);
extern int recv_requete(int fdsock, Game *game);
extern void send_grid(int sock, struct sockaddr_in6 mutlicastAddr, board *board, uint16_t num);
extern void *game_launcher(void *arg);
extern void start_server();
extern void free_clients(Client *head);
extern void free_game(Game *games_head);
extern Game *find_waiting_game(int mode);
extern Game *create_game(int mode);
extern void add_player_to_game(Game *game, int sock, Client *client);
extern void send_response_data(int fdsock, Game *game, Client *client);
extern void client_handler(int sockclient);
extern void *client_thread(void *arg);
extern void send_grid_freq(int sock, struct sockaddr_in6 multicastAddr, board *board1, board *after, uint16_t num);
extern void *send_grid_part(void *arg);

#include "requete.h"

#endif