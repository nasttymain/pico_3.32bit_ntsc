//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "swimming_triangle.hpp"
#include "color_palette.hpp"
#include "bouncing_squares.hpp"

#include "submit/lineart/lineart.cpp"
#include "submit/lineart/multilineart.hpp"
#include "submit/dejong/dejong2.hpp"
#include "submit/rose/rose.hpp"
#include "font.hpp"
#include "title_call.hpp"
#include "video_util/fps.hpp"

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
        
        bouncing_squares::draw();
        fps::draw_fps();
        
        //wait_for_vsync();
        do_flip();
        
        if(f % 180 == 0){
            if(f / 180 % 2 == 1){
                setDisplayMode(SCREEN_GRAYSCALE);
            }else{
                setDisplayMode(SCREEN_FULLWIDTH_COLOR);
            }
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