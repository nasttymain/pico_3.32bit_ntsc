//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include <stdint.h>

#include "video_util/fps.hpp"


int pattern_variation = 0;

uint8_t mode = 0;

void test_boxf(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    const int_fast16_t xr1 = (x1 > 0              ) ? x1 : 0;
    const int_fast16_t xr2 = (x2 < _display_size_x) ? x2 : _display_size_x;
    const int_fast16_t yr1 = (y1 > 0              ) ? y1 : 0;
    const int_fast16_t yr2 = (y2 < _display_size_y) ? y2 : _display_size_y;
    for(int_fast16_t yc = yr1; yc < yr1 + (yr2 - yr1 + 1); yc += 1){
        for(int_fast16_t xc = xr1; xc < xr1 + (xr2 - xr1 + 1); xc += 1){
            pset(xc, yc);
        }
    }
}

/*
boxf を __fast_hline を使う実装に書き換えた当初、アルゴリズムに誤りがあり矩形のサイズが1ピクセル間違って描画されることがあった。
そのときに使用した、boxfの描画の正しさを検証するためのテストコード。現在は修正済なので上下対象な図形が描画される
*/


int main() {
    stdio_init_all();
    
    init_video_on_core1();
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    set_flip_mode(1);
    uint f = 0;
    while(1){
        f += 1;
        
        clrgraph(1);
        palcolor(COLOR_BLACK);
        boxf(16, 16, 32, 32);
        boxf(16, 32, 33, 48);
        boxf(16, 48, 34, 64);
        boxf(16, 64, 35, 80);

        test_boxf(16, 80, 35, 96);
        test_boxf(16, 96, 34, 112);
        test_boxf(16, 112, 33, 128);
        test_boxf(16, 128, 32, 144);

        boxf(63, 16, 79, 32);
        boxf(63, 32, 80, 48);
        boxf(63, 48, 81, 64);
        boxf(63, 64, 82, 80);

        test_boxf(63, 80, 82, 96);
        test_boxf(63, 96, 81, 112);
        test_boxf(63, 112, 80, 128);
        test_boxf(63, 128, 79, 144);
        
        fps::draw_fps();
        
        do_flip();
    }
}
