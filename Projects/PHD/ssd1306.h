#ifndef __SSD1306_H__
#define __SSD1306_H__

#define SSD1306_I2C_ADDR 0x3C
#define SCREEN_128X64
#define COLUMNS 0x0080
#define PAGES 0x08

void oled_init();
void oled_fill(uint8_t data);
uint16_t Stretch (uint16_t x);
void drawPixel(uint8_t x, uint8_t y);
void oled_drawChar(int x, int y,  unsigned char c, const int BIG);
void oled_drawStr(int x, int y, const char *s, const int BIG);
void oled_drawXBMP(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap);
void oled_firstPage(void);
uint8_t oled_nextPage(void);
void oled_draw(int msg);

#endif //__SSD1306_H__