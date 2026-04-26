//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "swimming_triangle.hpp"

void proc_cin();

uint8_t mode = 0;

int main() {
    stdio_init_all();
    
    init_video_on_core1();
    
    palcolor(1);    
    
    clrgraph(1);
    while(1){
        //proc_cin();
        if(mode == 0){
            clrgraph(1);
            swimming_triangle::frame();
            boxf(20, 20, 50, 30);
            palcolor(0);
            tvvt::pos(0, 0);
            tvvt::puts("Hello, World!");
        }
        if(mode == 1){
            
        }
        wait_for_vsync();
        // BEGIN MODE_CHANGE
        if(frame % 300 == 0){
            mode = (mode + 1) % 2;
            if(mode == 0){
                setDisplayMode(SCREEN_PALETTE);
            }
            if(mode == 1){
                setDisplayMode(SCREEN_GRAYSCALE);
                clrgraph(1);
                palcolor(0);
                tvvt::pos(0, 0);
                tvvt::puts("LOREM IPSUM DOLOR SIT AMET, CONSECTETUR ADIPISCING ELIT, SED DO EIUSMOD TEMPOR INCIDIDUNT UT LABORE ET DOLORE MAGNA ALIQUA. UT ENIM AD MINIM VENIAM, QUIS NOSTRUD EXERCITATION ULLAMCO LABORIS NISI UT ALIQUIP EX EA COMMODO CONSEQUAT. DUIS AUTE IRURE DOLOR IN REPREHENDERIT IN VOLUPTATE VELIT ESSE CILLUM DOLORE EU FUGIAT NULLA PARIATUR. EXCEPTEUR SINT OCCAECAT CUPIDATAT NON PROIDENT, SUNT IN CULPA QUI OFFICIA DESERUNT MOLLIT ANIM ID EST LABORUM.");
            }
        }
        // END MODE_CHANGE
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