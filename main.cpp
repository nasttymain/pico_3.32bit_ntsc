//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "rca.hpp"
#include "rcavt.hpp"
#include "rcadimz.hpp"
#include "rcabezier.hpp"
#include "picopico_sound/picopico.hpp"
#include "pico/time.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"

#include "video_util/fps.hpp"

#include "example_or_test/test_model.hpp"

void proc_cin();

int pattern_variation = 0;

uint8_t mode = 0;

#include "donotstage/song.mid.cpp"

int main() {
    
    stdio_init_all();
    
    init_video_on_core1();
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    set_flip_mode(1);
    uint f = 0;
    
    picopico_all_init();
    FILE* fsmf = fmemopen((void*)smf, sizeof(smf), "rb");
    picopicomidi::load_from_file(fsmf);
    
    while(1){
        f += 1;
        
        
        clrgraph(0);
        
        palcolor(COLOR_WHITE);
        
        test_model::frame();
        palcolor(COLOR_WHITE);
        fps::draw_fps();
        
        palcolor(COLOR_WHITE);
        char s[6];
        for(uint i = 0; i < picopico::CH_COUNT; i += 1){
            snprintf(s, sizeof(s), "%4u", picopico::ss_toneon[i]);
            tvvt::pos(4 + i * 5, 5);
            tvvt::puts(s);
            snprintf(s, sizeof(s), "%4u", picopico::ss_phase_step[i] / 100);
            tvvt::pos(4 + i * 5, 6);
            tvvt::puts(s);
        }

        wait_for_vsync();
        do_flip();
        
        if(frame == 120){
            __dump_smsdat(&picopicomidi::smf_data);
            picopicomidi::is_playing = 1;
        }
        
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
