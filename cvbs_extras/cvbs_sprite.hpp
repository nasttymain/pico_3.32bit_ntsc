#include "cvbs.hpp"

#ifndef COLOR_TRANSPARENT
#define COLOR_TRANSPARENT 63
#endif

#ifndef __NASTTY_CVBS_SPRITE__
#define __NASTTY_CVBS_SPRITE__

namespace cvbssprite{
    #include <cstdint>
    constexpr const uint8_t default_palette[16]  = {
        63, 29, 30, 34, 46, 6, 8, 22, 48, 49, 50, 51, 0, 1, 2, 3
    };
    class sprite{
        public:
            sprite();
            const uint8_t* pattern;
            // palette: 63 は透明
            uint8_t palette[16];
            // pos は真ん中基準
            int16_t xpos;
            int16_t ypos;
            int16_t xsize;
            int16_t ysize;
            // 1: 正転 -1: 反転
            int8_t xdir;
            int8_t ydir;
            void draw();
    };
    sprite::sprite(){
        pattern = nullptr;
        for(uint8_t i = 0; i < 16; i += 1){
            palette[i] = default_palette[i];
        }
        xpos = 0;
        ypos = 0;
        xsize = 0;
        ysize = 0;
        xdir = 1;
        ydir = 1;
    }
    
    void sprite::draw(){
        if(xsize <= 0 || ysize <= 0){
            return;
        }
        if(xdir == 0 || ydir == 0){
            return;
        }
        
        auto _c = current_color;
        
        const int16_t x1 = xpos - xsize / 2;
        const int16_t x2 = x1 + xsize - 1;
        const int16_t y1 = ypos - ysize / 2;
        const int16_t y2 = y1 + ysize - 1;
        const int16_t xstart = (xdir == 1) ? x1 : x2;
        const int16_t ystart = (ydir == 1) ? y1 : y2;
        
        const uint8_t* ptn = pattern;
        
        int16_t y = ystart;
        for(int16_t ycnt = 0; ycnt < ysize; ycnt += 1){
            int16_t x = xstart;
            for(int16_t xcnt = 0; xcnt < xsize; xcnt += 1){
                const auto c = palette[*ptn];
                if(c != 63){
                    palcolor(c);
                    pset(x, y);
                }
                //
                x += xdir;
                ptn += 1;
            }
            //
            y += ydir;
        }
        palcolor(_c);
    }
}

#endif

