#include "breakthrough_simple.hpp"

void init_plateau(Plateau *p, const char *str)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            char c = str[i * 8 + j];
            if (c == '1')
                p->cases[i][j] = BLACK;
            else if (c == '0')
                p->cases[i][j] = WHITE;
            else
                p->cases[i][j] = VIDE;
        }
    }
}

void afficher_plateau(const Plateau *p)
{
    printf("  A B C D E F G H\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%d ", 8 - i);
        for (int j = 0; j < 8; j++)
        {
            printf("%c ", p->cases[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  A B C D E F G H\n");
}

void afficher_coup(const Coup *c)
{
    printf("%c%d-%c%d\n",
           'A' + c->from.col, 8 - c->from.ligne,
           'A' + c->to.col, 8 - c->to.ligne);
}

bool dans_plateau(int ligne, int col)
{
    return ligne >= 0 && ligne < 8 && col >= 0 && col < 8;
}

void generer_coups(const Plateau *p, Case joueur, std::vector<Coup> *coups)
{
    coups->clear();

    int direction = (joueur == BLACK) ? 1 : -1;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (p->cases[i][j] != joueur)
                continue;

            int destinations[3][2] = {
                {i + direction, j},
                {i + direction, j - 1},
                {i + direction, j + 1}};

            for (int k = 0; k < 3; k++)
            {
                int ni = destinations[k][0];
                int nj = destinations[k][1];

                if (!dans_plateau(ni, nj))
                    continue;

                Case dest = p->cases[ni][nj];

                if (k == 0 && dest != VIDE)
                    continue;

                if (k > 0)
                {
                    if (dest == joueur)
                        continue;
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

void jouer_coup(Plateau *p, const Coup *c)
{
    p->cases[c->to.ligne][c->to.col] = p->cases[c->from.ligne][c->from.col];
    p->cases[c->from.ligne][c->from.col] = VIDE;
}

void dejouer_coup(Plateau *p, const Coup *c, Case piece_capturee)
{
    p->cases[c->from.ligne][c->from.col] = p->cases[c->to.ligne][c->to.col];
    p->cases[c->to.ligne][c->to.col] = piece_capturee;
}

bool a_gagne(const Plateau *p, Case joueur)
{
    int ligne_victoire = (joueur == BLACK) ? 7 : 0;

    for (int j = 0; j < 8; j++)
    {
        if (p->cases[ligne_victoire][j] == joueur)
        {
            return true;
        }
    }
    return false;
}

Case adversaire(Case joueur)
{
    return (joueur == BLACK) ? WHITE : BLACK;
}

int evaluer(const Plateau *p, Case joueur)
{
    Case adv = adversaire(joueur);

    if (a_gagne(p, joueur))
        return 10000;
    if (a_gagne(p, adv))
        return -10000;

    int score = 0;

    int mes_pions = 0;
    int ses_pions = 0;

    int mon_avancement = 0;
    int son_avancement = 0;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (p->cases[i][j] == joueur)
            {
                mes_pions++;

                if (joueur == BLACK)
                {
                    mon_avancement += (7 - i);
                }
                else
                {
                    mon_avancement += i;
                }
            }
            else if (p->cases[i][j] == adv)
            {
                ses_pions++;
                if (adv == BLACK)
                {
                    son_avancement += (7 - i);
                }
                else
                {
                    son_avancement += i;
                }
            }
        }
    }

    score += (mes_pions - ses_pions) * 100;
    score += (mon_avancement - son_avancement) * 10;

    return score;
}

int evaluer_patterns_coup(Plateau *p, const Coup *c, Case joueur)
{
    int score = 0;
    Case adv = adversaire(joueur);
    int direction = (joueur == BLACK) ? 1 : -1;

    int ligne_victoire = (joueur == BLACK) ? 7 : 0;
    if (c->to.ligne == ligne_victoire)
    {
        return 50000;
    }
    if ((joueur == BLACK && c->to.ligne == 6) ||
        (joueur == WHITE && c->to.ligne == 1))
    {
        score += 5000;
    }

    if (p->cases[c->to.ligne][c->to.col] == adv)
    {
        score += 800;
        int distance_ennemie = (adv == BLACK) ? (7 - c->to.ligne) : c->to.ligne;
        score += distance_ennemie * 50;
    }

    int distance_avant = (joueur == BLACK) ? (7 - c->from.ligne) : c->from.ligne;
    int distance_apres = (joueur == BLACK) ? (7 - c->to.ligne) : c->to.ligne;
    score += (distance_apres - distance_avant) * 200;

    if (c->to.col >= 2 && c->to.col <= 5)
    {
        score += 100;
    }

    int arriere = c->to.ligne - direction;
    if (dans_plateau(arriere, c->to.col - 1) && p->cases[arriere][c->to.col - 1] == joueur)
    {
        score += 80;
    }
    if (dans_plateau(arriere, c->to.col + 1) && p->cases[arriere][c->to.col + 1] == joueur)
    {
        score += 80;
    }

    int ligne_menace = (joueur == BLACK) ? 5 : 2;
    if (c->to.ligne == ligne_menace)
    {
        int allies_avances = 0;
        for (int j = 0; j < 8; j++)
        {
            if ((joueur == BLACK && p->cases[ligne_menace][j] == joueur) ||
                (joueur == WHITE && p->cases[ligne_menace][j] == joueur))
            {
                allies_avances++;
            }
        }
        score += allies_avances * 150;
    }

    int ligne_suivante = c->to.ligne + direction;
    if (dans_plateau(ligne_suivante, c->to.col))
    {
        if (p->cases[ligne_suivante][c->to.col] == joueur)
        {
            score -= 200;
        }
    }

    return score;
}

int evaluer_meilleure_riposte(Plateau *p, Case joueur)
{
    Case adv = adversaire(joueur);
    std::vector<Coup> coups_adv;
    generer_coups(p, adv, &coups_adv);

    if (coups_adv.empty())
    {
        return 10000;
    }

    int pire_score = 99999;

    for (size_t i = 0; i < coups_adv.size(); i++)
    {
        Coup &c = coups_adv[i];
        Case piece_capturee = p->cases[c.to.ligne][c.to.col];
        jouer_coup(p, &c);

        int score = evaluer(p, joueur);

        dejouer_coup(p, &c, piece_capturee);

        if (score < pire_score)
        {
            pire_score = score;
        }
    }

    return pire_score;
}

Coup choisir_coup_my_algo(Plateau *p, Case joueur)
{
    std::vector<Coup> coups;
    generer_coups(p, joueur, &coups);

    if (coups.empty())
    {
        Coup c;
        c.from.ligne = -1;
        return c;
    }

    std::vector<EvaluationCoup> evaluations;

    for (size_t i = 0; i < coups.size(); i++)
    {
        EvaluationCoup eval;
        eval.coup = coups[i];

        Case piece_capturee = p->cases[coups[i].to.ligne][coups[i].to.col];
        jouer_coup(p, &coups[i]);

        eval.score_immediat = evaluer_patterns_coup(p, &coups[i], joueur);

        if (a_gagne(p, joueur))
        {
            eval.score_immediat = 100000;
            eval.score_apres_riposte = 100000;
            eval.score_final = 100000;
        }
        else
        {
            eval.score_apres_riposte = evaluer_meilleure_riposte(p, joueur);

            eval.score_final = (eval.score_immediat * 60 + eval.score_apres_riposte * 40) / 100;
        }

        dejouer_coup(p, &coups[i], piece_capturee);

        evaluations.push_back(eval);
    }

    int meilleur_idx = 0;
    int meilleur_score = evaluations[0].score_final;

    for (size_t i = 1; i < evaluations.size(); i++)
    {
        if (evaluations[i].score_final > meilleur_score)
        {
            meilleur_score = evaluations[i].score_final;
            meilleur_idx = i;
        }
    }

    return evaluations[meilleur_idx].coup;
}


void jouer_partie_humain_vs_ia()
{
    Plateau plateau;
    init_plateau(&plateau, "1111111111111111................................0000000000000000");

    Case joueur_humain = WHITE;
    Case joueur_ia = BLACK;
    Case joueur_actuel = joueur_humain;

    int tour = 1;
    const int MAX_TOURS = 200;

    printf("BREAKTHROUGH : HUMAIN vs IA\n");
    printf("But : Amener un pion sur la ligne adverse\n");
    printf("Deplacement : 1 case vers l'avant ou diagonale\n\n");

    afficher_plateau(&plateau);

    while (tour <= MAX_TOURS)
    {
        printf("\n--- Tour %d ---\n", tour);

        if (a_gagne(&plateau, joueur_humain))
        {
            printf("VICTOIRE HUMAIN ! Un de vos pions a atteint la ligne 8 !\n");
            break;
        }
        if (a_gagne(&plateau, joueur_ia))
        {
            printf("VICTOIRE IA ! Un pion noir a atteint la ligne 1 !\n");
            break;
        }

        std::vector<Coup> coups;
        generer_coups(&plateau, joueur_actuel, &coups);

        if (coups.empty())
        {
            printf("Aucun coup possible - PAT\n");
            break;
        }

        if (joueur_actuel == joueur_ia)
        {
            Coup coup_ia = choisir_coup_my_algo(&plateau, joueur_ia);
            printf("IA joue : ");
            afficher_coup(&coup_ia);
            jouer_coup(&plateau, &coup_ia);
        }
        else
        {
            printf("Votre tour\n");
            printf("Format: A1-B2\n");

            char coup_str[10];
            printf("Votre coup : ");
            if (scanf("%s", coup_str) != 1)
            {
                printf("Erreur de saisie\n");
                continue;
            }

            if (strlen(coup_str) != 5 || coup_str[2] != '-')
            {
                printf("Format invalide\n");
                continue;
            }

            Position from, to;
            from.col = coup_str[0] - 'A';
            from.ligne = 8 - (coup_str[1] - '0');
            to.col = coup_str[3] - 'A';
            to.ligne = 8 - (coup_str[4] - '0');

            bool coup_valide = false;
            Coup coup_choisi;
            for (size_t i = 0; i < coups.size(); i++)
            {
                if (coups[i].from.ligne == from.ligne && coups[i].from.col == from.col &&
                    coups[i].to.ligne == to.ligne && coups[i].to.col == to.col)
                {
                    coup_valide = true;
                    coup_choisi = coups[i];
                    break;
                }
            }

            if (!coup_valide)
            {
                printf("Coup invalide\n");
                continue;
            }

            printf("Vous jouez : ");
            afficher_coup(&coup_choisi);
            jouer_coup(&plateau, &coup_choisi);
        }

        printf("\nPlateau apres le coup :\n");
        afficher_plateau(&plateau);

        joueur_actuel = adversaire(joueur_actuel);
        tour++;
    }

    if (tour > MAX_TOURS)
    {
        printf("Partie trop longue - Match nul\n");
    }

    printf("FIN DE PARTIE\n");
}

void afficher_aide()
{
    printf("BREAKTHROUGH IA\n");
    printf("Usage:\n");
    printf("  %s partie                    # Jouer une partie humain vs IA\n", "breakthrough_simple");
    printf("  %s <plateau> <joueur>         # Calculer un coup unique\n", "breakthrough_simple");
    printf("\nExemples:\n");
    printf("  %s partie\n", "breakthrough_simple");
    printf("  %s 1111111111111111................................0000000000000000 0\n", "breakthrough_simple");
    printf("\nNotation plateau (64 caracteres):\n");
    printf("  1 = pion noir    0 = pion blanc    . = case vide\n");
    printf("  Joueur: 1 (noir) ou 0 (blanc)\n");
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc == 1 || (argc == 2 && (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0)))
    {
        afficher_aide();
        return 0;
    }

    if (argc >= 2 && strcmp(argv[1], "partie") == 0)
    {
        jouer_partie_humain_vs_ia();
        return 0;
    }

    if (argc < 3)
    {
        printf("Erreur: Arguments insuffisants\n");
        afficher_aide();
        return 1;
    }

    const char *plateau_str = argv[1];
    char joueur_char = argv[2][0];

    if (strlen(plateau_str) != 64)
    {
        printf("Erreur: Le plateau doit faire exactement 64 caracteres.\n");
        return 1;
    }

    Plateau p;
    init_plateau(&p, plateau_str);

    Case joueur;
    if (joueur_char == '1')
    {
        joueur = BLACK;
    }
    else if (joueur_char == '0')
    {
        joueur = WHITE;
    }
    else
    {
        printf("Erreur: Joueur doit etre '1' (noir) ou '0' (blanc).\n");
        return 1;
    }

    Coup c = choisir_coup_my_algo(&p, joueur);

    if (c.from.ligne == -1)
    {
        printf("Aucun coup possible\n");
        return 1;
    }

    afficher_coup(&c);

    return 0;
}
