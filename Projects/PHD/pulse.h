#ifndef __PULSE_H__
#define __PULSE_H__

#include <stdint.h>

#define NSAMPLE 24
#define NSLOT   4

typedef struct {
    int32_t sample_avg_total;
} dc_filter_type;

typedef struct {
    int16_t buffer[NSLOT];
    uint8_t nextslot; 
} ma_filter_type;

typedef struct {
    dc_filter_type dc;
    ma_filter_type ma;
    int16_t amplitude_avg_total;
    int16_t cycle_max;
    int16_t cycle_min;
    uint8_t positive;
    int16_t prev_sig;
} pulse_type;

pulse_type* pulse_init();
uint8_t pulse_isBeat(pulse_type *p, int16_t signal);
int16_t pulse_dc_filter(pulse_type *p, int32_t sample);
int16_t pulse_ma_filter(pulse_type *p, int32_t sample);
int32_t pulse_avg_dc(pulse_type *p);
int16_t pulse_avg_ac(pulse_type *p);
ma_filter_type* ma_filter_init();
int16_t bpm_filter(ma_filter_type *p, int16_t value);

#endif // __PULSE_H__