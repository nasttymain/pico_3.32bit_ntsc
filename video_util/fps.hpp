#ifndef __NASTTY_RCA_FPS__
#define __NASTTY_RCA_FPS__

#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "pico/stdio.h"

uint tstmp = 0;

namespace fps{
    void draw_fps();
    
    void draw_fps(){
        static uint frames = 0;
        frames += 1;
        static uint fps10 = 0;
        if(frames % 20 == 0){
            uint newt = time_us_32();
            if(newt != tstmp){
                fps10 = (uint)((float)(1000 * 20 * 10) / (float)((newt - tstmp) / 1000));
            }else{
                fps10 = 99999;
            }
            tstmp = newt;
        }
        
        char s[12];
        snprintf(s, 11, "%4u.%.1u FPS", fps10 / 10, fps10 % 10);
        
        tvvt::pos(34, 0);
        palcolor(COLOR_BLACK);
        tvvt::puts(s);
    }
}

#endif