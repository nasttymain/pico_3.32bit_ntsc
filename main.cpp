//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include "cvbs.hpp"
#include "cvbs_write.hpp"
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
    sprite_ei.xpos = 48;
    sprite_ei.ypos = 80;
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
    float mx = DISP_RES_X / 2;
    float my = DISP_RES_Y / 2;
    while(1){
        f += 1;
        
        clrgraph(1);
        palcolor(COLOR_BLACK);
        
        mx += sinf((float)(f % 720) / 360 * M_PI);
        my += cosf((float)(f % 720) / 360 * M_PI);
        video_pointer_set((int16_t)mx, (int16_t)my);
        
        c_g_test::draw();
        
        sprite_ei.draw();
        
        palcolor(COLOR_BLACK);
        
        //tvvt::pos(2, 2);
        //tvvt::puts("Hello, World! こんにちは世界!!コンニチハ!!\n");
        palcolor(COLOR_BLACK);
        pos(16, 16);
        mes("Hello, World! こんにちは世界!!コンニチハ!!\nEI DAYO!");
        palcolor(COLOR_LIGHTGRAY);
        box(16, 16, 16 + ginfo_mesx, 16 + ginfo_mesy);
        char s[40];
        snprintf(s, sizeof(s), "ginfo_mesx: %d ginfo_mesy: %d", ginfo_mesx, ginfo_mesy);
        palcolor(COLOR_BLACK);
        mes(s);
        
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