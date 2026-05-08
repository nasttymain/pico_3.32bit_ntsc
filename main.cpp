//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "rcabezier.hpp"
#include "pico/time.h"

#include "video_util/fps.hpp"

#include "example_or_test/bezierテスト.hpp"

void proc_cin();

int pattern_variation = 0;

uint8_t mode = 0;



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
        
        test_bezier::frame();
        
        fps::draw_fps();
        
        //wait_for_vsync();
        do_flip();
        
        if(f % 180 == 0){
            pattern_variation += 1;
        }
        
    }
}

uint8_t n = 0;
void proc_cin(){
    while(1){
        int c = getchar_timeout_us(0);
        if(c == PICO_ERROR_TIMEOUT){
            break;
        }
        if(color_mode == SCREEN_PALETTE){
            palcolor(n);
            n += 1;
        }else{
            palcolor(0);
        }
        tvvt::putc((char)c);
        if(c == 13){
            tvvt::putc(10);
        }
    }
}