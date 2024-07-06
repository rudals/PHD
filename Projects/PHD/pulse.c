#include <stdlib.h>
#include <string.h>
#include "pulse.h"

pulse_type* pulse_init()
{
    pulse_type *p = malloc(sizeof(pulse_type));
    memset(p, 0x0, sizeof(pulse_type));
    p->cycle_max = 20;
    p->cycle_min = -20;
    p->positive = 0;
    p->prev_sig = 0;
    p->amplitude_avg_total = 0;
    return p;
}

void pulse_deinit(pulse_type *p)
{
    if(p != NULL)
        free(p);
}

uint8_t pulse_isBeat(pulse_type *p, int16_t signal)
{
    uint8_t beat = 0;
    
    if(p->positive && (signal > p->prev_sig))
        p->cycle_max = signal;

    if(!p->positive && (signal < p->prev_sig))
        p->cycle_min = signal;

    if(p->positive && (signal < p->prev_sig)) {
        int amplitude = p->cycle_max - p->cycle_min;

        if(amplitude > 20 && amplitude < 3000) {
            beat = 1;
            p->amplitude_avg_total += (amplitude - p->amplitude_avg_total / 4);
        }

        p->cycle_min = 0;
        p->positive = 0;
    }

    if(!p->positive && (signal > p->prev_sig)) {
        p->cycle_max = 0;
        p->positive = 1;
    }

    p->prev_sig = signal;
    return beat;
}

int16_t pulse_dc_filter(pulse_type *p, int32_t sample)
{
    p->dc.sample_avg_total += (sample - p->dc.sample_avg_total / NSAMPLE);
    return (int16_t)(sample - p->dc.sample_avg_total / NSAMPLE);
}

int16_t pulse_ma_filter(pulse_type *p, int32_t sample)
{
    int16_t total = 0;
    p->ma.buffer[p->ma.nextslot] = sample;
    p->ma.nextslot = (p->ma.nextslot + 1) % NSLOT;

    for(int i = 0; i < NSLOT; ++i) {
        total += p->ma.buffer[i];
    }

    return total / NSLOT;
}

int32_t pulse_avg_dc(pulse_type *p)
{
    return p->dc.sample_avg_total / NSAMPLE;
}

int16_t pulse_avg_ac(pulse_type *p)
{
    return (p->amplitude_avg_total / 4);
}

ma_filter_type* ma_filter_init()
{
    ma_filter_type *p = malloc(sizeof(ma_filter_type));
    memset(p, 0x0, sizeof(ma_filter_type));
    return p;
}

int16_t bpm_filter(ma_filter_type *p, int16_t value)
{
    int16_t total = 0;

    p->buffer[p->nextslot] = value;
    p->nextslot = (p->nextslot + 1) % NSLOT;

    for(int i = 0; i < NSLOT; ++i)
        total += p->buffer[i];

    return total / NSLOT;
}