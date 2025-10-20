#ifndef GRID_H
#define GRID_H

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define TEXT_SIZE 255

typedef struct board
{
    char *grid;
    int w;
    int h;
} board;

typedef struct
{
    int x;
    int y;
    board *board;
} Bomb;

typedef enum ACTION
{
    UP,
    LEFT,
    DOWN,
    RIGHT,
    ENTER,
    QUIT,
    P,
    NONE,
    SEND,
    
} ACTION;

typedef struct line
{
    char data[TEXT_SIZE];
    int cursor;
} line;

typedef struct pos
{
    int x;
    int y;
} pos;

extern void test();
extern bool is_coin(board *b, int x, int y);
bool is_player_next(board* board, int x, int y);
extern void affiche_grid(board *b);
extern void add_rand_walls(board *board);
extern void setup_board(board *board, int l, int c);
extern void set_up_bomb(board *board, int x, int y);
extern void free_board(board *board);
extern int get_grid(board *b, int x, int y);
extern void set_grid(board *b, int x, int y, int v);
extern void refresh_game(board *b, line *l, pos *p);
extern ACTION control(line *l);
extern bool is_wall_or_bomb(board *b, int x, int y);
extern bool perform_action(board *b, pos *p, ACTION a,int id_joueur);
extern bool is_bomb(board *b, int x, int y);
extern bool is_breakable_wall(board *b, int x, int y);
extern bool is_off_board(board *board, int x, int y);
extern bool is_affected_by_bomb(board *board, int x, int y);
extern void explode_bomb(board *board, int x, int y);
extern bool is_player(board *b, int x, int y);
extern void *bomb_timer(void *arg);
extern bool is_unbreakable_wall(board *b, int x, int y);
extern void start_game();
extern void start_game_multicast(board *b, line *l, pos *p);

#endif