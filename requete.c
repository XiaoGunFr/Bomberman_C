#include "requete.h"

CodereqIdEq init_codereqid(uint16_t code_req, uint16_t id, uint16_t eq){
    struct CodereqIdEq codereqid;
    codereqid.all_in_one = htons((code_req << 3) | (id << 1) | eq);
    return codereqid;
}

void reverse_codereqid(CodereqIdEq codereqid, ServerResponse *response){
    uint16_t all_in_one = codereqid.all_in_one;
    all_in_one = ntohs(all_in_one);
    response->code_req = (all_in_one >> 3) & 0x1FFF;
    response->id = (all_in_one >> 1) & 0x3;
    response->eq = all_in_one & 0x1;
}

void reverse_codereqid_serv(CodereqIdEq codereqid, Client *client){
    uint16_t all_in_one = codereqid.all_in_one;
    all_in_one = ntohs(all_in_one);
    client->req = (all_in_one >> 3) & 0x1FFF;
    client->id = (all_in_one >> 1) & 0x3;
    client->equipe = all_in_one & 0x1;
}

void reverse_codereq_9_10(Codereq_9_10 codereq, ServerResponse *response){
    response->port_udp = ntohs(codereq.portudp);
    response->port_mdiff = ntohs(codereq.portmdiff);
    response->addr_mdiff = codereq.addrmdiff;
    response->is_init_addr = true;
}

void recv_msg(Codereq_13_14 codereq, ServerResponse *response){
    if(response->msg != NULL){
        free(response->msg);
    }
    response->msg = (char*)malloc(sizeof(codereq.data));
    memcpy(response->msg, codereq.data, sizeof(&codereq.data));

}

void recv_all_grid(char *buffer, ServerResponse *response){
    if(!response->is_init_board){
        response->board = (board*)malloc(sizeof(board));
        uint16_t num;
        memcpy(&num, buffer + sizeof(uint16_t), sizeof(uint16_t));
        response->num_grid_full = ntohs(num);
        memcpy(&num, buffer + 2*sizeof(uint16_t), sizeof(uint16_t));
        response->board->h = num >> 8;
        response->board->w = num & 0xFF;
        response->board->grid = (char*)malloc(response->board->h * response->board->w * sizeof(char));
        response->is_init_board = true;
    }
    memcpy(response->board->grid, buffer + 3*sizeof(uint16_t), response->board->h * response->board->w * sizeof(char));
    // printf("teste\n");
    // for(int i = 0; i < response->board->h; i++){
    //     for(int j = 0; j < response->board->w; j++){
    //         printf("%d ", get_grid(response->board, j, i));
    //     }
    //     printf("\n");
    // }
}

void recv_min_grid(char *buffer, ServerResponse *response){
    // free(response->modif_case);
    uint16_t num;
    memcpy(&num, buffer + sizeof(uint16_t), sizeof(uint16_t));
    response->num_grid_part = ntohs(num);
    uint8_t nb;
    memcpy(&nb, buffer + 2*sizeof(uint16_t), sizeof(uint8_t));
    char *tmp = (char*)malloc(nb * 3 * sizeof(char));
    memcpy(tmp, buffer + 2*sizeof(uint16_t) + sizeof(uint8_t), nb * 3 * sizeof(char));
    for(int i = 0; i < nb; i++){
        int x = tmp[i*3];
        int y = tmp[i*3 + 1];
        int val = tmp[i*3 + 2];
        set_grid(response->board, y, x, val);
    }
}

void send_action(char *buffer , ClientInfo *client){
    uint16_t num_action = htons((client->num << 3) | client->num_action);
    char *r = realloc(buffer, 2*sizeof(uint16_t));
    if (r == NULL){
        perror("realloc");
        free(buffer);
        return;
    }
    buffer = r;
    memcpy(buffer + sizeof(CodereqIdEq), &num_action, sizeof(uint16_t));
}

// Adapte le buffer pour envoyer un message
void send_msg(char *buffer, uint8_t len, char *msg){
    char *new_buffer = realloc(buffer, sizeof(CodereqIdEq) + 257*sizeof(char));
    if (new_buffer == NULL){
        perror("realloc");
        free(buffer);
        return;
    }
    buffer = new_buffer;
    memcpy(buffer + sizeof(CodereqIdEq), &len, sizeof(u_int8_t));
    memcpy(buffer + sizeof(CodereqIdEq) + sizeof(u_int8_t), msg, sizeof(char)*len);
}

void send_9_10(char *buffer, Game *game){
    char *r = realloc(buffer, sizeof(CodereqIdEq) + 2*sizeof(uint16_t) + sizeof(struct in6_addr));
    if (r == NULL){
        perror("realloc");
        free(buffer);
        return;
    }
    buffer = r;
    uint16_t port_udp = htons(game->port_udp);
    uint16_t portmdiff = htons(game->port_mdiff);
    struct in6_addr addrmdiff = game->addr.sin6_addr;
    memcpy(buffer + sizeof(CodereqIdEq), &port_udp, sizeof(uint16_t));
    memcpy(buffer + sizeof(CodereqIdEq) + sizeof(uint16_t), &portmdiff, sizeof(uint16_t));
    memcpy(buffer + sizeof(CodereqIdEq) + 2*sizeof(uint16_t), &addrmdiff, sizeof(struct in6_addr));
}

void send_11(char *buffer, board *board, uint16_t num){
    // char *r = realloc(buffer, sizeof(CodereqIdEq) + 2*sizeof(uint16_t) + 3 * board->h * board->w * sizeof(char));
    // if (r == NULL){
    //     perror("realloc");
    //     free(buffer);
    //     return;
    // }
    // buffer = r;
    num = htons(num);
    uint16_t hl = (board->h << 8 | board->w);
    memcpy(buffer + sizeof(CodereqIdEq), &num, sizeof(uint16_t));
    memcpy(buffer + sizeof(CodereqIdEq) + sizeof(uint16_t), &hl, sizeof(uint16_t));
    memcpy(buffer + sizeof(CodereqIdEq) + 2*sizeof(uint16_t), board->grid, board->h * board->w * sizeof(char));
}

void send_12(char *buffer, board *board1, board *after, uint16_t num){
    // char *r = realloc(buffer, sizeof(CodereqIdEq) + sizeof(uint16_t) + sizeof(uint8_t) + 3 * board1->h * board1->w * sizeof(char));
    // if (r == NULL){
    //     perror("realloc");
    //     free(buffer);
    //     return;
    // }
    // buffer = r;
    num = htons(num);
    uint8_t nb = 0;
    for (int j = 0; j < board1->h; j++){
        for (int i = 0; i < board1->w; i++){
            int tmp = get_grid(after, i, j);
            if (get_grid(board1, i, j) != tmp){
                memcpy(buffer + sizeof(CodereqIdEq) + sizeof(uint16_t) + sizeof(uint8_t) + nb * 3, &i, sizeof(uint8_t));
                memcpy(buffer + sizeof(CodereqIdEq) + sizeof(uint16_t) + sizeof(uint8_t) + nb * 3 + 1, &j, sizeof(uint8_t));
                memcpy(buffer + sizeof(CodereqIdEq) + sizeof(uint16_t) + sizeof(uint8_t) + nb * 3 + 2, &tmp, sizeof(uint8_t));
                nb++;
            }
        }
    }
    memcpy(buffer + sizeof(CodereqIdEq), &num, sizeof(uint16_t));
    memcpy(buffer + sizeof(CodereqIdEq) + sizeof(uint16_t), &nb, sizeof(uint8_t));
}



void reverse_action(Codereq_5_6 *codereq, Client *client){
    uint16_t num_action = codereq->num_action;
    num_action = ntohs(num_action);
    client->num = (num_action >> 3) & 0x1FFF;
    client->action = num_action & 0x7;
}