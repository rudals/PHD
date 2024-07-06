#include "waveform.h"
#include "ssd1306.h"

waveform_type* waveform_init()
{
    waveform_type* p = malloc(sizeof(waveform_type));
    memset(p, 0x0, sizeof(waveform_type));
    return p;
}

void waveform_reset(waveform_type* p)
{
  if(p) memset(p, 0x0, sizeof(waveform_type));
}

void waveform_record(waveform_type* p, int value)
{
    value = value / 8;
    value += 128;
    value = value < 0 ? 0 : value;
    p->waveData[p->waveIdx] = (uint8_t)(value > 255) ? 255 : value;
    p->waveTime[p->waveIdx] = millis();
    p->waveIdx = (p->waveIdx + 1) % MAXWAVE;
}

void waveform_scale(waveform_type* p)
{
  uint8_t maxw = 0;
  uint8_t minw = 255;
  for (int i = 0; i < MAXWAVE; i++) {
    maxw = p->waveData[i] > maxw ? p->waveData[i] : maxw;
    minw = p->waveData[i] < minw ? p->waveData[i] : minw;
  }
  
  uint8_t scale8 = (maxw - minw) / 3 + 1;
  uint8_t index = p->waveIdx;

  for (int i = 0; i < MAXWAVE; i++) {
    p->dispWave[i] = 31 - ((uint16_t)(p->waveData[index] - minw) * 8) / scale8;
    p->ppgTime[i] = p->waveTime[index];
    index = (index + 1) % MAXWAVE;
  }
}

void waveform_draw(waveform_type* p, uint8_t X)
{
  int y_pos = 32;
  int prevData = 0;
  uint8_t y = 31;
  int temp = 0;
  long prevTime = 0;
  uint8_t lastData = p->dispWave[0];

  for (int i = 0; i < MAXWAVE; i++) {
    y = p->dispWave[i];
    
    drawPixel(X + i*2 + 0, y + y_pos);
    drawPixel(X + i*2 + 1, y + y_pos);
    drawPixel(X + i*2 + 2, y + y_pos);
    if (i < MAXWAVE - 1) {
      uint8_t nexty = p->dispWave[i + 1];
      if (nexty > y) {
        for (uint8_t iy = y + 1; iy < nexty; ++iy)
          drawPixel(X + i*2 + 2, iy + y_pos);
      } else if (nexty < y) {
        for (uint8_t iy = nexty + 1; iy < y; ++iy)
          drawPixel(X + i*2 + 2, iy + y_pos);
      }
    }
  }
}