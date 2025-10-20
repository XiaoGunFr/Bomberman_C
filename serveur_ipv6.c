#include "serveur_ipv6.h"

Game *games_head = NULL;
int game_id = 0;
// Variable statique pour générer des ports multicast uniques
static int next_multicast_port = 5000;
static int default_udp = 3000;
pthread_mutex_t lock_grid;
pthread_mutex_t lock_create_game;
pthread_mutex_t end_game;

pthread_t suivi[50];
static int nb_thread = 0;

volatile sig_atomic_t terminate = 0;
void handle_sigint(int sig) {
    terminate = 1;
}

//vérifie que chaque joueur soit mort ou pas 
void id_missing(Game *b){
  bool alive=false;
    for (size_t i = 0; i < HEIGHT; i++) //joueur 1 est-il vivant
    {
      for (size_t j = 0; j < WIDTH; j++)
      {
        if(get_grid(b->grid, j, i)==5){
                      alive=true;    
        }
      }
    }
    b->alive[0]=alive;
    alive=false;
    for (size_t i = 0; i < HEIGHT; i++) //joueur 1 est-il vivant
    {
      for (size_t j = 0; j < WIDTH; j++)
      {
        if(get_grid(b->grid, j, i)==6){
                      alive=true;    
        }
      }
    }
    b->alive[1]=alive;
    alive=false;

    for (size_t i = 0; i < HEIGHT; i++) //joueur 1 est-il vivant
    {
      for (size_t j = 0; j < WIDTH; j++)
      {
        if(get_grid(b->grid, j, i)==7){
                      alive=true;    
        }
      }
    }
    b->alive[2]=alive;
    alive=false;
    for (size_t i = 0; i < HEIGHT; i++) //joueur 1 est-il vivant
    {
      for (size_t j = 0; j < WIDTH; j++)
      {
        if(get_grid(b->grid, j, i)==8){
                      alive=true;    
        }
      }
    }
    b->alive[3]=alive;

}
// Fonction pour générer un port multicast unique
int generate_unique_multicast_port()
{
  return next_multicast_port++;
}
int generate_unique_udp_port()
{
  return default_udp++;
}

// Fonction pour créer une nouvelle partie
Game *create_game(int mode)
{
  Game *new_game = malloc(sizeof(Game));
  if (new_game == NULL)
  {
    perror("Erreur d'allocation de mémoire pour la nouvelle partie");
    exit(EXIT_FAILURE);
  }
  new_game->id = game_id++;
  new_game->mode = mode;
  new_game->nbr_players = 0;
  new_game->players_ready = 0;
  new_game->next = games_head;
  games_head = new_game;
  for (int i = 0; i < 4; i++)
  {
    new_game->players[i] = NULL;
  }

  // Initialiser les ports
  new_game->port_udp = generate_unique_udp_port();
  new_game->port_mdiff = generate_unique_multicast_port();

  // Créer la socket de multidiffusion
  new_game->sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (new_game->sockfd < 0)
  {
    perror("Erreur lors de la création de la socket de multidiffusion");
    exit(EXIT_FAILURE);
  }

  // Liaison de la socket à l'adresse de multidiffusion et au port de multidiffusion

  memset(&(new_game->addr), 0, sizeof(new_game->addr));
  new_game->addr.sin6_family = AF_INET6;
  inet_pton(AF_INET6, ADDR_MDIFF_DEFAULT, &(new_game->addr).sin6_addr);
  new_game->addr.sin6_port = htons(new_game->port_mdiff);
  int ifindex = if_nametoindex("eth0");
  if (ifindex == 0)
  {
    perror("if_nametoindex");
  }

  new_game->addr.sin6_scope_id = ifindex; // ou 0 pour interface par défaut

  return new_game;
}

// Ajouter un client à une partie
void add_player_to_game(Game *game, int sock, Client *player)
{
  if (player == NULL)
  {
    perror("Erreur d'allocation de mémoire pour le nouveau client");
    exit(EXIT_FAILURE);
  }
  player->sock = sock;
  // On donne un id au joueur
  player->id = game->nbr_players;

  // Si il a choisi 4 adversaires, pas d'équipe
  if (game->mode == 1)
  {
    player->equipe = 0;
  }
  // Sinon, on lui attribue une équipe
  else
  {
    // 0
    if (game->nbr_players < 2)
    {
      player->equipe = 0;
    }
    // Ou 1
    else
    {
      player->equipe = 1;
    }
  }
  game->players[game->nbr_players] = player;
  game->nbr_players++;
}

// Fonction pour trouver une partie en attente pour un mode donné
Game *find_waiting_game(int mode)
{
  Game *current_game = games_head;

  // Tant qu'on ne trouve pas de partie en attente avec le bon mode, on continue
  while (current_game != NULL)
  {
    if (current_game->mode == mode && current_game->nbr_players < 4)
    {
      return current_game;
    }
    current_game = current_game->next;
  }
  return NULL;
}

// Fonction pour libérer la mémoire occupée par la liste chaînée des clients
void free_clients(Client *head)
{
  close(head->sock);
  free(head);
}

// Fonction pour libérer la mémoire occupée par la liste chaînée des parties
void free_game(Game *games_head)
{
  Game *current = games_head;
  while (current != NULL)
  {
    Game *next = current->next;
    free(current);
    current = next;
  }
  games_head = NULL;
}

void client_handler(int sockclient)
{
  Client *client = malloc(sizeof(Client));
  while (!terminate)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockclient, &readfds);
    struct timeval timeout;
    timeout.tv_sec = 1;  // Attendre 1 seconde
    timeout.tv_usec = 0;
    int ret = select(sockclient + 1, &readfds, NULL, NULL, &timeout);
  if (ret > 0) {
    CodereqIdEq codereqid;
    // Réception des données du client (premier message)
    int byte_received = recv(sockclient, &codereqid, sizeof(codereqid), 0);
    // printf("byte_received %d\n", byte_received);
    if (byte_received == -1)
    {
      perror("Erreur de lecture");
      close(sockclient);
      exit(EXIT_FAILURE);
    }
    else if (byte_received == 0)
    {
      printf("Client deconnecté\n");
      close(sockclient);
      break;
    }
    else
    {
      reverse_codereqid_serv(codereqid, client);
      // printf("Client %d connecté\n", client->id);
      // printf("Equipe %d\n", client->equipe);
      // printf("Mode %d\n", client->req);
      Game *waiting_game;
      if (client->req == 1 || client->req == 2)
      {
        pthread_mutex_lock(&lock_create_game);
        // Recherche d'une partie en attente pour le mode demandé
        waiting_game = find_waiting_game(client->req);

        // Si une partie en attente est trouvée on ajoute le client
        if (waiting_game != NULL)
        {
          add_player_to_game(waiting_game, sockclient, client);
          printf("Client ajouté à la partie numéro %d en mode %s\n", waiting_game->id, (client->req == 1) ? "4 adversaires" : "equipe");
        }
        else
        {
          // Sinon on en crée une nouvelle
          waiting_game = create_game(client->req);
          add_player_to_game(waiting_game, sockclient, client);
          printf("Nouvelle partie pour le mode %s\n", (client->req == 1) ? "4 adversaires" : "equipe");
        }

        // Envoi du des informations attribuées au joueur + infos sur multidiffusion
        send_response_data(sockclient, waiting_game, waiting_game->players[waiting_game->nbr_players - 1]);
        pthread_mutex_unlock(&lock_create_game);
      }
      // Si on reçoit un message de client prêt

      else if (client->req == 3 || client->req == 4)
      {
        // printf("id %d\n", id);
        waiting_game->players[client->id]->is_ready = true;
        waiting_game->players_ready++;
        printf("Joueur %d est préparé pour la partie numero %d en mode %s\n", client->id, waiting_game->id, (waiting_game->mode == 1) ? "4 adversaires" : "equipe");
      }
      else
      {
        printf("code_req invalide\n");
      }
      // On vérifie si tous les joueurs sont présents et prêts
      if (waiting_game->players_ready == 4)
      {
        // Création d'un thread pour gérer le multicast et lancer la partie
        pthread_t threadClient;
        if (pthread_create(&threadClient, NULL, game_launcher, waiting_game) != 0)
        {
          perror("Erreur lors de la création du thread client");
          free(waiting_game);
          close(sockclient);
        }
        suivi[nb_thread] = threadClient;
        nb_thread++;
        printf("Tous les joueurs sont présents pour la partie numero %d\n", waiting_game->id);
        // Il faudrait multidiffuser ici
        break;
      }
      printf("Partie en attente numero %d en mode %s contient actuellement %d joueurs et %d sont prêts\n\n", waiting_game->id, (waiting_game->mode == 1) ? "4 adversaires" : "equipe", waiting_game->nbr_players, waiting_game->players_ready);
    }
  
    }else if (ret == 0) {
          // Le délai d'attente est dépassé, vérifiez la condition de la boucle
          continue;
    } else {
          // Une erreur s'est produite
          perror("select 3 ");
          break;
    }
  }
  // Fermeture de la socket client
  close(sockclient);
}
int one_alive(Game game){
  if(game.alive[0]==true && game.alive[1]==false && game.alive[2]==false && game.alive[3]==false){
    return 0; //joueur 0 dernier vivant
  }
  if(game.alive[0]==false && game.alive[1]==true && game.alive[2]==false && game.alive[3]==false){
    return 1; // joueur 1 dernier vivant
  }
  if(game.alive[0]==false && game.alive[1]==false && game.alive[2]==true && game.alive[3]==false){
    return 2; //joueur 2 dernier vivant
  }
  if(game.alive[0]==false && game.alive[1]==false && game.alive[2]==false && game.alive[3]==true){
    return 3; // joueur 3 dernier vivant
  }
  return -1; //il reste plus d'un joueur en vie, la partie continue
}
int team_alive(Game game){
  if((game.alive[0]==true || game.alive[1]==true) && (game.alive[2]==false && game.alive[3]==false)){
    return 0; //equipe 1 a gagné
  }
   if((game.alive[2]==true || game.alive[3]==true) && (game.alive[0]==false && game.alive[1]==false)){
    return 1; //equipe 2 a gagné
  }
  return -1; //aucune équipe a gagné, la partie continue
}
void *send_grid_thread(void *arg)
{
  Game *waiting_game = (Game *)arg;
  uint16_t num = 0;
  while (!terminate && !waiting_game->is_end)
  {

    pthread_mutex_lock(&lock_grid);
    send_grid(waiting_game->sockfd, waiting_game->addr, waiting_game->grid, num);
    pthread_mutex_unlock(&lock_grid);
  
    num = (num + 1) % (uint16_t)pow(2, 16);
    sleep(1);

    // usleep(30 * 1000);
  }
  return EXIT_SUCCESS;
}

void *send_grid_part(void *arg){
  Game *waiting_game = (Game *)arg;
  board * after = malloc(sizeof(board));
  after->grid = malloc(HEIGHT * WIDTH * sizeof(char));
  pthread_mutex_lock(&lock_grid);
  memcpy(after->grid, waiting_game->grid->grid, HEIGHT * WIDTH * sizeof(char));
  pthread_mutex_unlock(&lock_grid);
  uint16_t num = 0;
  while(!terminate && !waiting_game->is_end){
    pthread_mutex_lock(&lock_grid);
    send_grid_freq(waiting_game->sockfd, waiting_game->addr, after, waiting_game->grid, num);
    pthread_mutex_unlock(&lock_grid);
    num = (num + 1) % (uint16_t)pow(2, 16);
    usleep(30 * 1000);
    pthread_mutex_lock(&lock_grid);
    memcpy(after->grid, waiting_game->grid->grid, HEIGHT * WIDTH * sizeof(char));
    pthread_mutex_unlock(&lock_grid);
  }
  free(after->grid);
  free(after);
  return NULL;
}

void setup_pos_alive(Game *game){
  game->position[0].x=0;
  game->position[0].y=0;
  game->position[1].x=0;
  game->position[1].y=WIDTH-1;
  game->position[2].x=HEIGHT-1;
  game->position[2].y=0;
  game->position[3].x=HEIGHT-1;
  game->position[3].y=WIDTH-1;
  game->alive[0]=true;
  game->alive[1]=true;
  game->alive[2]=true;
  game->alive[3]=true;
}


// void tcp_chat(Game *game){
//   if((game->socktchat=socket(AF_INET6,SOCK_STREAM,0))==0){
//     perror("sockTchat failed");
//     exit(EXIT_FAILURE);
//   }
//   int opt=1;
//   if(setsockopt(game->socktchat,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))){
//     perror("setsockopt");
//     close(game->socktchat);
//     exit(EXIT_FAILURE);
//   }
//   game->tchat_addr.sin6_family =AF_INET6;
//   game->tchat_addr.sin6_addr=in6addr_any;
//   game->tchat_addr.sin6_port =htons(game->port_udp);

//   if(bind(game->socktchat,(struct sockaddr *)&game->tchat_addr,sizeof(&game->tchat_addr))<0){
//     perror("bind failed");
//     close(game->socktchat);
//     exit(EXIT_FAILURE);
//   }
//   if(listen(game->socktchat,4)<0){
//     perror("listen failed");
//     close(game->socktchat);
//     exit(EXIT_FAILURE);
//   }
//   int new_socket;
//   new_socket=malloc(sizeof(int));
//   int cpt = 0;
//   while(!terminate){
//     if((new_socket=accept(game->socktchat,(struct sockaddr *)&game->tchat_addr,sizeof(game->tchat_addr)))<0){
//       perror("accept failed");
//       free(new_socket);
//       continue;
//     }
//     pthread_t thread_tchat;
//     if (pthread_create(&thread_tchat, NULL, thread_msg, game) != 0){
//       perror("Erreur lors de la création du thread client");
//       free(new_socket);
//       close(new_socket);
//     }
//     cpt++;
//   }
//   close(game->socktchat);
// }

// void* thread_msg(void *arg){
//   Game *game = (Game *)arg;
  
//   return NULL;
// }
void *game_launcher(void *arg)
{

    Game *waiting_game = (Game *)arg;
    if (sendto(waiting_game->sockfd, "Vos adversaires sont prêts", 28, 0, (struct sockaddr *)&waiting_game->addr, sizeof(waiting_game->addr)) < 0)
    {
        printf("erreur send\n");
    }
    board *grid = malloc(sizeof(board));
    setup_board(grid, HEIGHT, WIDTH);
    setup_pos_alive(waiting_game);
    waiting_game->grid = grid;
    for (size_t i = 0; i < HEIGHT; i++)
    {
        for (size_t j = 0; j < WIDTH; j++)
        {
            printf("%d", get_grid(grid, j, i)) ;
        }
        printf("\n");
    }
    
    pthread_t thread_grid, thread_part;

    // printf("test avant seng_gridthread \n");
    pthread_create(&thread_grid, NULL, send_grid_thread, waiting_game);
    // printf("test aprés seng_gridthread \n");
    sleep(2);
    pthread_create(&thread_part, NULL, send_grid_part, waiting_game);
    suivi[nb_thread] = thread_grid;
    nb_thread++;
    int id_player;
    // printf("waiting_game->sockfd %d\n", waiting_game->sockfd);
    int udp_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (udp_sock < 0)
    {
        perror("Erreur lors de la création de la socket de multidiffusion");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in6 udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin6_family = AF_INET6;
    udp_addr.sin6_port = htons(waiting_game->port_udp);
    udp_addr.sin6_addr = in6addr_any;
    if (bind(udp_sock, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
    {
        perror("Erreur lors de la liaison de la socket au port");
        close(udp_sock);
        exit(EXIT_FAILURE);
    }

    while (!terminate)
    {

      fd_set readfds;
      FD_ZERO(&readfds);
      FD_SET(udp_sock, &readfds);
      struct timeval timeout;
      timeout.tv_sec = 1;  // Attendre 1 seconde
      timeout.tv_usec = 0;
      int ret = select(udp_sock + 1, &readfds, NULL, NULL, &timeout);
      if (ret > 0){
        id_player=recv_requete(udp_sock,waiting_game);
        for (size_t i = 0; i < HEIGHT; i++)
        {
            for (size_t j = 0; j < WIDTH; j++)
            {
                printf("%d", get_grid(grid, j, i)) ;
            }
            printf("\n");
        }
        printf("\n");
        printf("%s \n",waiting_game->alive[0] ? "true": "false");
        printf("%s \n",waiting_game->alive[1] ? "true": "false");
        if (perform_action(waiting_game->grid,&waiting_game->position[id_player], waiting_game->players[id_player]->action,id_player)){
          break;
        }
        id_missing(waiting_game);
        //je pars du principe que 1 c'est mode 4 joueurs et 0 mode équipe
        if(waiting_game->mode==1){
          waiting_game->fin_partie=one_alive(*waiting_game);
        }else{
          waiting_game->fin_partie=team_alive(*waiting_game);
        }
      } else if (ret == 0) {
          // Le délai d'attente est dépassé, vérifiez la condition de la boucle
          id_missing(waiting_game);
          //je pars du principe que 1 c'est mode 4 joueurs et 0 mode équipe
          if(waiting_game->mode==1){
            waiting_game->fin_partie=one_alive(*waiting_game);
          }else{
            waiting_game->fin_partie=team_alive(*waiting_game);
          }
          continue;
      } else {
          // Une erreur s'est produite
          perror("select 2");
          break;
      }
      if(waiting_game->fin_partie!=-1){
        waiting_game->is_end=true;
        printf("Partie terminée\n");
        break;
      }
    }
    // pthread_mutex_lock(&lock_grid);
    // // Partie pour l'écriture dans la grille
    // pthread_mutex_unlock(&lock_grid);
  return NULL;
}

void *client_thread(void *arg)
{
  int sockclient = *((int *)arg);
  client_handler(sockclient);
  free(arg);
  return NULL;
}

void start_server()
{
  signal(SIGINT, handle_sigint);
  // Initialisation du mutex
  if (pthread_mutex_init(&lock_grid, NULL) != 0 || pthread_mutex_init(&lock_create_game, NULL) != 0 || pthread_mutex_init(&end_game, NULL) != 0)
  {
    perror("Erreur lors de l'initialisation du mutex");
    exit(EXIT_FAILURE);
  }

    int server_socket, sockclient;
    struct sockaddr_in6 address_sock, adrclient;
    // Création de la socket serveur
    server_socket = socket(PF_INET6, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("Erreur lors de la configuration de SO_REUSEADDR");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

  // Initialisation de l'adresse du serveur
  memset(&address_sock, 0, sizeof(address_sock));
  address_sock.sin6_family = AF_INET6;
  address_sock.sin6_port = htons(PORT);
  address_sock.sin6_addr = in6addr_any;

  // Liaison de la socket au port spécifié
  if (bind(server_socket, (struct sockaddr *)&address_sock, sizeof(address_sock)) < 0)
  {
    perror("Erreur lors de la liaison de la socket au port");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  // Attente des connexions entrantes
  if (listen(server_socket, SOMAXCONN) < 0)
  {
    perror("Erreur lors de l'attente des connexions entrantes");
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  printf("Attente de la demande du client...\n");
  while (!terminate)
  {
    // printf("trminated %d\n", terminate);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server_socket, &readfds);
    struct timeval timeout;
    timeout.tv_sec = 1;  // Attendre 1 seconde
    timeout.tv_usec = 0;
    int ret = select(server_socket + 1, &readfds, NULL, NULL, &timeout);

    if(ret > 0){
      memset(&adrclient, 0, sizeof(adrclient));
      socklen_t size = sizeof(adrclient);
      sockclient = accept(server_socket, (struct sockaddr *)&adrclient, &size);
      if (sockclient == -1)
      {
        perror("Erreur lors de l'acceptation de la connexion");
        exit(EXIT_FAILURE);
      }

      // Création d'un thread pour gérer le client
      pthread_t threadClient;
      int *client_sock = malloc(sizeof(int));
      if (client_sock == NULL)
      {
        perror("Erreur lors de l'allocation mémoire");
        close(sockclient);
        continue;
      }
      *client_sock = sockclient;

      if (pthread_create(&threadClient, NULL, client_thread, client_sock) != 0)
      {
        perror("Erreur lors de la création du thread client");
        free(client_sock);
        close(sockclient);
      }
      suivi[nb_thread] = threadClient;
      nb_thread++;
      // Détachement du thread pour libérer automatiquement les ressources une fois terminé
      pthread_detach(threadClient);
    } else if (ret == 0) {
        // Le délai d'attente est dépassé, vérifiez la condition de la boucle
        continue;
    } else {
        // Une erreur s'est produite
        perror("select 1");
        break;
    }

  }

  for(int i = 0; i<nb_thread; i++){
    pthread_join(suivi[i], NULL);
    printf("thread %d terminé\n", i);
  }
  // Fermeture des sockets
  close(server_socket);

  // Destruction du mutex
  pthread_mutex_destroy(&lock_grid);
  pthread_mutex_destroy(&lock_create_game);
  pthread_mutex_destroy(&end_game);
}

int recv_requete(int fdsock, Game *game){
    char *codereqid = malloc(sizeof(CodereqIdEq) + sizeof(uint16_t));
    struct sockaddr adrclient;
    socklen_t adrclientlen = sizeof(adrclient);
    int byte_received = recvfrom(fdsock, codereqid, sizeof(codereqid), 0, &adrclient, &adrclientlen);
    if (byte_received == -1)
    {
        perror("Erreur de lecture");
        close(fdsock);
        exit(EXIT_FAILURE);
    }
    else if (byte_received == 0)
    {
        printf("Client deconnecté\n");
        close(fdsock);
    }
    CodereqIdEq codereqid2;
    Codereq_5_6 codereq;
    memcpy(&codereqid2, codereqid, sizeof(CodereqIdEq));
    memcpy(&codereq, codereqid + sizeof(CodereqIdEq), sizeof(u_int16_t));
    Client response;
    reverse_codereqid_serv(codereqid2, &response);
    game->players[response.id]->req = response.req;
    // byte_received = recvfrom(fdsock, &codereq, sizeof(codereq), 0, &adrclient, &adrclientlen);
    reverse_action(&codereq, game->players[response.id]);
    // free(codereqid);
    return response.id;
}

void send_response_data(int fdsock, Game *game, Client *client)
{
  char *buffer = malloc(sizeof(CodereqIdEq));   
  CodereqIdEq buf;
  if (game->mode == 1)
  {
    buf = init_codereqid((uint16_t)9, client->id, client->equipe);
  }
  else
  {
    buf = init_codereqid((uint16_t)10, client->id, client->equipe);
  }
  memcpy(buffer, &buf, sizeof(CodereqIdEq));
  send_9_10(buffer, game);
  int ecrit = send(fdsock, buffer, sizeof(CodereqIdEq) + 2*sizeof(uint16_t) + sizeof(struct in6_addr), 0);
  if (ecrit < 0)
  {
      perror("Erreur lors de l'envoi du message");
  }
  // free(buffer);
}

void send_grid(int sock, struct sockaddr_in6 multicastAddr, board *board, uint16_t num)
{
    char *msg_udp = malloc(4096 * sizeof(char));
    CodereqIdEq bufchar = init_codereqid((uint16_t)11, (uint16_t)0, (uint16_t)0);
    memcpy(msg_udp, &bufchar, sizeof(CodereqIdEq));
    send_11(msg_udp, board, num);
    int ecrit = sendto(sock, msg_udp, 4096 * sizeof(char), 0, (struct sockaddr *)&multicastAddr, sizeof(multicastAddr));
    // printf("ecrit %d\n", ecrit);
    if (ecrit < 0)
    {
        perror("Erreur lors de l'envoi du message");
    }
    free(msg_udp);
}

void send_grid_freq(int sock, struct sockaddr_in6 multicastAddr, board *board1, board *after, uint16_t num)
{
    char *msg_udp = malloc(4096 * sizeof(char));
    CodereqIdEq bufchar = init_codereqid((uint16_t)12, (uint16_t)0, (uint16_t)0);
    memcpy(msg_udp, &bufchar, sizeof(CodereqIdEq));
    send_12(msg_udp, board1, after, num);
    int ecrit = sendto(sock, msg_udp, 4096 * sizeof(char), 0, (struct sockaddr *)&multicastAddr, sizeof(multicastAddr));
    if (ecrit < 0)
    {
        perror("Erreur lors de l'envoi du message");
    }
    // free(msg_udp);
}

void send_message(int sock, Client *client){
  char *buffer = malloc(sizeof(CodereqIdEq));   
  CodereqIdEq buf;
  if (client->req == 13)
  {
    buf = init_codereqid((uint16_t)13, client->id, client->equipe);
  }
  else
  {
    buf = init_codereqid((uint16_t)14, client->id, client->equipe);
  }
  memcpy(buffer, &buf, sizeof(CodereqIdEq));
  send_msg(buffer, client->len, client->data);
  int ecrit = send(sock, buffer, sizeof(CodereqIdEq)+sizeof(uint8_t)+256*sizeof(char), 0);
  if (ecrit < 0)
  {
      perror("Erreur lors de l'envoi du message");
  }
  // free(buffer);
}

void send_end(int sock, Game *game){
  char *buffer = malloc(sizeof(CodereqIdEq));   
  CodereqIdEq buf;
  if (game->mode == 1)
  {
    buf = init_codereqid((uint16_t)15, (uint16_t)0, (uint16_t)0);
  }
  else
  {
    buf = init_codereqid((uint16_t)16, (uint16_t)0, (uint16_t)0);
  }
  memcpy(buffer, &buf, sizeof(CodereqIdEq));
  int ecrit = send(sock, buffer, sizeof(CodereqIdEq), 0);
  if (ecrit < 0)
  {
      perror("Erreur lors de l'envoi du message");
  }
  // free(buffer);
}



