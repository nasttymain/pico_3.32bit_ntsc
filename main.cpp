//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "swimming_triangle.hpp"
#include "color_palette.hpp"
#include "bouncing_squares.hpp"

void proc_cin();

uint8_t mode = 0;

constexpr const char lorem[] = "LOREM IPSUM DOLOR SIT AMET, CONSECTETUR ADIPISCING ELIT, SED DO EIUSMOD TEMPOR INCIDIDUNT UT LABORE ET DOLORE MAGNA ALIQUA. UT ENIM AD MINIM VENIAM, QUIS NOSTRUD EXERCITATION ULLAMCO LABORIS NISI UT ALIQUIP EX EA COMMODO CONSEQUAT. DUIS AUTE IRURE DOLOR IN REPREHENDERIT IN VOLUPTATE VELIT ESSE CILLUM DOLORE EU FUGIAT NULLA PARIATUR. EXCEPTEUR SINT OCCAECAT CUPIDATAT NON PROIDENT, SUNT IN CULPA QUI OFFICIA DESERUNT MOLLIT ANIM ID EST LABORUM.";

int main() {
    
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 1);

    stdio_init_all();
    
    init_video_on_core1();
    
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    
    set_flip_mode(1);
    
    clrgraph(1);
    
    uint f = 0;
    
    while(1){
        f += 1;
        wait_for_vsync();
        bouncing_squares::draw();
        do_flip();
        gpio_put(0, 0);
    }
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