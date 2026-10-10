//#define VIDEO_TEST_PTN_COLOR

#include <stdio.h>
#include <cvbs.hpp>
#include <cvbs_extras/cvbs_dimz.hpp>
#include <cvbs_extras/cvbs_sprite.hpp>
#include <cvbs_extras/picopico_sound/picopico.hpp>
#include "pico/time.h"

#include "video_util/fps.hpp"


#include "example_or_test/color_grayscale_test.hpp"

uint8_t mode = 0;

constexpr const uint8_t test_ei[384] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 0, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 2, 1, 1, 0, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2, 2, 1, 0, 0, 1, 2, 2, 2, 3, 3, 2, 2, 3, 3, 2, 2, 2, 1, 0, 0, 0, 1, 2, 3, 2, 2, 2, 2, 2, 2, 3, 2, 1, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};

cvbssprite::sprite sprite_ei;

void draw_by_core1();

void frame_pointer();
void frame_f_counter();
void frame_demo1();
void frame_demo2();

int main() {
    
    init_video_on_core1();
    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
    set_flip_mode(0);

    
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

uint f = 0;
float mx = DISP_RES_X / 4;
float my = DISP_RES_Y / 2;

void draw_by_core1(){
    while(1){
        switch (f / 300 % 4){
            case 0:
                if(f % 300 == 0){
                    set_flip_mode(0);
                    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
                    clrgraph(1);
                    frame_demo1();
                }
                break;
            case 1:
                if(f % 300 == 0){
                    set_flip_mode(0);
                    setDisplayMode(SCREEN_GRAYSCALE);
                    clrgraph(1);
                    frame_demo1();
                }
                break;
            case 2:
                if(f % 300 == 0){
                    set_flip_mode(0);
                    setDisplayMode(SCREEN_FULLWIDTH_COLOR);
                    clrgraph(1);
                    frame_demo2();
                }
                break;
            case 3:
                break;
            default:
                break;
        }
        
        font("", 8);
        palcolor(COLOR_BLACK);
        frame_f_counter();
        frame_pointer();
        fps::draw_fps();
        
        wait_for_vsync();
        do_flip();
        f += 1;
    }
}


void frame_pointer(){
    mx += sinf((float)(f % 720) / 360 * M_PI);
    my += cosf((float)(f % 720) / 360 * M_PI);
    video_pointer_set((int16_t)mx, (int16_t)my);
}

void frame_f_counter(){
    palcolor(COLOR_WHITE);
    boxf(0, 0, DISP_RES_X, 24);
    palcolor(COLOR_BLACK);
    box (0, 0, DISP_RES_X, 24);
    pos(16, 16);
    char s[40];
    snprintf(s, sizeof(s), "frame %u", f);
    mes(s);
}

void frame_demo1(){
    font("", 8);
    c_g_test::draw();
    sprite_ei.draw();
    palcolor(COLOR_BLACK);
    pos(16, 32);
    font("", 8);
    mes("Hello, World! こんにちは世界!!コンニチハ!!\nEI DAYO!");
    palcolor(COLOR_LIGHTGRAY);
    box(16, 32, 16 + ginfo_mesx, 32 + ginfo_mesy);
    char s[40];
    snprintf(s, sizeof(s), "ginfo_mesx: %d ginfo_mesy: %d\n", ginfo_mesx, ginfo_mesy);
    palcolor(COLOR_BLACK);
    mes(s);    
}


void frame_demo2(){
    palcolor(COLOR_BLACK);
    pos(0, 48);
    font("", 8);
    mes("FONTS TEST.");
    
    pos(0, 64);
    for(uint fs = 8; fs < 40; fs += 8){
        font("", fs);
        mes("Hello, World! こんにちは世界!!コンニチハ!!");
    }

    font("", 8);
    mes("");
    
    const uint _y = ginfo_cy;
    
    palcolor(COLOR_RED);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_ORANGE);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_YELLOW);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_GREEN);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_SKYBLUE);
    mes("Hello, World! こんにちは世界!");
    
    pos(DISP_RES_X / 2, _y);
    palcolor(COLOR_BLUE);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_PURPLE);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_BLACK);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_DARKGRAY);
    mes("Hello, World! こんにちは世界!");
    palcolor(COLOR_LIGHTGRAY);
    mes("Hello, World! こんにちは世界!");
}