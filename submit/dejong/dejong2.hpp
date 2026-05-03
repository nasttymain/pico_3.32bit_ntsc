#include <math.h>
#include "pico/rand.h"
#include "../../rca.hpp"



extern int pattern_variation;

namespace dejong_m{

    static uint16_t f = 0;


    static float da = 3.20;
    static float db = 0.91;
    static float dc = 1.23;
    static float dd = 2.832;
    static float xp = 0.0;
    static float yp = 0.0;
    static float x_new;
    static float y_new;
    static uint16_t shikisou = 0;

    static int dejong_current_var = 0;

    static uint32_t rnd(uint32_t m){
        return rand() % (m + 1);
    }

    void dejong_init(){
        f = 0;
        dejong_current_var = pattern_variation;
        db = 0.91;
        dc = 1.23;
        dd = 2.832;
    }

    void dejong_frame(){
        if(dejong_current_var != pattern_variation){
            dejong_current_var = pattern_variation;
            db = (float)(rnd(101) - 50) / 20;
            dc = (float)(rnd(101) - 50) / 20;
            dd = (float)(rnd(101) - 50) / 20;
        }
        da = 3.25 + sinf(float(f) / 100 ) * 0.8;
        
        //shikisou = (shikisou + 1) % 360;
        shikisou = (shikisou + 4) % 360;
        
        //color(hsv2rgb64(shikisou, 255, 255));
        palcolor(0x02 + ((0x03 * pattern_variation % 12) << 2));
        for(int_fast16_t i = 0; i < 3072; i += 1){
            x_new = sinf(da * yp) - cosf(db * xp);
            y_new = sinf(dc * xp) - cosf(dd * yp);
            xp = x_new;
            yp = y_new;
            
            pset( (84 + xp * 24) * 2 + 0, (54 + yp * 24) * 2 + 0 );
            pset( (84 + xp * 24) * 2 + 0, (54 + yp * 24) * 2 + 1 );
            pset( (84 + xp * 24) * 2 + 1, (54 + yp * 24) * 2 + 0 );
            pset( (84 + xp * 24) * 2 + 1, (54 + yp * 24) * 2 + 1 );
        }
        f += 1;
    }
}