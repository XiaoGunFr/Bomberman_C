#include "grid.h"

// Ajout des murs à des positions random
void add_rand_walls(board *board)
{
    int x, y;
    set_grid(board, 0, 0, 5);
    set_grid(board, 0, board->h - 1, 6);
    set_grid(board, board->w - 1, 0, 7);
    set_grid(board, board->w - 1, board->h - 1, 8);

    for (y = 0; y < board->h; y++)
    {
        for (x = 0; x < board->w; x++)
        {
            if (!is_coin(board, x, y))
            {
                if (rand() % 11 == 0)
                {
                    // Murs cassables
                    board->grid[y * board->w + x] = 2;
                }
                else if (rand() % 13 == 2)
                {
                    // Murs incassables
                    board->grid[y * board->w + x] = 3;
                }
            }
        }
    }
}

void setup_board(board *board, int l, int c)
{

    int lines = l;
    int columns = c;
    // getmaxyx(stdscr, lines, columns);
    board->h = lines;   // 2 rows reserved for border, 1 row for chat
    board->w = columns; // 2 columns reserved for border
    board->grid = calloc((board->w) * (board->h), sizeof(char));
    add_rand_walls(board);
}

// Fonction pour poser une bombe
void set_up_bomb(board *board, int x, int y)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    // Verrouiller le mutex
    pthread_mutex_lock(&mutex);

    if (!is_bomb(board, x, y))
    {
        // Poser la bombe sur le tableau
        set_grid(board, x, y, 4);
        // Créer un thread pour les 3 secondes
        Bomb *bomb = malloc(sizeof(Bomb));
        bomb->x = x;
        bomb->y = y;
        bomb->board = board;
        pthread_t tid;
        pthread_create(&tid, NULL, bomb_timer, bomb);
        pthread_detach(tid);
    }
    // Déverrouiller le mutex
    pthread_mutex_unlock(&mutex);
}

// Fonction pour le timer de la bombe
void *bomb_timer(void *arg)
{
    Bomb *bomb = (Bomb *)arg;
    sleep(3);
    explode_bomb(bomb->board, bomb->x, bomb->y);
    set_grid(bomb->board, bomb->x, bomb->y, 0);
    free(bomb);
    return NULL;
}

// Explose bombe autour de la position (x,y)
void explode_bomb(board *board, int x, int y)
{
    // À gauche
    if (!is_unbreakable_wall(board, x - 1, y) && !is_off_board(board, x - 1, y))
    {
        /*if (is_player(board, x - 1, y))
        {
            printf("YOU DIED\n");
        }*/
        // Deux à gauche
        if (get_grid(board, x - 1, y) == 0)
        {
            if ((is_breakable_wall(board, x - 2, y) || is_player(board, x - 2, y)) && !is_off_board(board, x - 2, y))
            {
                set_grid(board, x - 2, y, 0);
            }
        }
        set_grid(board, x - 1, y, 0);
    }
    // À droite
    if (!is_unbreakable_wall(board, x + 1, y) && !is_off_board(board, x + 1, y))
    {
        // Deux à droite
        if (get_grid(board, x + 1, y) == 0)
        {
            if ((is_breakable_wall(board, x + 2, y) || is_player(board, x + 2, y)) && !is_off_board(board, x + 2, y))
            {
                set_grid(board, x + 2, y, 0);
            }
        }
        set_grid(board, x + 1, y, 0);
    }
    // En bas
    if (!is_unbreakable_wall(board, x, y - 1) && !is_off_board(board, x, y - 1))
    {
        // Deux en bas
        if (get_grid(board, x, y - 1) == 0)
        {
            if ((is_breakable_wall(board, x, y - 2) || is_player(board, x, y - 2)) && !is_off_board(board, x, y - 2))
            {
                set_grid(board, x, y - 2, 0);
            }
        }
        set_grid(board, x, y - 1, 0);
    }
    // En haut
    if (!is_unbreakable_wall(board, x, y + 1) && !is_off_board(board, x, y + 1))
    {
        // Deux en haut
        if (get_grid(board, x, y + 1) == 0)
        {
            if ((is_breakable_wall(board, x, y + 2) || is_player(board, x, y - 2)) && !is_off_board(board, x, y + 2))
            {
                set_grid(board, x, y + 2, 0);
            }
        }
        set_grid(board, x, y + 1, 0);
    }
    // En haut à gauche
    if ((is_breakable_wall(board, x - 1, y - 1) || is_player(board, x - 1, y - 1)) && !is_off_board(board, x - 1, y - 1))
    {
        set_grid(board, x - 1, y - 1, 0);
    }
    // En bas à droite
    if ((is_breakable_wall(board, x + 1, y - 1) || is_player(board, x + 1, y - 1)) && !is_off_board(board, x + 1, y - 1))
    {
        set_grid(board, x + 1, y - 1, 0);
    }
    // En haut à gauche
    if ((is_breakable_wall(board, x - 1, y + 1) || is_player(board, x - 1, y + 1)) && !is_off_board(board, x - 1, y + 1))
    {
        set_grid(board, x - 1, y + 1, 0);
    }
    // En haut à droite
    if ((is_breakable_wall(board, x + 1, y + 1) || is_player(board, x + 1, y + 1)) && !is_off_board(board, x + 1, y + 1))
    {
        set_grid(board, x + 1, y + 1, 0);
    }
}

bool is_coin(board *b, int x, int y)
{
    if ((y == 0 || y == b->h - 1) && (x == 0 || x == b->w - 1))
    {
        return true;
    }
    return false;
}

bool is_off_board(board *board, int x, int y)
{
    return x < 0 || x >= board->w || y < 0 || y >= board->h;
}
bool is_player_next(board *board, int x, int y)
{
    if (get_grid(board, x, y) == 5 || get_grid(board, x, y) == 6 || get_grid(board, x, y) == 7 || get_grid(board, x, y) == 8)
    {
        return true;
    }
    return false;
}

void free_board(board *board)
{
    free(board->grid);
}

int get_grid(board *b, int x, int y)
{
    return b->grid[y * b->w + x];
}

void set_grid(board *b, int x, int y, int v)
{
    b->grid[y * b->w + x] = v;
}

void affiche_grid(board *b)
{
    int x, y;
    for (y = 0; y < b->h; y++)
    {
        for (x = 0; x < b->w; x++)
        {
            char c;
            switch (get_grid(b, x, y))
            {
            case 0:
                c = ' ';
                break;
            case 1:
                c = 'O';
                break;
            // Murs cassables
            case 2:
                c = '*';
                break;
            // Murs incassables
            case 3:
                c = '#';
                break;
            case 4:
                c = '!';
                break;
            default:
                c = '?';
                break;
            }
            mvaddch(y + 1, x + 1, c);
        }
    }
    attroff(A_BOLD);        // Disable bold
    attroff(COLOR_PAIR(1)); // Disable custom color 1
    refresh();
}
void test()
{
    printf("hello");
}
void refresh_game(board *b, line *l, pos *p)
{
    // Update grid
    int x, y;
    for (y = 0; y < b->h; y++)
    {
        for (x = 0; x < b->w; x++)
        {
            char c;
            switch (get_grid(b, x, y))
            {
            case 0:
                c = ' ';
                break;
            case 1:
                c = 'O';
                break;
            // Murs cassables
            case 2:
                c = '*';
                break;
            // Murs incassables
            case 3:
                c = '#';
                break;
            case 4:
                c = '!';
                break;
            case 5:
                c = '1';
                break;
            case 6:
                c = '2';
                break;
            case 7:
                c = '3';
                break;
            case 8:
                c = '4';
                break;
            default:
                c = '?';
                break;
            }
            mvaddch(y + 1, x + 1, c);
        }
    }

    // Update border
    for (x = 0; x < b->w + 2; x++)
    {
        mvaddch(0, x, '-');
        mvaddch(b->h + 1, x, '-');
    }
    for (y = 0; y < b->h + 2; y++)
    {
        mvaddch(y, 0, '|');
        mvaddch(y, b->w + 1, '|');
    }

    // Update chat text
    attron(COLOR_PAIR(1)); // Enable custom color 1
    attron(A_BOLD);        // Enable bold
    for (x = 0; x < b->w + 2; x++)
    {
        if (x >= TEXT_SIZE || x >= l->cursor)
            mvaddch(b->h + 2, x, ' ');
        else
            mvaddch(b->h + 2, x, l->data[x]);
    }
    attroff(A_BOLD);        // Disable bold
    attroff(COLOR_PAIR(1)); // Disable custom color 1
    refresh();              // Apply the changes to the terminal
}

ACTION control(line *l)
{
    int c;
    int prev_c = ERR;
    // We consume all similar consecutive key presses
    while ((c = getch()) != ERR)
    { // getch returns the first key press in the queue
        if (prev_c != ERR && prev_c != c)
        {
            ungetch(c); // put 'c' back in the queue
            break;
        }
        prev_c = c;
    }
    ACTION a = NONE;
    switch (prev_c)
    {
    case 27: // touche alt apparemtn
        a = SEND;
        break;
    case ERR:
        break;
    case KEY_LEFT:
        a = LEFT;

        break;
    case KEY_RIGHT:
        a = RIGHT;

        break;
    case KEY_UP:
        a = UP;

        break;
    case KEY_DOWN:
        a = DOWN;

        break;
    case '\n':
        a = ENTER;
        break;
    case '~':
        a = QUIT;
        break;
    case KEY_BACKSPACE:
        if (l->cursor > 0)
            l->cursor--;
        break;
    default:
        if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE)
            l->data[(l->cursor)++] = prev_c;
        break;
    }
    return a;
}

// On vérifie qu'il n'y a pas de mur càd que la case n'est ni 2 ni 3
bool is_wall_or_bomb(board *b, int x, int y)
{
    return (get_grid(b, x, y) == 2 || get_grid(b, x, y) == 3 || get_grid(b, x, y) == 4);
}

bool is_breakable_wall(board *b, int x, int y)
{
    return (get_grid(b, x, y) == 2);
}

bool is_unbreakable_wall(board *b, int x, int y)
{
    return (get_grid(b, x, y) == 3);
}

bool is_bomb(board *b, int x, int y)
{
    return (get_grid(b, x, y) == 4);
}

bool is_player(board *b, int x, int y)
{
    return (get_grid(b, x, y) == 5 || get_grid(b, x, y) == 6 || get_grid(b, x, y) == 7 || get_grid(b, x, y) == 8);
}

bool perform_action(board *b, pos *p, ACTION a, int id_joueur)
{
    int xd = 0;
    int yd = 0;
    int prev_x = p->x;
    int prev_y = p->y;

    switch (a)
    {
    case LEFT:
        xd = -1;
        yd = 0;
        break;
    case RIGHT:
        xd = 1;
        yd = 0;
        break;
    case UP:
        xd = 0;
        yd = -1;
        break;
    case DOWN:
        xd = 0;
        yd = 1;
        break;
    case ENTER:
        set_up_bomb(b, p->x, p->y);
        break;
    case P:
        explode_bomb(b, p->x, p->y);
        break;
    case QUIT:
        return true;
    default:
        break;
    }

    int new_x = (p->x + xd + b->w) % b->w;
    int new_y = (p->y + yd + b->h) % b->h;

    // On vérifie que la case suivante n'est ni un mur ni une bombe
    if (is_wall_or_bomb(b, new_x, new_y))
    {
        return false;
    }
    if (is_player_next(b, new_x, new_y))
    {
        return false;
    }
    // On efface les traces de déplacements
    if (new_x != prev_x || new_y != prev_y)
    {
        if (!is_wall_or_bomb(b, prev_x, prev_y))
        {
            set_grid(b, prev_x, prev_y, 0);
        }
    }

    p->x = new_x;
    p->y = new_y;
    set_grid(b, p->x, p->y, 5 + id_joueur);
    return false;
}

// void start_game()
// {
//     board *b = malloc(sizeof(board));
//     ;
//     line *l = malloc(sizeof(line));
//     l->cursor = 0;
//     pos *p = malloc(sizeof(pos));
//     p->x = 0;
//     p->y = 0;

//     // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
//     initscr();                               /* Start curses mode */
//     raw();                                   /* Disable line buffering */
//     intrflush(stdscr, FALSE);                /* No need to flush when intr key is pressed */
//     keypad(stdscr, TRUE);                    /* Required in order to get events from keyboard */
//     nodelay(stdscr, TRUE);                   /* Make getch non-blocking */
//     noecho();                                /* Don't echo() while we do getch (we will manually print characters when relevant) */
//     curs_set(0);                             // Set the cursor to invisible
//     start_color();                           // Enable colors
//     init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

//     setup_board(b);
//     while (true)
//     {
//         ACTION a = control(l);
//         if (perform_action(b, p, a))
//             break;
//         refresh_game(b, l, p);
//         usleep(30 * 1000);
//     }
//     free_board(b);

//     curs_set(1); // Set the cursor to visible again
//     endwin();    /* End curses mode */

//     free(p);
//     free(l);
//     free(b);
// }
// void start_game_multicast(board *b, line *l, pos *p)
// {
//     b = malloc(sizeof(board));
//     ;
//     l = malloc(sizeof(line));
//     l->cursor = 0;
//     p = malloc(sizeof(pos));
//     p->x = 0;
//     p->y = 0;

//     // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
//     initscr();                               /* Start curses mode */
//     raw();                                   /* Disable line buffering */
//     intrflush(stdscr, FALSE);                /* No need to flush when intr key is pressed */
//     keypad(stdscr, TRUE);                    /* Required in order to get events from keyboard */
//     nodelay(stdscr, TRUE);                   /* Make getch non-blocking */
//     noecho();                                /* Don't echo() while we do getch (we will manually print characters when relevant) */
//     curs_set(0);                             // Set the cursor to invisible
//     start_color();                           // Enable colors
//     init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

//     setup_board(b);
// }
