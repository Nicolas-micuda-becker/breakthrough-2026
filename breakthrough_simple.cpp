#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

// ============================================================================
// TYPES DE BASE
// ============================================================================

enum Case {
    VIDE = '.',
    WHITE = '0',  // white - O dans la notation
    BLACK = '1'    // black - @ dans la notation
};

struct Position {
    int ligne;  // 0-7 (0 = ligne 8, 7 = ligne 1)
    int col;    // 0-7 (0 = colonne A, 7 = colonne H)
};

struct Coup {
    Position from;
    Position to;
};

struct Plateau {
    Case cases[8][8];
};

// ============================================================================
// FONCTIONS UTILITAIRES
// ============================================================================

// Initialise un plateau à partir d'une chaîne (format ludii)
void init_plateau(Plateau* p, const char* str) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char c = str[i * 8 + j];
            if (c == '@') p->cases[i][j] = BLACK;
            else if (c == 'O') p->cases[i][j] = WHITE;
            else p->cases[i][j] = VIDE;
        }
    }
}

// Affiche le plateau
void afficher_plateau(const Plateau* p) {
    printf("  A B C D E F G H\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < 8; j++) {
            if (p->cases[i][j] == BLACK) printf("1 ");
            else if (p->cases[i][j] == WHITE) printf("0 ");
            else printf(". ");
        }
        printf("%d\n", 8 - i);
    }
    printf("  A B C D E F G H\n");
}

// Affiche un coup au format "A1-B2"
void afficher_coup(const Coup* c) {
    printf("%c%d-%c%d\n", 
           'A' + c->from.col, 8 - c->from.ligne,
           'A' + c->to.col, 8 - c->to.ligne);
}

// ============================================================================
// GÉNÉRATION DE COUPS
// ============================================================================

// Vérifie si une position est dans le plateau
bool dans_plateau(int ligne, int col) {
    return ligne >= 0 && ligne < 8 && col >= 0 && col < 8;
}

// Génère tous les coups légaux pour un joueur
void generer_coups(const Plateau* p, Case joueur, std::vector<Coup>* coups) {
    coups->clear();
    
    // Direction : NOIR descend (+1), BLANC monte (-1)
    int direction = (joueur == BLACK) ? 1 : -1;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (p->cases[i][j] != joueur) continue;
            
            // 3 mouvements possibles : avant, diag gauche, diag droite
            int destinations[3][2] = {
                {i + direction, j},      // Avant
                {i + direction, j - 1},  // Diag gauche
                {i + direction, j + 1}   // Diag droite
            };
            
            for (int k = 0; k < 3; k++) {
                int ni = destinations[k][0];
                int nj = destinations[k][1];
                
                if (!dans_plateau(ni, nj)) continue;
                
                Case dest = p->cases[ni][nj];
                
                // Avancer droit : case doit être vide
                if (k == 0 && dest != VIDE) continue;
                
                // Diagonale : peut être vide OU ennemi
                if (k > 0) {
                    if (dest == joueur) continue;  // Pas capturer allié
                }
                
                Coup c;
                c.from.ligne = i;
                c.from.col = j;
                c.to.ligne = ni;
                c.to.col = nj;
                coups->push_back(c);
            }
        }
    }
}

// ============================================================================
// JOUER / DÉJOUER UN COUP
// ============================================================================

// Applique un coup sur le plateau
void jouer_coup(Plateau* p, const Coup* c) {
    p->cases[c->to.ligne][c->to.col] = p->cases[c->from.ligne][c->from.col];
    p->cases[c->from.ligne][c->from.col] = VIDE;
}

// Annule un coup (pour minimax ou MCTS)
void dejouer_coup(Plateau* p, const Coup* c, Case piece_capturee) {
    p->cases[c->from.ligne][c->from.col] = p->cases[c->to.ligne][c->to.col];
    p->cases[c->to.ligne][c->to.col] = piece_capturee;
}

// ============================================================================
// VÉRIFICATION DE FIN
// ============================================================================

// Vérifie si un joueur a gagné (pion sur ligne adverse)
bool a_gagne(const Plateau* p, Case joueur) {
    int ligne_victoire = (joueur == BLACK) ? 7 : 0;
    
    for (int j = 0; j < 8; j++) {
        if (p->cases[ligne_victoire][j] == joueur) {
            return true;
        }
    }
    return false;
}

// Retourne l'adversaire
Case adversaire(Case joueur) {
    return (joueur == BLACK) ? WHITE : BLACK;
}

// ============================================================================
// FONCTION D'ÉVALUATION
// ============================================================================

// Évalue la position du point de vue du joueur
int evaluer(const Plateau* p, Case joueur) {
    Case adv = adversaire(joueur);
    
    // Vérifier victoire
    if (a_gagne(p, joueur)) return 10000;
    if (a_gagne(p, adv)) return -10000;
    
    int score = 0;
    
    // 1. Compter les pions (valeur matérielle)
    int mes_pions = 0;
    int ses_pions = 0;
    
    // 2. Avancement des pions (plus un pion est proche de la promotion, mieux c'est)
    int mon_avancement = 0;
    int son_avancement = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (p->cases[i][j] == joueur) {
                mes_pions++;
                // Avancement : pour NOIR (descend), ligne 7 = promotion (distance = 7-i)
                // Pour BLANC (monte), ligne 0 = promotion (distance = i)
                if (joueur == BLACK) {
                    mon_avancement += (7 - i);  // Plus la ligne est haute, mieux c'est
                } else {
                    mon_avancement += i;  // Plus la ligne est basse, mieux c'est
                }
            } else if (p->cases[i][j] == adv) {
                ses_pions++;
                if (adv == BLACK) {
                    son_avancement += (7 - i);
                } else {
                    son_avancement += i;
                }
            }
        }
    }
    
    // Score = matériel + avancement
    score += (mes_pions - ses_pions) * 100;      // Chaque pion vaut 100
    score += (mon_avancement - son_avancement) * 10;  // Avancement vaut 10 par ligne
    
    return score;
}

// ============================================================================
// ALGORITHME : "THREAT WAVE" (Vague de Menaces)
// ============================================================================

struct EvaluationCoup {
    Coup coup;
    int score_immediat;      // Score juste après notre coup
    int score_apres_riposte; // Score après la meilleure riposte adverse
    int score_final;         // Combinaison des deux
};

// Évalue les patterns tactiques d'un coup
int evaluer_patterns_coup(Plateau* p, const Coup* c, Case joueur) {
    int score = 0;
    Case adv = adversaire(joueur);
    int direction = (joueur == BLACK) ? 1 : -1;
    
    // PATTERN 1 : Promotion imminente (pion à 1 case de la victoire)
    int ligne_victoire = (joueur == BLACK) ? 7 : 0;
    if (c->to.ligne == ligne_victoire) {
        return 50000;  // VICTOIRE !
    }
    if ((joueur == BLACK && c->to.ligne == 6) || 
        (joueur == WHITE && c->to.ligne == 1)) {
        score += 5000;  // Une case de la victoire
    }
    
    // PATTERN 2 : Capture (éliminer un ennemi)
    if (p->cases[c->to.ligne][c->to.col] == adv) {
        score += 800;
        // Bonus si on capture un pion avancé
        int distance_ennemie = (adv == BLACK) ? (7 - c->to.ligne) : c->to.ligne;
        score += distance_ennemie * 50;
    }
    
    // PATTERN 3 : Avancer un pion (course à la promotion)
    int distance_avant = (joueur == BLACK) ? (7 - c->from.ligne) : c->from.ligne;
    int distance_apres = (joueur == BLACK) ? (7 - c->to.ligne) : c->to.ligne;
    score += (distance_apres - distance_avant) * 200;
    
    // PATTERN 4 : Contrôle du centre (colonnes C,D,E,F)
    if (c->to.col >= 2 && c->to.col <= 5) {
        score += 100;
    }
    
    // PATTERN 5 : Pion protégé (a des alliés en diagonale arrière)
    int arriere = c->to.ligne - direction;
    if (dans_plateau(arriere, c->to.col - 1) && p->cases[arriere][c->to.col - 1] == joueur) {
        score += 80;
    }
    if (dans_plateau(arriere, c->to.col + 1) && p->cases[arriere][c->to.col + 1] == joueur) {
        score += 80;
    }
    
    // PATTERN 6 : Créer des menaces multiples (plusieurs pions proches de la promotion)
    int ligne_menace = (joueur == BLACK) ? 5 : 2;
    if (c->to.ligne == ligne_menace) {
        // Compter combien d'alliés sont aussi sur cette ligne ou plus avancés
        int allies_avances = 0;
        for (int j = 0; j < 8; j++) {
            if ((joueur == BLACK && p->cases[ligne_menace][j] == joueur) ||
                (joueur == WHITE && p->cases[ligne_menace][j] == joueur)) {
                allies_avances++;
            }
        }
        score += allies_avances * 150;
    }
    
    // PATTERN 7 : Éviter les colonnes bloquées
    int ligne_suivante = c->to.ligne + direction;
    if (dans_plateau(ligne_suivante, c->to.col)) {
        if (p->cases[ligne_suivante][c->to.col] == joueur) {
            score -= 200;  // Pion bloqué par un allié
        }
    }
    
    return score;
}

// Trouve la meilleure riposte adverse (simulation 1 coup)
int evaluer_meilleure_riposte(Plateau* p, Case joueur) {
    Case adv = adversaire(joueur);
    std::vector<Coup> coups_adv;
    generer_coups(p, adv, &coups_adv);
    
    if (coups_adv.empty()) {
        return 10000;  // Adversaire ne peut pas jouer = on gagne
    }
    
    int pire_score = 99999;
    
    for (size_t i = 0; i < coups_adv.size(); i++) {
        Coup& c = coups_adv[i];
        Case piece_capturee = p->cases[c.to.ligne][c.to.col];
        jouer_coup(p, &c);
        
        // Évaluation simple après riposte adverse
        int score = evaluer(p, joueur);
        
        dejouer_coup(p, &c, piece_capturee);
        
        if (score < pire_score) {
            pire_score = score;
        }
    }
    
    return pire_score;
}

// ALGORITHME PRINCIPAL : Threat Wave
Coup choisir_coup_threat_wave(Plateau* p, Case joueur) {
    std::vector<Coup> coups;
    generer_coups(p, joueur, &coups);
    
    if (coups.empty()) {
        Coup c;
        c.from.ligne = -1;
        return c;
    }
    
    std::vector<EvaluationCoup> evaluations;
    
    // Phase 1 : Évaluer chaque coup avec patterns + simulation riposte
    for (size_t i = 0; i < coups.size(); i++) {
        EvaluationCoup eval;
        eval.coup = coups[i];
        
        // Jouer notre coup
        Case piece_capturee = p->cases[coups[i].to.ligne][coups[i].to.col];
        jouer_coup(p, &coups[i]);
        
        // Score immédiat basé sur patterns
        eval.score_immediat = evaluer_patterns_coup(p, &coups[i], joueur);
        
        // Victoire immédiate ? Pas besoin de simuler
        if (a_gagne(p, joueur)) {
            eval.score_immediat = 100000;
            eval.score_apres_riposte = 100000;
            eval.score_final = 100000;
        } else {
            // Simuler la meilleure riposte adverse
            eval.score_apres_riposte = evaluer_meilleure_riposte(p, joueur);
            
            // Score final = pondération entre immédiat et après riposte
            eval.score_final = (eval.score_immediat * 60 + eval.score_apres_riposte * 40) / 100;
        }
        
        dejouer_coup(p, &coups[i], piece_capturee);
        
        evaluations.push_back(eval);
    }
    
    // Phase 2 : Choisir le meilleur coup
    int meilleur_idx = 0;
    int meilleur_score = evaluations[0].score_final;
    
    for (size_t i = 1; i < evaluations.size(); i++) {
        if (evaluations[i].score_final > meilleur_score) {
            meilleur_score = evaluations[i].score_final;
            meilleur_idx = i;
        }
    }
    
    return evaluations[meilleur_idx].coup;
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char** argv) {
    srand(time(NULL));
    
    if (argc < 3) {
        printf("Usage: %s <plateau> <joueur> [debug]\n", argv[0]);
        printf("Exemple: %s @@@@@@@@@@@@@@@@................................OOOOOOOOOOOOOOOO O debug\n", argv[0]);
        return 1;
    }
    
    const char* plateau_str = argv[1];
    char joueur_char = argv[2][0];
    
    // Mode debug (optionnel)
    bool debug = false;
    for (int i = 3; i < argc; i++) {
        if (argv[i][0] == 'd' || argv[i][0] == 'D') {
            debug = true;
        }
    }
    
    // Initialiser plateau
    Plateau p;
    init_plateau(&p, plateau_str);
    
    // Déterminer le joueur
    Case joueur = (joueur_char == '@') ? BLACK : WHITE;
    
    if (debug) {
        printf("board 1\n");
        afficher_plateau(&p);
        printf("Algorithme: Threat Wave\n");
    }
    
    // Choisir et afficher un coup avec Threat Wave
    Coup c = choisir_coup_threat_wave(&p, joueur);
    
    if (c.from.ligne == -1) {
        printf("Aucun coup possible!\n");
        return 1;
    }
    
    afficher_coup(&c);
    
    return 0;
}
