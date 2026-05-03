#include "pico/rand.h"
#include <stdint.h>
#include "../../rca.hpp"

extern int pattern_variation;

namespace multilineart{
    static constexpr const uint8_t MAX_INSTANCES = 8;
    static constexpr const uint8_t MAX_LINES = 127;
    static uint8_t lines;
    static uint8_t instances;
    static int16_t x[MAX_INSTANCES][MAX_LINES * 2];
    static int16_t y[MAX_INSTANCES][MAX_LINES * 2];
    static int16_t xs[MAX_INSTANCES][2];
    static int16_t ys[MAX_INSTANCES][2];
    static uint32_t tick = 0;

    static uint32_t rnd(uint32_t m){
        return rand() % (m);
    }

    void multilineart_init(){
        lines = 64;
        instances = 4;
        for(uint_fast8_t i = 0; i < instances; i += 1){
            x [i][0] = rnd(VIEWPORT_RES_X);
            y [i][0] = rnd(VIEWPORT_RES_Y);
            x [i][1] = rnd(VIEWPORT_RES_X);
            y [i][1] = rnd(VIEWPORT_RES_Y);
            xs[i][0] = rnd(2) * 8 - 4;
            ys[i][0] = rnd(2) * 8 - 4;
            xs[i][1] = rnd(2) * 8 - 4;
            ys[i][1] = rnd(2) * 8 - 4;
        }
        x[0][0] = 20;
        y[0][0] = 90;
        x[0][1] = 160;
        y[0][1] = 60;
        xs[0][0] = -4;
        ys[0][0] = -4;
        xs[0][1] = -4;
        ys[0][1] = -4;
        for(uint_fast8_t icnt = 0; icnt < instances; icnt += 1){
            for(uint_fast8_t i = 1; i < MAX_LINES; i += 1){
                x[icnt][i * 2 + 0] = 0;
                y[icnt][i * 2 + 0] = 0;
                x[icnt][i * 2 + 1] = 0;
                y[icnt][i * 2 + 1] = 0;
            }
        }
    }
    void multilineart_frame(){
        tick += 1;
        for(uint_fast8_t icnt = 0; icnt < instances; icnt += 1){
            for(uint_fast8_t i = 0; i < 2; i += 1){
                x[icnt][i] += xs[icnt][i];
                y[icnt][i] += ys[icnt][i];
                if(x[icnt][i] <= 0 && xs[icnt][i] <= 0){
                    xs[icnt][i] = -xs[icnt][i];
                }
                if(y[icnt][i] <= 0 && ys[icnt][i] <= 0){
                    ys[icnt][i] = -ys[icnt][i];
                }
                if(x[icnt][i] >= VIEWPORT_RES_X && xs[icnt][i] >= 0){
                    xs[icnt][i] = -xs[icnt][i];
                }
                if(y[icnt][i] >= VIEWPORT_RES_Y && ys[icnt][i] >= 0){
                    ys[icnt][i] = -ys[icnt][i];
                }
            }

            for(uint_fast8_t cnt = 0; cnt < lines - 1; cnt += 1){
                uint_fast8_t lcnt = lines - 1 - cnt;
                x[icnt][lcnt * 2 + 0] = x[icnt][lcnt * 2 - 2];
                y[icnt][lcnt * 2 + 0] = y[icnt][lcnt * 2 - 2];
                x[icnt][lcnt * 2 + 1] = x[icnt][lcnt * 2 - 1];
                y[icnt][lcnt * 2 + 1] = y[icnt][lcnt * 2 - 1];
            }
            for(uint_fast8_t cnt = 0; cnt < lines; cnt += 1){
                uint_fast8_t lcnt = lines - 1 - cnt;
                uint8_t n = 3 - lcnt * 3 / lines;
                /*if(pattern_variation == 0){
                    if      (icnt % 4 == 0){
                        color(n * 16 + n * 4 + n);
                    }else if(icnt % 4 == 1){
                        color(n * 16            );
                    }else if(icnt % 4 == 2){
                        color(         n * 4    );
                    }else if(icnt % 4 == 3){
                        color(                 n);
                    }
                }else if(pattern_variation >= 1){
                    color(hsv2rgb64((icnt * 90 + tick / 30) % 360, 255, 255));
                }*/
                palcolor(0x02 + ((0x02 * icnt + ((tick / 300 + pattern_variation)) % 16) << 2));
                line(x[icnt][lcnt * 2 + 0], y[icnt][lcnt * 2 + 0], x[icnt][lcnt * 2 + 1], y[icnt][lcnt * 2 + 1]);
            }
        }
    }
}