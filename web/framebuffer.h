#ifndef WEB_FRAMEBUFFER_H
#define WEB_FRAMEBUFFER_H
#define WEB_WIDTH 40
#define WEB_HEIGHT 15
#define WEB_STRIDE (WEB_WIDTH + 1)
void web_clear(void);
int web_put(int x, int y, unsigned char ch);
const unsigned char *web_buffer(void);
int web_buffer_size(void);
int web_width(void);
int web_height(void);
#endif
