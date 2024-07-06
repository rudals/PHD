#ifndef __WAVEFORM_H__
#define __WAVEFORM_H__

#include <stdint.h>

#define MAXWAVE 60

typedef struct {
    uint8_t waveIdx;
    
    uint8_t waveData[MAXWAVE];
    uint32_t waveTime[MAXWAVE];

    uint8_t dispWave[MAXWAVE];
    uint32_t ppgTime[MAXWAVE];
} waveform_type;

waveform_type* waveform_init();
void waveform_reset(waveform_type* p);
void waveform_record(waveform_type* p, int value);
void waveform_scale(waveform_type* p);
void waveform_draw(waveform_type* p, uint8_t X);

#endif //__WAVEFORM_H__