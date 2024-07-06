#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wizchip_conf.h"
#include "i2c.h"
#include "ssd1306.h"
#include "font.h"

extern wiz_NetInfo gWIZNETINFO;
extern I2C_ConfigStruct i2c;
extern int peak, beatAvg, SPO2, SPO2f;

char digit_buffer[10];
char ip_buffer[20];

uint8_t currentPage = 0;
uint8_t pageBuf[COLUMNS];
uint32_t count = 0;

static const uint8_t ssd1306_configuration[] = {
    0xA8, 0x3F,
    0xD3, 0x00,
    0x40,
    0xA1,
    0xC8,
    0xDA, 0x12,
    0x81, 0xFF,
    0xA4,
    0xA6,
    0xD5, 0x80,
    0x8D, 0x14,
    0xAF
};

static const uint8_t heart_bits[] = {
    0x00, 0x00, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0xfe, 0xfe, 0xff,
    0xfe, 0xff, 0xfc, 0x7f, 0xf8, 0x3f, 0xf0, 0x1f, 0xe0, 0x0f,
    0xc0, 0x07, 0x80, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

void command_start(void)
{
    I2C_Start(&i2c);

    if(I2C_WriteByte(&i2c, SSD1306_I2C_ADDR << 1)) {
        printf("Received NACK at address phase!!\r\n");
        return;
    }

    if(I2C_WriteByte(&i2c, 0x00)) {
        printf("Received NACK at address phase!!\r\n");
        return;
    }
}

void command_stop(void)
{
    I2C_Stop(&i2c);
}

void command(uint8_t command)
{
    command_start();

    if(I2C_WriteByte(&i2c, command)) {
        printf("Received NACK at address phase!!\r\n");
    }

    command_stop();
}

void data_start(void)
{
    I2C_Start(&i2c);

    if(I2C_WriteByte(&i2c, SSD1306_I2C_ADDR << 1)) {
        printf("Received NACK at address phase!!\r\n");
        return;
    }

    if(I2C_WriteByte(&i2c, 0x40)) {
        printf("Received NACK at address phase!!\r\n");
        return;
    }
}

void data_stop(void)
{
    I2C_Stop(&i2c);
}

void data_byte(uint8_t data)
{
    if(I2C_WriteByte(&i2c, data)) {
        // push data if detect buffer used up
        data_stop();
        data_start();

        if(I2C_WriteByte(&i2c, data)) {
            printf("Received NACK at address phase!!\r\n");
        }
    }
}

void set_area(uint8_t col, uint8_t page, uint8_t col_range, uint8_t page_range)
{
    command_start();
    I2C_WriteByte(&i2c, 0x20);
    I2C_WriteByte(&i2c, 0x01);
    I2C_WriteByte(&i2c, 0x21);
    I2C_WriteByte(&i2c, col);
    I2C_WriteByte(&i2c, col + col_range - 1);
    I2C_WriteByte(&i2c, 0x22);
    I2C_WriteByte(&i2c, page);
    I2C_WriteByte(&i2c, page + page_range - 1);
    command_stop();
}

void oled_init()
{
    for(uint8_t i = 0; i < sizeof(ssd1306_configuration); i++) {
        command(ssd1306_configuration[i]);
    }

    currentPage = 0;

    for(uint8_t i = 0; i < COLUMNS; ++i)
        pageBuf[i] = 0x00;
}

void oled_fill(uint8_t data)
{
    set_area(0, 0, COLUMNS, PAGES);
    uint16_t size = (COLUMNS) * (PAGES);
    data_start();

    for(uint16_t i = 0; i < size; i++) {
        data_byte(data);
    }

    data_stop();
}

void writePage(uint8_t page)
{
    set_area(0, page, COLUMNS, 1);
    data_start();

    for(uint8_t i = 0; i < COLUMNS; i++) {
        data_byte(pageBuf[i]);
    }

    data_stop();
}


uint8_t inPage(uint8_t y, uint8_t h)
{
    return currentPage >= y / 8 && currentPage <= (y + h - 1) / 8;
}

void drawPixel(uint8_t x, uint8_t y)
{
    if(y / 8 != currentPage)
        return;

    pageBuf[x] |= 1 << (y % 8);
}

uint16_t Stretch(uint16_t x)
{
    x = (x & 0xF0) << 4 | (x & 0x0F);
    x = (x << 2 | x) & 0x3333;
    x = (x << 1 | x) & 0x5555;
    return x | x << 1;
}

void oled_drawChar(int x, int y,  unsigned char c, const int BIG)
{
    uint16_t data;

    for(uint8_t i = 0; i < FONT_WIDTH; i++) {
        data = font_bitmap[c - FONT_START ][i];

        if(BIG == 2)
            data = Stretch(data);

        for(uint8_t d = 0; d < BIG; d++) {
            for(uint8_t j = 0; j < FONT_HEIGHT * BIG; j++) {
                if(data & (1 << j))
                    drawPixel(x + (i * BIG + d), y + j);
            }
        }
    }
}

void oled_drawStr(int x, int y, const char *s, const int BIG)
{
    if(!inPage(y, FONT_HEIGHT * BIG))
        return;

    for(int k = 0; s[k] != '\0'; ++k) {
        oled_drawChar(x, y, s[k], BIG);
        x += (FONT_WIDTH + 1) * BIG;
    }
}

void oled_drawXBMP(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap)
{
    uint8_t data = 0;

    if(!inPage(y, h))
        return;

    uint8_t bytewidth = (w % 8 == 0) ? w / 8 : w / 8 + 1;

    for(int j = 0; j < h; ++j) {
        for(int i = 0; i < w; ++i) {
            uint8_t bitno = i % 8;

            if(bitno == 0)
                data = bitmap[j * bytewidth + i / 8];

            if(data & (1 << bitno))
                drawPixel(x + i, y + j);
        }
    }
}

void oled_firstPage(void)
{
    currentPage = 0;
}

uint8_t oled_nextPage(void)
{
    writePage(currentPage++);
    memset(pageBuf, 0x00, COLUMNS);
    return currentPage != PAGES;
}

void print_digit(int x, int y, long val, char c , uint8_t field, const int BIG)
{
  uint8_t ff = field;
  do
  {
    char ch = (val != 0) ? val % 10 + '0' : c;
    oled_drawChar(x + BIG * (ff - 1) * 6, y, ch, BIG);
    val = val / 10;
    --ff;
  } while (ff > 0);
}
#include "waveform.h"
extern waveform_type* p_wave;

void oled_draw(int msg)
{
    oled_firstPage();

    do {
        switch(msg) {
            case 0:
                oled_drawStr(10, 0, "Device error", 1);
                break;

            case 1:
                oled_drawStr(20, 5, "W7500-Surf Demo", 1);
                oled_drawStr(5, 20, "Personal", 1);
                oled_drawStr(40, 30, "Healthcare", 1);
                oled_drawStr(80, 40, "Device", 1);
                oled_drawStr(15, 55, "IP requesting...", 1);
                break;

            case 2:
                oled_drawStr(27, 15, "Place finger", 1);
                sprintf(ip_buffer, "%d.%d.%d.%d",
                    gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
                oled_drawStr(27, 55, ip_buffer, 1);
                break;

            case 3:
                oled_drawStr(15, 2, "HR(bpm)", 1);
                print_digit(10, 14, beatAvg, ' ', 3, 2);
                oled_drawStr(83, 2, "SpO2", 1);
                print_digit(70, 14, SPO2, ' ', 3, 2);
                oled_drawChar(110, 14, '%', 2);
                waveform_draw(p_wave, 0);
                break;
        }
    } while(oled_nextPage());
}