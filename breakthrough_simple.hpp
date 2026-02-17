#ifndef BREAKTHROUGH_SIMPLE_HPP
#define BREAKTHROUGH_SIMPLE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <vector>

enum Case
{
    VIDE = '.',
    WHITE = '0',
    BLACK = '1'
};

struct Position
{
    int ligne;
    int col;
};

struct Coup
{
    Position from;
    Position to;
};

struct Plateau
{
    Case cases[8][8];
};

struct EvaluationCoup
{
    Coup coup;
    int score_immediat;
    int score_apres_riposte;
    int score_final;
};

void init_plateau(Plateau *p, const char *str);
void afficher_plateau(const Plateau *p);
void afficher_coup(const Coup *c);
bool dans_plateau(int ligne, int col);
void generer_coups(const Plateau *p, Case joueur, std::vector<Coup> *coups);
void jouer_coup(Plateau *p, const Coup *c);
void dejouer_coup(Plateau *p, const Coup *c, Case piece_capturee);
bool a_gagne(const Plateau *p, Case joueur);
Case adversaire(Case joueur);
int evaluer(const Plateau *p, Case joueur);
int evaluer_patterns_coup(Plateau *p, const Coup *c, Case joueur);
int evaluer_meilleure_riposte(Plateau *p, Case joueur);
Coup choisir_coup_my_algo(Plateau *p, Case joueur);
void jouer_partie_humain_vs_ia();
void afficher_aide();

#endif