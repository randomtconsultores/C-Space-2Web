#ifndef WEB_TERMINAL_H
#define WEB_TERMINAL_H
#include "framebuffer.h"
#include "runtime.h"
/* Only the terminal operations used by space.c, boss.c and draw.c. */
#define TRUE 1
#define FALSE 0
#define ERR (-1)
#define stdscr ((void *)0)
#define COLOR_GREEN 2
#define COLOR_BLACK 0
#define COLOR_PAIR(n) (n)
static inline void *initscr(void) { web_clear(); return stdscr; }
static inline int noecho(void) { return 0; }
static inline int cbreak(void) { return 0; }
static inline int curs_set(int n) { (void)n; return 0; }
static inline int keypad(void *w, int n) { (void)w; (void)n; return 0; }
static inline int has_colors(void) { return 0; }
static inline int start_color(void) { return 0; }
static inline int init_pair(int n, int f, int b) { (void)n; (void)f; (void)b; return 0; }
static inline int bkgd(int n) { (void)n; return 0; }
static inline int nodelay(void *w, int n) { (void)w; (void)n; return 0; }
static inline int clear(void) { web_clear(); return 0; }
static inline int refresh(void) { return 0; }
static inline int endwin(void) { return 0; }
static inline int getch(void) { return web_getch(); }
static inline int flushinp(void) { web_flush_pressed(); return 0; }
static inline int mvaddch(int y, int x, unsigned int ch) { return web_put(x, y, (unsigned char)ch) ? 0 : ERR; }
int web_mvprintw(int y, int x, const char *format, ...);
#define mvprintw web_mvprintw
#endif
