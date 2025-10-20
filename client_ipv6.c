#include "client_ipv6.h"

ClientInfo client = {0};
ServerResponse response = {0};
pthread_mutex_t lock_grid;
volatile sig_atomic_t terminate = 0;

bool still_alive(ServerResponse *b, int id){

    for (size_t i = 0; i < HEIGHT; i++) //joueur 1 est-il vivant
    {
      for (size_t j = 0; j < WIDTH; j++)
      {
        if(get_grid(b->board, j, i)==5+id){
                      return true;    
        }
      }
    }
    return false;
}
// Fonction pour envoyer une demande au serveur
void send_requestTCP(int fdsock)
{
    char *buffer = malloc(sizeof(CodereqIdEq));
    CodereqIdEq buf = init_codereqid(client.code_req, client.id, client.eq);
    memcpy(buffer, &buf, sizeof(buf));
    switch (client.code_req)
    {
    case 1:
    case 2:
    {
        int ecrit = send(fdsock, buffer, sizeof(CodereqIdEq), 0);
        if (ecrit < 0)
        {
            perror("Erreur d'écriture");
        }
        free(buffer);
        break;
    }
    case 3:
    case 4:
    {
        int ecrit = send(fdsock, buffer, sizeof(CodereqIdEq), 0);
        if (ecrit < 0)
        {
            perror("Erreur d'écriture");
        }
        free(buffer);
        break;
    }
    case 7:
    case 8:
    {
        send_msg(buffer, client.len, client.msg);
        int ecrit = send(fdsock, buffer, sizeof(CodereqIdEq)+sizeof(uint8_t)+256*sizeof(char), 0);
        if (ecrit < 0)
        {
            perror("Erreur d'écriture");
        }
        free(buffer);
        break;
    }
    default:
        break;
    }
}

void send_requestUDP(int fdsock, struct sockaddr_in6 adr){
    char *buffer = malloc(sizeof(CodereqIdEq));
    CodereqIdEq buf = init_codereqid(client.code_req, client.id, client.eq);
    memcpy(buffer, &buf, sizeof(buf));
    send_action(buffer, &client);
    int ecrit = sendto(fdsock, buffer, sizeof(CodereqIdEq) + sizeof(uint16_t), 0, (struct sockaddr *)&adr.sin6_addr, (socklen_t)sizeof(adr.sin6_addr));
    if (ecrit < 0)
    {
        terminate = 1;
        perror("Erreur d'écriture");
        close(fdsock);
    }
    free(buffer);
}

void receive_data(int fdsock){
    char *buffer = malloc(sizeof(CodereqIdEq) + 2*sizeof(uint16_t) + sizeof(struct in6_addr));
    int recu = recv(fdsock, buffer, sizeof(CodereqIdEq) + 2*sizeof(uint16_t) + sizeof(struct in6_addr), 0);
    if (recu < 0)
    {
        perror("Erreur de lecture 1");
        close(fdsock);
        exit(EXIT_FAILURE);
    }
    if (recu == 0)
    {
        terminate = 1;
        printf("Serveur hors ligne\n");
        close(fdsock);
        exit(EXIT_SUCCESS);
    }
    CodereqIdEq cie;
    memcpy(&cie, buffer, sizeof(cie));
    reverse_codereqid(cie, &response);
    struct Codereq_9_10 buf;
    memcpy(&buf, buffer + sizeof(CodereqIdEq), sizeof(buf));
    if(!response.is_init_addr){
        reverse_codereq_9_10(buf, &response);
    }
}

void receive_grid(int fdsock){

    char *buffer= malloc(4096 * sizeof(char));
    // printf("test avant recvfrom \n");
    int recu = recvfrom(fdsock, buffer, 4096* sizeof(char), 0, (struct sockaddr *)&response.diffadr, &response.difflen);
    // printf("test apres   recvfrom \n");
    if (recu < 0)
    {
        perror("Erreur de lecture 2");  
        close(fdsock);
        exit(EXIT_FAILURE);
    }
    if (recu == 0)
    {
        terminate = 1;
        printf("Serveur hors ligne\n");
        close(fdsock);
        exit(EXIT_SUCCESS);
    }
    // printf("recu : %d\n", recu);
    CodereqIdEq cie;
    memcpy(&cie, buffer, sizeof(cie));
    reverse_codereqid(cie, &response);
    if(response.code_req == 11){
        // printf("test avant recv_all_grid \n");
        recv_all_grid(buffer, &response);
    }else if(response.code_req == 12){
        // printf("test avant recv_min_grid \n");
        recv_min_grid(buffer, &response);
    }
    free(buffer);
}
void tchat_tcp(ServerResponse client){
    if(client.socktchat=socket(AF_INET6,SOCK_STREAM,0)<0){
        perror("socket creation error");
        // return EXIT_FAILURE;
    }

    memset(&client.tchat_serv,0,sizeof(client.tchat_serv));
    client.tchat_serv.sin6_family=AF_INET6;
    client.tchat_serv.sin6_port=htons(client.port_udp);


    if(inet_pton(AF_INET6,ADDR,&client)){
        perror("Invalid adress / adress not supported");
        // return EXIT_FAILURE;
    }

    if(connect(client.socktchat, (struct sockaddr *) &client.tchat_serv,sizeof(client.tchat_serv))<0){
        perror("connection failed");
        // return EXIT_FAILURE;
    }
}


int main(void)
{

    // Initialisation du mutex
    if (pthread_mutex_init(&lock_grid, NULL) != 0)
    {
        perror("Erreur lors de l'initialisation du mutex");
        exit(EXIT_FAILURE);
    }

    int fdsock = socket(PF_INET6, SOCK_STREAM, 0);
    // printf("fdsock Client server tcp : %d\n", fdsock);
    if (fdsock == -1)
    {
        perror("creation socket");
        exit(1);
    }

    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(PORT);
    inet_pton(AF_INET6, ADDR, &address_sock.sin6_addr);

    int r = connect(fdsock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r == -1)
    {
        perror("echec de la connexion");
        close(fdsock);
        exit(2);
    }

    
    uint16_t input;
    int result;
    do {
        printf("Quel type de partie (1 pour 4 adversaires, 2 pour le mode équipe) : ");
        result = scanf("%hu", &input);

        // Vider le buffer d'entrée si l'utilisateur a entré une valeur non numérique
        while (getchar() != '\n');

        if (result == 1) {
            client.code_req = input;
        } else {
            printf("Entrée invalide. Veuillez réessayer.\n");
        }
    } while (result != 1);
    client.id = 0;
    client.eq = 0;

    send_requestTCP(fdsock);
    receive_data(fdsock);
    printf("Reponse du serveur :\n");
    printf("Code de la requête : %hu\n", response.code_req);
    printf("ID attribué par le serveur : %hu\n", response.id);
    printf("Numéro d'équipe attribué : %hu\n", response.eq);
    printf("Port UDP : %hu\n", response.port_udp);
    printf("Port de multidiffusion : %hu\n", response.port_mdiff);
    char addr_str[INET6_ADDRSTRLEN];
    printf("Adresse de multidiffusion : %s\n", inet_ntop(AF_INET6, &(response.addr_mdiff), addr_str, INET6_ADDRSTRLEN));

    int rep;
    do {
        printf("Prêt à commencer la partie ? (1 pour oui, 0 pour non) : ");
        result = scanf("%d", &rep);

        // Vider le buffer d'entrée si l'utilisateur a entré une valeur non numérique
        while (getchar() != '\n');

        if (result == 1) {
            client.code_req = input;
        } else {
            printf("Entrée invalide. Veuillez réessayer.\n");
        }
    } while (result != 1);

    
    if (rep == 1)
    {
        // Création de la socket UDP
        int sock;
        if ((sock = socket(PF_INET6, SOCK_DGRAM, 0)) < 0)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        // Configuration de l'option SO_REUSEADDR
        int ok = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0)
        {
            perror("setsockopt SO_REUSEADDR");
            close(sock);
            exit(EXIT_FAILURE);
        }

        // Initialisation de l'adresse de réception
        struct sockaddr_in6 adr;
        memset(&adr, 0, sizeof(adr));
        adr.sin6_family = AF_INET6;
        adr.sin6_addr = in6addr_any;
        adr.sin6_port = htons(response.port_mdiff);
        // Lier la socket à l'adresse de réception
        if (bind(sock, (struct sockaddr *)&adr, sizeof(adr)) < 0)
        {
            perror("bind");
            close(sock);
            exit(EXIT_FAILURE);
        }
        // printf("socketfd Client server udp : %d\n", fdsock);
        // printf("socket Client server udp : %d\n", sock);
        char addr_str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(response.addr_mdiff), addr_str, INET6_ADDRSTRLEN);
        // S'abonner au groupe multicast
        struct ipv6_mreq group;
        memset(&group, 0, sizeof(group));
        inet_pton(AF_INET6, addr_str, &group.ipv6mr_multiaddr.s6_addr);
        int ifindex = if_nametoindex("eth0");
        if (ifindex == 0)
        {
            perror("if_nametoindex");
        }
        group.ipv6mr_interface = ifindex;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group)) < 0)
        {
            perror("setsockopt");
            close(sock);
            exit(EXIT_FAILURE);
        }
        // printf("test\n");
        if (response.code_req == 9)
        {
            client.code_req = 3;
            client.eq = 0;
            client.id = response.id;
            // printf("id : %d\n", client.id);
            send_requestTCP(fdsock);
        }
        else if (response.code_req == 10)
        {
            client.code_req = 4;
            client.eq = response.eq;
            client.id = response.id;
            client.is_team = true;
            // printf("id : %d\n", client.id);
            send_requestTCP(fdsock);
        }
    
        char tmp[28];
        recvfrom(sock, tmp, 28, 0, (struct sockaddr *)&adr, &response.difflen);
        printf("%s\n", tmp);

        pthread_mutex_lock(&lock_grid);
        receive_grid(sock);
        pthread_mutex_unlock(&lock_grid);
        // printf("test apres receive \n");
        int udp_sock = socket(PF_INET6, SOCK_DGRAM, 0);
        if (udp_sock == -1)
        {
            perror("creation socket UDP");
            exit(1);
        }

        struct sockaddr_in6 udp_address;
        memset(&udp_address, 0, sizeof(udp_address));
        udp_address.sin6_family = AF_INET6;
        udp_address.sin6_port = htons(response.port_udp);
        inet_pton(AF_INET6, ADDR, &udp_address.sin6_addr);
        int udp_r = connect(udp_sock, (struct sockaddr *)&udp_address, sizeof(udp_address));
        if (udp_r == -1)
        {
            perror("echec de la connexion UDP");
            close(udp_sock);
            exit(2);
        }
        tchat_tcp(response);
        
        line *l = malloc(sizeof(line));
        l->cursor = 0;
        pos *p = malloc(sizeof(pos));
        p->x = 0;
        p->y = 0;
     
        // // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
        initscr();                               /* Start curses mode */
        raw();                                   /* Disable line buffering */
        intrflush(stdscr, FALSE);                /* No need to flush when intr key is pressed */
        keypad(stdscr, TRUE);                    /* Required in order to get events from keyboard */
        nodelay(stdscr, TRUE);                   /* Make getch non-blocking */
        noecho();                                /* Don't echo() while we do getch (we will manually print characters when relevant) */
        curs_set(0);                             // Set the cursor to invisible
        start_color();                           // Enable colors
        init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)


        int num = 0;
        bool alive=true;

        // printf("test avant boucle \n");
        while (!terminate)
        {
            if(still_alive(&response, client.id)==false){
                alive=false;
                //printf("mort");
            }
            if(alive==true){
                ACTION a = control(l);
                if(a != NONE){
                    client.code_req = 5;
                    if(client.is_team){
                        client.code_req = 6;
                    }
                    client.num = num%(int)pow(2, 16);
                    client.num_action = a;
                    pthread_mutex_lock(&lock_grid);
                    send_requestUDP(udp_sock, udp_address);
                    pthread_mutex_unlock(&lock_grid);
                    if(a==SEND){
                        //fonction d'envoie de message de david
                        client.msg = malloc(256*sizeof(char));
                        memset(client.msg, 0, 256*sizeof(char));
                        memcpy(client.msg, l->data, strlen(l->data) * sizeof(char));
                        client.len = strlen(l->data);
                        if(client.is_team){
                            client.code_req = 7;
                        }else{
                            client.code_req = 8;
                        } 
                        send_requestTCP(response.socktchat);
                    }
                }
            }
                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(sock, &readfds);
                struct timeval timeout;
                timeout.tv_sec = 0;  // Attendre 1 seconde
                timeout.tv_usec = 10000;
                int ret = select(sock + 1, &readfds, NULL, NULL, &timeout);
            if (ret > 0){
        // printf("test avant receive \n");
                    pthread_mutex_lock(&lock_grid);
                    // printf("test avant receive \n");
                    receive_grid(sock);
                    // printf("test apres receive \n");
                    pthread_mutex_unlock(&lock_grid);
                    refresh_game(response.board, l, p);
                    // printf("test apres refresh \n");
            }else if (ret == 0) {
                refresh_game(response.board, l, p);
        // Le délai d'attente est dépassé, vérifiez la condition de la boucle
                continue;
            } else {
                // Une erreur s'est produite
                perror("select 1");
                break;
            }
            
        }

        curs_set(1); // Set the cursor to visible
        endwin();    // End curses mode

        free(l);
        free(p);
       
        close(udp_sock);
        close(sock);
    }

    close(fdsock);
    pthread_mutex_destroy(&lock_grid);
    return 0;
}
