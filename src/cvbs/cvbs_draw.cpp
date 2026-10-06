#include "cvbs_core.hpp"
#include "cvbs_draw.hpp"

// ----------------------------------------------------------------

#include <cstdint>

#include "hardware/gpio.h"

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/dma.h"
#include "cvbs_output.pio.h"
#include "pico/stdlib.h"
#include <cmath>
#include <utility>
#include "pico/time.h"
#include "pico/multicore.h"
#include "hardware/structs/bus_ctrl.h"
#include "pico/mutex.h"



void lcscolor(uint8_t chroma, uint8_t luma, uint8_t saturation){
    if(saturation == 0){
        current_color = (0xC << 2) + (luma & 3);
    }else{
        current_color = ((chroma % 12) << 2) + (luma & 3);
    }
}


void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    bool steep = std::abs(x1 - x2) < std::abs(y1 - y2);
    if(steep){
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    
    if (x1 > x2){
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    int16_t y = y1;
    int ierror = 0;
    for(int_fast16_t x = x1; x <= x2; x += 1){
        if(steep){
            pset(y, x);
        }else{
            pset(x, y);
        }
        
        ierror += 2 * std::abs(y2 - y1);
        if (ierror > x2 - x1){
            y += y2 > y1 ? 1 : -1;
            ierror -= 2 * (x2 - x1);
        }
        
    }
}

void boxf(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    const int_fast16_t xr1 = (x1 > 0              ) ? x1 : 0;
    const int_fast16_t xr2 = (x2 < _display_size_x) ? x2 : _display_size_x;
    const int_fast16_t yr1 = (y1 > 0              ) ? y1 : 0;
    const int_fast16_t yr2 = (y2 < _display_size_y) ? y2 : _display_size_y;
    for(int_fast16_t yc = yr1; yc < yr1 + (yr2 - yr1 + 1); yc += 1){
        __fast_hline(yc, xr1, xr2);
    }
}

void box(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    line(x1, y1, x1, y2);
    line(x2, y1, x2, y2);
    line(x1, y1, x2, y1);
    line(x1, y2, x2, y2);
}


// カスの実装なので閉空間じゃないと(おそらくスタックオーバーフローで)クラッシュするし、遅い
void fill(int16_t x, int16_t y){
    if(x < 0 || x >= _display_size_x + drawing_x_offset){
        return;
    }
    if(y < 0 || y >= _display_size_y){
        return;
    }
    if(__pget(x, y) == current_color){
        return;
    }
    const uint8_t target_color = __pget(x, y);

    int_fast16_t lx = x;
    int_fast16_t rx = x;
    while(1){
        if(lx == 0){
            break;
        }
        if(__pget(lx, y) != target_color){
            lx += 1;
            break;
        }
        lx -= 1;
    }
    while(1){
        if(rx == _display_size_x + drawing_x_offset - 1){
            break;
        }
        if(__pget(rx, y) != target_color){
            rx -= 1;
            break;
        }
        rx += 1;
    }
    __fast_hline(y, lx, rx);
    
    if(y >= 1){
        for(int_fast16_t xcnt = lx; xcnt < rx; xcnt += 1){
            if(__pget(xcnt, y - 1) == target_color){
                fill(xcnt, y - 1);
            }
        }
    }
    if(y <= _display_size_y - 1){
        for(int_fast16_t xcnt = lx; xcnt < rx; xcnt += 1){
            if(__pget(xcnt, y + 1) == target_color){
                fill(xcnt, y + 1);
            }
        }
    }
}


void clrgraph(uint8_t clr_mode){
    const uint8_t c = __clrgraph_pattern(clr_mode);
    
    for(int_fast32_t i = 0; i < FRAMEBUF_MEM_SIZE; i += 1){
        framebuf[flip_offset + i] = c;
    }
}

void triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
    line(x1, y1, x2, y2);
    line(x2, y2, x3, y3);
    line(x3, y3, x1, y1);
}

void trianglef(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
    
    // bubble sort
    if(y2 > y3){
        std::swap(x2, x3);
        std::swap(y2, y3);
    }
    
    if(y1 > y2){
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    
    if(y2 > y3){
        std::swap(x2, x3);
        std::swap(y2, y3);
    }
    
    for(int_fast16_t ycnt = y1; ycnt < y3; ycnt += 1){
        int16_t xleft;
        if (ycnt < y2){
            xleft = x1 + (((x2 - x1) * 16) * ((ycnt - y1) * 16) / ((y2 - y1) * 16) + 8) / 16;
        }else{
            xleft = x2 + (((x3 - x2) * 16) * ((ycnt - y2) * 16) / ((y3 - y2) * 16) + 8) / 16;
        }
        int xright = x1 + (((x3 - x1) * 16) * ((ycnt - y1) * 16) / ((y3 - y1) * 16) + 8) / 16;
        
        line(xleft, ycnt, xright, ycnt);
    }
    
}

void pos(int16_t x, int16_t y){
    ginfo_cx = x;
    ginfo_cy = y;
}

void gcopy(uint8_t window_id, int16_t x1, int16_t y1, int16_t xsize, int16_t ysize){
    const uint8_t cc = current_color;
    if(x1 >= _display_size_x + drawing_x_offset || y1 >= _display_size_y){
        return;
    }
    for(int_fast16_t ycnt = 0; ycnt < ysize; ycnt += 1){
        for(int_fast16_t xcnt = 0; xcnt < xsize; xcnt += 1){
            pget(x1 + xcnt, y1 + ycnt);
            pset(ginfo_cx + xcnt, ginfo_cy + ycnt);
        }
    }
    current_color = cc;
}

void circle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fill_mode){
    // そうだ! 全部「2 倍」で扱おう
    const int_fast16_t xbase = (x1 + x2);
    const int_fast16_t ybase = (y1 + y2);
    const int_fast16_t a = (int_fast16_t)abs(x2 - x1);
    const int_fast16_t b = (int_fast16_t)abs(y2 - y1);
    const int_fast16_t asq = a * a;
    const int_fast16_t bsq = b * b;
    
    int_fast16_t oldx = 0;
    
    if(fill_mode != 0){
        // 塗りつぶし
        for(int_fast16_t ycnt = - b; ycnt <= 0; ycnt += 1){
            if((ycnt % 2) != 0){
                continue;
            }
            // 2 の倍数ってことはスクリーン座標的に整数
            // 注意: xc は 2 倍になってない
            const int xc = (asq - ((int)ycnt * ycnt * asq / bsq));
            // たぶんテーブルにするより double の計算したほうが速い
            const int_fast16_t newx = (int)sqrt(xc);
            const int_fast16_t newy = (int)ycnt;
            
            __fast_hline((ybase - newy) / 2, (xbase - newx) / 2, (xbase + newx) / 2);
            __fast_hline((ybase + newy) / 2, (xbase - newx) / 2, (xbase + newx) / 2);
            oldx = newx;
        }
        
    }else /*if(fill_mode == 0)*/{
        // 輪郭
        for(int_fast16_t ycnt = - b; ycnt <= 0; ycnt += 1){
            if((ycnt % 2) != 0){
                continue;
            }
            // 2 の倍数ってことはスクリーン座標的に整数
            // 注意: xc は 2 倍になってない
            const int xc = (asq - ((int)ycnt * ycnt * asq / bsq));
            // たぶんテーブルにするより double の計算したほうが速い
            const int_fast16_t newx = (int)sqrt(xc);
            const int_fast16_t newy = (int)ycnt;
            
            // fast_hline より line のほうが速く、意味不明
            line((xbase - newx) / 2, (ybase - newy) / 2, (xbase - oldx) / 2, (ybase - newy) / 2);
            line((xbase + newx) / 2, (ybase - newy) / 2, (xbase + oldx) / 2, (ybase - newy) / 2);
            line((xbase - newx) / 2, (ybase + newy) / 2, (xbase - oldx) / 2, (ybase + newy) / 2);
            line((xbase + newx) / 2, (ybase + newy) / 2, (xbase + oldx) / 2, (ybase + newy) / 2);
            oldx = newx;
        }
    }
}

