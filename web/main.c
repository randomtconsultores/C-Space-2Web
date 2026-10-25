#include "levels.h"
#include "framebuffer.h"
_Static_assert(LARGURA_TELA == WEB_WIDTH, "Framebuffer width mismatch");
_Static_assert(ALTURA_TELA == WEB_HEIGHT, "Framebuffer height mismatch");
static char mapa[ALTURA_TELA][LARGURA_MAPA];
/* First browser increment: original level generation, not a gameplay loop. */
int main(void)
{
    gerar_mapa_dinamico(mapa, 12345u, 0);
    web_clear();
    for (int y = 0; y < ALTURA_TELA; ++y) {
        for (int x = 0; x < LARGURA_TELA; ++x) {
            unsigned char ch = (unsigned char)mapa[y][x];
            if (x == 0 || x == LARGURA_TELA - 1)
                ch = '|';
            else if (ch == '#' && y % 2 != 0)
                ch = '=';
            web_put(x, y, ch);
        }
    }
    return 0;
}
