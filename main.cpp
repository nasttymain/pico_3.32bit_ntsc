//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "pico/time.h"
#include "swimming_triangle.hpp"

void proc_cin();

uint8_t mode = 2;

constexpr const char lorem[] = "LOREM IPSUM DOLOR SIT AMET, CONSECTETUR ADIPISCING ELIT, SED DO EIUSMOD TEMPOR INCIDIDUNT UT LABORE ET DOLORE MAGNA ALIQUA. UT ENIM AD MINIM VENIAM, QUIS NOSTRUD EXERCITATION ULLAMCO LABORIS NISI UT ALIQUIP EX EA COMMODO CONSEQUAT. DUIS AUTE IRURE DOLOR IN REPREHENDERIT IN VOLUPTATE VELIT ESSE CILLUM DOLORE EU FUGIAT NULLA PARIATUR. EXCEPTEUR SINT OCCAECAT CUPIDATAT NON PROIDENT, SUNT IN CULPA QUI OFFICIA DESERUNT MOLLIT ANIM ID EST LABORUM.";

int main() {
    stdio_init_all();
    
    init_video_on_core1();
    
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    
    palcolor(1);    
    
    clrgraph(1);
    while(1){
        //proc_cin();
        if(mode % 3 == 0){
            clrgraph(1);
            boxf(20, 20, 50, 30);
            palcolor(0);
            tvvt::pos(0, 0);
            tvvt::puts("Hello, World!");
            swimming_triangle::frame();
        }
        if(mode % 3 == 1){
            if(frame % 60 == 1){
                clrgraph(1);
                palcolor(frame / 60 % 64);
                tvvt::pos(0, 0);
                tvvt::puts(lorem);
            }            
        }
        if(mode % 3 == 2){
            
        }
        wait_for_vsync();
        // BEGIN MODE_CHANGE
        if(frame % 300 == 0){
            //mode = (mode + 1) % 3;
            if(mode % 3 == 0){
            }
            if(mode % 3 == 1){
            }
            if(mode % 3 == 2){
                clrgraph(1);
                const int left = _display_size_x / 2 - 160;
                const int top = _display_size_y / 2 - 40;
                for(int i = 0; i < 16; i += 1){
                    char s[2];
                    snprintf(s, 2, "%.1X", i);
                    palcolor(0);
                    tvvt::put_char_graphic(s[0], left + 20 * i + 6, top - 10);
                }
                for(int i = 0; i < 4; i += 1){
                    char s[2];
                    snprintf(s, 2, "%.1X", i);
                    palcolor(0);
                    tvvt::put_char_graphic(s[0], left - 10, top + 20 * i + 6);
                }
                for(int i = 0; i < 64; i += 1){
                    const int x = (i / 4) % 16 * 20 + left;
                    const int y = (i % 4) * 20 + top;
                    palcolor(i);
                    boxf(x, y, x + 20, y + 20);
                }
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