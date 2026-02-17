CC=g++
CFLAGS=-std=c++11 -Wall -O3

# Cibles principales
all: breakthrough_simple nb_playout_per_sec rand_player

# IA principale avec algorithme "My Algo"
breakthrough_simple: breakthrough_simple.cpp
	$(CC) $(CFLAGS) breakthrough_simple.cpp -o $@

# Benchmark de performance
nb_playout_per_sec: bkbb64.h nb_playout_per_sec.cpp
	$(CC) $(CFLAGS) nb_playout_per_sec.cpp -o $@

# Joueur aléatoire original
rand_player: bkbb64.h rand_player.cpp
	$(CC) $(CFLAGS) rand_player.cpp -o $@

# Tests
test: breakthrough_simple
	@echo "=== Test coup unique ==="
	./breakthrough_simple @@@@@@@@@@@@@@@@................................OOOOOOOOOOOOOOOO O
	@echo ""
	@echo "=== Test aide ==="
	./breakthrough_simple help

# Nettoyage
clean:
	rm -rf *~ breakthrough_simple nb_playout_per_sec rand_player

# Installation pour Ludii (avec renommage selon les initiales du groupe)
install-ludii: breakthrough_simple
	@echo "Copiez votre binaire dans LudiiPlayers/bin/ avec vos initiales"
	@echo "Exemple: cp breakthrough_simple ../LudiiPlayers/bin/ab-cd"

.PHONY: all test clean install-ludii