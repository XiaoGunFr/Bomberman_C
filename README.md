# Bomberman Multijoueur en C

Un jeu Bomberman multijoueur en réseau utilisant IPv6, développé en C avec ncurses.

## Description

Jeu Bomberman classique permettant à 4 joueurs de s'affronter en réseau local via IPv6. Deux modes de jeu sont disponibles :
- **Mode 4 adversaires** : chacun pour soi
- **Mode équipe** : 2 équipes de 2 joueurs

## Fonctionnalités

- Multijoueur réseau (4 joueurs)
- Communication UDP/TCP en IPv6
- Multidiffusion pour synchronisation de la grille
- Bombes avec explosion temporisée (3 secondes)
- Murs destructibles et indestructibles
- Interface graphique en terminal (ncurses)
- Chat intégré (en développement)

## Compilation

### Prérequis
- `gcc`
- `ncurses` library
- Support IPv6

### Compilation du serveur et du client
```bash
make
```

Cela génère deux exécutables :
- `Bomberman` (serveur)
- `Client` (client)

### Nettoyage
```bash
make clean
```

## Lancement

### Démarrer le serveur
```bash
./Bomberman
```

### Connecter un client
```bash
./Client
```

1. Choisir le mode de jeu (1 ou 2)
2. Attendre que 4 joueurs soient connectés
3. Confirmer que vous êtes prêt
4. La partie démarre automatiquement

## Contrôles

- **Flèches directionnelles** : Déplacer le joueur
- **Entrée** : Poser une bombe
- **~** : Quitter la partie
- **Alt** : Envoyer un message (chat)

## Structure du projet

```
.
├── bomberman.c        # Point d'entrée du serveur
├── serveur_ipv6.c/h   # Logique serveur et gestion des parties
├── client_ipv6.c/h    # Logique client et interface
├── grid.c/h           # Gestion de la grille et des actions
├── requete.c/h        # Protocole de communication
├── Makefile           # Script de compilation
└── README.md
```

## Protocole réseau

- **TCP** : Connexion initiale, synchronisation des joueurs
- **UDP** : Actions des joueurs en temps réel
- **Multidiffusion IPv6** : Distribution de la grille de jeu

Ports par défaut :
- TCP : 7878
- UDP : 3000+ (généré dynamiquement)
- Multidiffusion : 5000+ (généré dynamiquement)

## Debug

```bash
# Analyser les fuites mémoire (serveur)
make valgrind_server

# Analyser les fuites mémoire (client)
make valgrind_client
```

## Auteurs

Voir [AUTHORS.md](AUTHORS.md)

## Licence

Ce projet est sous licence MIT - voir le fichier [LICENSE](LICENSE) pour plus de détails.