# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -o

# Bibliothèques à lier
LIBS = -lncurses -lpthread
# Fichiers source
SOURCE = grid.c serveur_ipv6.c bomberman.c requete.c
SOURCE2 = grid.c client_ipv6.c requete.c

# Fichiers headers
HEADER = grid.h serveur_ipv6.h client_ipv6.h requete.h

# Nom de l'exécutable
EXECUTABLE = Bomberman
EXECUTABLE2 = Client

all: $(EXECUTABLE) $(EXECUTABLE2)

$(EXECUTABLE): $(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) $(EXECUTABLE) $(SOURCE) $(LIBS)

$(EXECUTABLE2): $(SOURCE2) $(HEADER)
	$(CC) $(CFLAGS) $(EXECUTABLE2) $(SOURCE2) $(LIBS)

clean:
	rm -f $(EXECUTABLE)
	rm -f $(EXECUTABLE2)

valgrind_server :
	valgrind --leak-check=full --show-leak-kinds=definite ./$(EXECUTABLE)

valgrind_client :
	valgrind --leak-check=full --show-leak-kinds=definite ./$(EXECUTABLE2)
