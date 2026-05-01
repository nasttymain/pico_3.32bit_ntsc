#include "rca.hpp"
#include "rcavt.hpp"
#include <stdio.h>

namespace bouncing_squares{
    constexpr const uint16_t BOXCOUNT = 96;
    float bx[BOXCOUNT];
    float by[BOXCOUNT];
    float bxs[BOXCOUNT];
    float bys[BOXCOUNT];
    uint8_t initd = 0;
    void draw(){
        clrgraph(1);
        if(initd == 0){
            for(uint_fast16_t i = 0; i < BOXCOUNT; i += 1){
                bx[i] = (float)(rand() % 360);
                by[i] = (float)(rand() % 240);
                bxs[i] = (float)(rand() % 41 - 20) / 10;
                bys[i] = (float)(rand() % 41 - 20) / 10;
            }
            initd = 1;
        }
        for(uint_fast16_t i = 0; i < BOXCOUNT; i += 1){
            bys[i] += 0.1;
            bx[i] += bxs[i];
            by[i] += bys[i];
            if(bx[i] < 0.0 && bxs[i] < 0.0){
                bxs[i] = -bxs[i];
            }
            if(bx[i] > 360.0 && bxs[i] > 0.0){
                bxs[i] = -bxs[i];
            }
            if(by[i] > 240.0 && bys[i] > 0.0){
                bys[i] = -bys[i];
            }
        }
        for(uint_fast16_t i = 0; i < BOXCOUNT; i += 1){
            palcolor(i % 64);
            boxf(bx[i], by[i], bx[i] + 24, by[i] + 24);
        }
        char s[8];
        snprintf(s, 7, "%u", ::frame);
        tvvt::pos(2, 0);
        palcolor(COLOR_BLACK);
        tvvt::puts(s);
    }
}