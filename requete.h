#ifndef REQUETE_H
#define REQUETE_H

#include "client_ipv6.h"
#include "serveur_ipv6.h"
#include "grid.h"

typedef struct CodereqIdEq {
    uint16_t all_in_one;
} CodereqIdEq;

typedef struct Codereq_5_6 {
    uint16_t num_action;
} Codereq_5_6;

typedef struct Codereq_7_8 {
    uint8_t len;
    char *data;
} Codereq_7_8;

typedef struct Codereq_9_10 {
    uint16_t portudp;
    uint16_t portmdiff;
    struct in6_addr addrmdiff;
} Codereq_9_10;

typedef struct Codereq_11 {
    uint16_t num;
    uint8_t hauteur;
    uint8_t largeur;
} Codereq_11;

typedef struct Codereq_12 {
    uint16_t num;
    uint8_t nb;
    char *data;
} Codereq_12;

typedef struct Codereq_13_14 {
    uint8_t len;
    char *data;
} Codereq_13_14;

typedef struct Codereq_15_16 {
    struct CodereqIdEq codereqid;
} Codereq_15_16;

extern struct CodereqIdEq init_codereqid(uint16_t code_req, uint16_t id, uint16_t eq);
extern void reverse_codereqid(CodereqIdEq codereqid, ServerResponse *response);
extern void reverse_codereqid_serv(CodereqIdEq codereqid, Client *client);
extern void reverse_codereq_9_10(Codereq_9_10 codereq, ServerResponse *response);
extern void recv_msg(Codereq_13_14 codereq, ServerResponse *response);
extern void recv_all_grid(char *buffer, ServerResponse *response);
extern void recv_min_grid(char *buffer, ServerResponse *response);
extern void send_action(char *buffer, ClientInfo *client);
extern void send_msg(char *buffer, uint8_t len, char *msg);
extern void send_9_10(char *buffer, Game *game);
extern void send_11(char *buffer, board *board, uint16_t num);
extern void send_12(char *buffer, board *board1, board *after, uint16_t num);
extern void reverse_action(Codereq_5_6 *codereq, Client *client);
extern int recv_requete(int fdsock, Game *game);


#endif