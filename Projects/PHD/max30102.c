#include <stdlib.h>
#include <string.h>
#include "i2c.h"
#include "max30102.h"
#include "pulse.h"

extern I2C_ConfigStruct i2c;

extern void delay(__IO uint32_t milliseconds);

const uint8_t spo2_table[184] = {
  95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99,
  99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97,
  97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91,
  90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81,
  80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67,
  66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50,
  49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29,
  28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 3, 2, 1
};

static uint8_t readRegister8(uint8_t reg)
{
    int ret = 0;
    uint8_t r_data = 0;

    ret = I2C_Write(&i2c, MAX30105_ADDRESS << 1, &reg, 1);

    if(ret) {
        printf("[readRegister8] I2C_Write error(%d)\r\n", ret);
        return -1;
    }

    ret = I2C_Read(&i2c, MAX30105_ADDRESS << 1, &r_data, 1);

    if(ret) {
        printf("[readRegister8] I2C_Read error(%d)\r\n", ret);
        return -1;
    }

    return r_data;
}

static uint8_t writeRegister8(uint8_t reg, uint8_t value)
{
    int ret;

    uint8_t data[2];

    data[0] = reg;
    data[1] = value;

    ret = I2C_Write(&i2c, MAX30105_ADDRESS << 1, data, 2);
    if(ret) {
        printf("I2C_Write error\r\n");
        return -1;
    }
    return 0;
}

sense_type* max30102_init()
{
    sense_type *p = malloc(sizeof(sense_type));
    memset(p, 0x0, sizeof(sense_type));
    return p;
}

int max30102_begin()
{
    uint8_t id = 0;
    id = readRegister8(REG_PART_ID);

    if(id != MAX_30102_ID) {
        printf("MAX30102 not found(id:0x%X)\r\n", id);
        return -1;
    }
    printf("MAX30102 device found!!!\r\n");
    return 0;
}

void max30102_setup()
{
    writeRegister8(REG_MODE_CONFIG, 0x40);
    delay(500);
    writeRegister8(REG_FIFO_WR_PTR, 0x00);
    writeRegister8(REG_OVF_COUNTER, 0x00);
    writeRegister8(REG_FIFO_RD_PTR, 0x00);
    writeRegister8(REG_FIFO_CONFIG, 0x4f);
    writeRegister8(REG_MODE_CONFIG, 0x03);
    writeRegister8(REG_SPO2_CONFIG, 0x27);
    writeRegister8(REG_LED1_PA, 0x17);
    writeRegister8(REG_LED2_PA, 0x17);
}

uint8_t max30102_available(sense_type* p) {
  int8_t numberOfSamples = p->head - p->tail;
  if (numberOfSamples < 0) numberOfSamples += STORAGE_SIZE;
  return (numberOfSamples);
}

uint32_t max30102_getRed(sense_type* p) {
  return (p->red[p->tail]);
}

uint32_t max30102_getIR(sense_type* p) {
  return (p->ir[p->tail]);
}

void max30102_nextSample(sense_type* p) {
  if(max30102_available(p)) {
    p->tail++;
    p->tail %= STORAGE_SIZE;
  }
}

uint16_t max30102_check(sense_type* p) {
    int ret = 0;
    uint8_t reg[2] = {0, };
    uint8_t sample[32] = {0, };

    uint8_t readPointer = readRegister8(REG_FIFO_RD_PTR);
    uint8_t writePointer = readRegister8(REG_FIFO_WR_PTR);
    int numberOfSamples = 0;

    if(readPointer != writePointer) {
        numberOfSamples = writePointer - readPointer;

        if(numberOfSamples < 0)
            numberOfSamples += 32;

        int bytesLeftToRead = numberOfSamples * 6;
        bytesLeftToRead = bytesLeftToRead <= 32 ? bytesLeftToRead : 32;

        reg[0] = REG_FIFO_DATA;

        ret = I2C_WriteRepeated(&i2c, MAX30105_ADDRESS << 1, reg, 1);
        if(ret) {
            printf("[chk] I2C_Write error(%d)\r\n", ret);
            return -1;
        }

        ret = I2C_Read(&i2c, MAX30105_ADDRESS << 1, sample, bytesLeftToRead);
        if(ret) {
            printf("[chk] I2C_Read error(%d)\r\n", ret);
            return -1;
        }

        uint8_t idx = 0;

        while(bytesLeftToRead > 0) {
            p->head++;
            p->head %= STORAGE_SIZE;
            p->ir[p->head] = ((uint32_t)(sample[idx+0] << 16) | (uint32_t)(sample[idx+1] << 8) | (uint32_t)(sample[idx+2])) & 0x3ffff;;

            p->red[p->head] = ((uint32_t)(sample[idx+3] << 16) | (uint32_t)(sample[idx+4] << 8) | (uint32_t)(sample[idx+5])) & 0x3ffff;;
            bytesLeftToRead -= 6;
            idx += 6;
        }
    }

    return (numberOfSamples);
}