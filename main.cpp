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

void proc_cin();

int pattern_variation = 0;

uint8_t mode = 0;

int main() {
    
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 1);

    stdio_init_all();
    
    init_video_on_core1();
    
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    
    set_flip_mode(1);
    
    
    uint f = 0;
    
    
    dejong_m::dejong_init();
    
    while(1){
        f += 1;
        clrgraph(0);
        dejong_m::dejong_frame();
        wait_for_vsync();
        do_flip();
        
        if(f % 180 == 0){
            pattern_variation += 1;
        }
    }
    
    /*
    while(1){
        f += 1;
        //wait_for_vsync();
        bouncing_squares::draw();
        do_flip();
        gpio_put(0, 0);
    }
    */
}
    /*
    while(1){
        // BEGIN MODE_CHANGE
        if(f % 300 == 0){
            mode = (mode + 1) % 3;
            if(mode % 3 == 0){
            }
            if(mode % 3 == 1){
            }
            if(mode % 3 == 2){
                color_palette::draw();
                gpio_put(0, 0);
            }
        }
        // END MODE_CHANGE
        if(mode % 3 == 0){
            clrgraph(1);
            boxf(20, 20, 50, 30);
            palcolor(0);
            tvvt::pos(0, 0);
            tvvt::puts("Hello, World!");
            swimming_triangle::frame();
        }
        if(mode % 3 == 1){
            if(f % 60 == 1){
                clrgraph(1);
                palcolor(f / 60 % 64);
                tvvt::pos(0, 0);
                tvvt::puts(lorem);
            }            
        }
        if(mode % 3 == 2){
            
        }
        wait_for_vsync();
        f += 1;
    }
}
*/


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