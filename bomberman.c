#include "serveur_ipv6.h"
#include "grid.h"
#include "client_ipv6.h"
#include "requete.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    // Lancer tous les trucs
    start_server();
    free_game(games_head);
}
