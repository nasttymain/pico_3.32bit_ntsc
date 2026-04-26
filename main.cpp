//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "swimming_triangle.hpp"

void proc_cin();

int main() {
    stdio_init_all();
    
    init_video_on_core1();
    
    palcolor(1);    
    
    clrgraph(1);
    while(1){
        //proc_cin();
        clrgraph(1);
        swimming_triangle::frame();
        palcolor(0);
        c_pos(0, 0);
        c_puts("Hello, World!");
        wait_for_vsync();
        if(frame % 300 == 0){
            if((frame / 300) % 2 == 1){
                setDisplayMode(SCREEN_GRAYSCALE);
            }else{
                setDisplayMode(SCREEN_PALETTE);
            }
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
        c_putc((char)c);
        if(c == 13){
            c_putc(10);
        }
    }
}