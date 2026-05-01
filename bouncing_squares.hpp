#include "rca.hpp"
#include "rcavt.hpp"
#include <stdio.h>

namespace bouncing_squares{
    constexpr const uint16_t BOXCOUNT = 512;
    float bx[BOXCOUNT];
    float by[BOXCOUNT];
    float bxs[BOXCOUNT];
    float bys[BOXCOUNT];
    uint32_t fc = 0;
    uint8_t initd = 0;
    void draw(){
        clrgraph(1);
        if(initd == 0){
            for(uint_fast16_t i = 0; i < BOXCOUNT; i += 1){
                bx[i] = (float)(rand() % 3600) / 10;
                by[i] = (float)(rand() % 2400) / 10;
                bxs[i] = (float)(rand() % 401 - 200) / 100;
                bys[i] = (float)(rand() % 401 - 200) / 100;
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
        snprintf(s, 7, "%u", fc);
        fc += 1;
        tvvt::pos(2, 0);
        palcolor(COLOR_BLACK);
        tvvt::puts(s);
    }
}