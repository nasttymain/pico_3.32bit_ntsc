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

#include "rcasprite/rcasprite.hpp"

uint8_t mode = 0;

constexpr const uint8_t test_ei[384] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 0, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 2, 1, 1, 0, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2, 2, 1, 0, 0, 1, 2, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 2, 1, 0, 0, 0, 1, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};

rcasprite::sprite sprite_ei;
int main() {
    
    stdio_init_all();
    
    init_video_on_core1();
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    set_flip_mode(1);
    uint f = 0;
    
    picopico_all_init();

    sprite_ei.pattern = test_ei;
    sprite_ei.palette[1] = COLOR_BLACK;
    sprite_ei.palette[2] = COLOR_WHITE;
    sprite_ei.palette[3] = COLOR_LIGHTGRAY;
    sprite_ei.xpos = 32;
    sprite_ei.ypos = 32;
    sprite_ei.xsize = 16;
    sprite_ei.ysize = 24;


    while(1){
        f += 1;
        
        clrgraph(1);
        palcolor(COLOR_BLACK);
        
        sprite_ei.draw();
        
        fps::draw_fps();
        
        wait_for_vsync();
        do_flip();
        
        
        
    }
}

