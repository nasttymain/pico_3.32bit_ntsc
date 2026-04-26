/*

強制カラーキラー.cpp

本プロジェクトのテストパターン(各色のバー)を表示します。
5秒ごとにカラーバーストの送出ON/OFFを切り替えます。
これにより、色表示のON/OFFが切り替わります。
また、キャプボや変換器、HDTV はアナログ RCA 映像を高い周波数でサンプリングするので、
カラーキラーが発動した状態だと、
もともと色があった部分に、3.579545Hz (色搬送波) を由来とする縦縞が映るようになります。
副作用として、モノクロモードだとカラーモードよりも高い横解像度が得られます。
これを用いて、本プロジェクトではモノクロモードだとカラーモードの倍の横解像度で表示しています。

*/
#define VIDEO_TEST_PTN_COLOR

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
        proc_cin();
        //clrgraph(1);
        //swimming_triangle::frame();
        wait_for_vsync();
        if(frame % 300 == 0){
            if((frame / 300) % 2 == 1){
                _remove_colorburst();
            }else{
                _restore_colorburst();
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
        palcolor(n);
        n += 1;
        c_putc((char)c);
        if(c == 13){
            c_putc(10);
        }
    }
}