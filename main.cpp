//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "cvbs.hpp"
#include "cvbs_vt_mb.hpp"
#include "cvbs_vt.hpp"
#include "cvbs_dimz.hpp"
#include "cvbs_bezier.hpp"
#include "picopico_sound/picopico.hpp"
#include "pico/time.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"

#include "video_util/fps.hpp"

#include "cvbssprite/cvbs_sprite.hpp"

#include "example_or_test/color_grayscale_test.hpp"

uint8_t mode = 0;

constexpr const uint8_t test_ei[384] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 0, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 2, 1, 1, 0, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2, 2, 1, 0, 0, 1, 2, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 2, 1, 0, 0, 0, 1, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};

cvbssprite::sprite sprite_ei;

void draw_by_core1();

int main() {
    
    init_video_on_core1();
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    set_flip_mode(1);

    
    //picopico_all_init();

    sprite_ei.pattern = test_ei;
    sprite_ei.palette[0] = COLOR_TRANSPARENT;
    sprite_ei.palette[1] = COLOR_BLACK;
    sprite_ei.palette[2] = COLOR_WHITE;
    sprite_ei.palette[3] = COLOR_LIGHTGRAY;
    sprite_ei.xpos = 32;
    sprite_ei.ypos = 32;
    sprite_ei.xsize = 16;
    sprite_ei.ysize = 24;

    
    stdio_init_all();

    core1_loop = &draw_by_core1;

    while(1){
        sleep_ms(1);
    }
}

void draw_by_core1(){
    uint f = 0;
    while(1){
        f += 1;
        
        clrgraph(1);
        palcolor(COLOR_BLACK);
        
        c_g_test::draw();
        
        sprite_ei.draw();
        
        palcolor(COLOR_BLACK);
        
        tvvt::puts("Hello, World! こんにちは世界!!コンニチハ!!\n");
                
        fps::draw_fps();
        
        wait_for_vsync();
        do_flip();
        
        if(::frame % 300 == 0){
            if((::frame / 300) % 2 == 0){
                setDisplayMode(SCREEN_GRAYSCALE);
            }else{
                setDisplayMode(SCREEN_FULLWIDTH_COLOR);
            }
        }
        
    }
}