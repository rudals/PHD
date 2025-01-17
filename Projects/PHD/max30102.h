#ifndef __MAX30102_H__
#define __MAX30102_H__

#include <stdint.h>

#define MAX30105_ADDRESS 0x57
#define MAX_30102_ID    0x15

#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D

#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21

#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

#define STORAGE_SIZE 3

typedef struct {
    uint32_t red[STORAGE_SIZE];
    uint32_t ir[STORAGE_SIZE];
    uint8_t head;
    uint8_t tail;
} sense_type;

sense_type* max30102_init();
int max30102_begin();
void max30102_setup();
uint8_t max30102_available(sense_type* p);
uint32_t max30102_getRed(sense_type* p);
uint32_t max30102_getIR(sense_type* p);
void max30102_nextSample(sense_type* p);
uint16_t max30102_check(sense_type* p);

#endif // __MAX30102_H__