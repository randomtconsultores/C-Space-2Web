#ifndef SPACE_H
#define SPACE_H

#ifndef SPACE_WEB
#include <ncurses.h>
#endif

#define DELAY 30000
#define LARGURA_TELA 40
#define ALTURA_TELA 15
#define LARGURA_MAPA 1150
#define MAX_TIROS 20
#define MAX_NIVEIS 4
#define MAX_TIROS_INIMIGO 5
#define BOSS_LARGURA 3
#define BOSS_ALTURA 3
#define BOSS_VIDA_MAX 150
#define MAX_VIDAS_JOGADOR 9


extern int som_ativo; // Variável global para controlar o som (0 = Desligado, 1 = Ligado)
extern int tecla_up;
extern int tecla_down;
extern int tecla_left;
extern int tecla_right;
extern int tecla_fire;

typedef struct
{
    int x, y;
    int ativo;
} Projétil;

typedef struct
{
    int x, y;
    int vidas;
    int imunidade;
    long pontuacao;
    long proximo_patamar_pontos;
    Projétil tiro[MAX_TIROS];
} Player;

typedef struct
{
    int x, y;
    int vida;
    int ativo;             // 0 = Inativo, 1 = Luta, 2 = Derrotado
    int direcao_y;
    int direcao_x;
    int hp_arma_superior; // 150 HP (linha b->y - 1)
    int hp_arma_inferior; // 150 HP (linha b->y + 1)
    int hp_nucleo;        // 200 HP (linha b->y)
    int fase_atual;       // 1 = Desarmamento, 2 = Modo Laser/Touro
} Boss;

void init_game();
void gerar_mapa(char mapa[ALTURA_TELA][LARGURA_MAPA], unsigned int seed, int nivel_atual);
void process_input(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, int upgrade_x);
void update_game(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, Boss *b, Projétil tiros_ini[MAX_TIROS_INIMIGO], int nivel_atual);
void draw_game(Player *p, char mapa[ALTURA_TELA][LARGURA_MAPA], int offset, int nivel_atual, Boss *b, Projétil tiros_ini[MAX_TIROS_INIMIGO]);
void draw_transition(int nivel_terminado);

#endif
