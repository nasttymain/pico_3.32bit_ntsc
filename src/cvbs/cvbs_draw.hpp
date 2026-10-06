#include <cstdint>

#include "cvbs_core.hpp"

#ifndef __NASTTY_CVBS_DRAW_H__
#define __NASTTY_CVBS_DRAW_H__

void lcscolor(uint8_t chroma, uint8_t luma, uint8_t saturation);
void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void boxf(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void box(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void fill(int16_t x, int16_t y);
void clrgraph(uint8_t clr_mode);
void triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);
void trianglef(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);
void pos(int16_t x, int16_t y);
void gcopy(uint8_t window_id, int16_t x1, int16_t y1, int16_t xsize, int16_t ysize);
void circle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fill_mode);

#endif