//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "swimming_triangle.hpp"

void proc_cin();

int main() {
    stdio_init_all();
    /*
    // core0 で映像出力を駆動する場合は以下を呼ぶ
    init_framedata();
    init_dma();
    */
   
    // core1 で映像出力を駆動する場合は以下を呼ぶ。250ms くらい待たされる
    init_video_on_core1();
    
    palcolor(1);    
    
    clrgraph(1);
    while(1){
        //proc_cin();
        clrgraph(1);
        swimming_triangle::frame();
        wait_for_vsync();
    }
}


uint8_t n = 0;
void proc_cin(){
    while(1){
        int c = getchar_timeout_us(0);
        if(c == PICO_ERROR_TIMEOUT){
            break;
        }
        palcolor(n);
        n += 1;
        c_putc((char)c);
        if(c == 13){
            c_putc(10);
        }
    }
}