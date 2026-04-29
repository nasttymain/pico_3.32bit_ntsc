#include <cstdint>
void pset(int16_t x, int16_t y);
void palcolor(uint8_t palno);
void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void wait_for_vsync();
void triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);
void trianglef(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);

void init_framedata();
void _remove_colorburst();
void _restore_colorburst();
void setDisplayMode(uint16_t mode);
void init_dma();

#define SCREEN_PALETTE 2
#define SCREEN_GRAYSCALE 1024
#define SCREEN_FULLWIDTH_COLOR 2048


/*
void boxf(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void cls(uint8_t cls_mode);
*/


#ifndef __NASTTY_RCA_1BIT__
#define __NASTTY_RCA_1BIT__

#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/dma.h"
#include "main.pio.h"
#include "pico/stdlib.h"
#include <cmath>
#include <utility>
#include "pico/time.h"
#include "pico/multicore.h"

#define START_PIN 16
#define ROW_PINS 4

// PIO program is a simple pull and shift out
// simply copy & paste the following into main.pio file
// .program main
//     pull
//     out pins, 4
int dma_chan;

// タイミング一覧
constexpr const uint16_t LEN_FRONT_PORCH            = 21;
constexpr const uint16_t LEN_SYNC_PULSE             = 67;
constexpr const uint16_t LEN_BACK_PORCH             = 68;
constexpr const uint16_t LEN_ACTIVE_VIDEO           = 756;
constexpr const uint16_t LEN_BEFORE_ACTIVE_VIDEO    = LEN_FRONT_PORCH + LEN_SYNC_PULSE + LEN_BACK_PORCH; //156
constexpr const uint16_t LEN_LINE_LENGTH            = LEN_FRONT_PORCH + LEN_SYNC_PULSE + LEN_BACK_PORCH + LEN_ACTIVE_VIDEO; // 912

// 画面モードに関する変数
uint16_t color_mode = SCREEN_PALETTE;
int8_t drawing_x_offset = 0;

volatile uint16_t lineno = 0;
volatile uint32_t frame = 0;
// もともとのコードでは 2bit だったが、今回から 4bit (同期信号のほかに 3bit 分の信号がある。モノクロなら 8 階調分)
// 色数は (有彩色 12 + 無彩色 1) x 4 = 52 色。なんとやり方がファミコンといっしょでびっくり!
// でも、DAC の階調が少ない関係で輝度 100% ではカラーバーストの波形を正しく現せず、実際はもう少し少ない。
constexpr const uint16_t LINEBUF_LEN = LEN_LINE_LENGTH / 2;
uint8_t linebuf_a[LINEBUF_LEN] __attribute__((aligned(4)));
uint8_t linebuf_b[LINEBUF_LEN] __attribute__((aligned(4)));
uint8_t* ptr_linebuf[2] = {linebuf_a, linebuf_b};
uint8_t linebuf_vblank[LINEBUF_LEN] __attribute__((aligned(4)));
uint8_t linebuf_vsync[LINEBUF_LEN] __attribute__((aligned(4)));


// X 方向に 189 pixel。縦は 224 ラインを使う
constexpr const uint16_t DISP_RES_X = (LEN_ACTIVE_VIDEO) / 4;
constexpr const uint16_t DISP_RES_X_GRAYSCALE = LEN_ACTIVE_VIDEO / 2;
constexpr const uint16_t DISP_RES_Y = 240;

uint16_t _display_size_x = DISP_RES_X - drawing_x_offset;
uint16_t _display_size_y = DISP_RES_Y;

uint8_t framebuf[256 * DISP_RES_Y] __attribute__((aligned(4)));



PIO pio = pio0;
uint sm;
dma_channel_config dc;
uint8_t current_color = 1;

// いや 2 周してる!!! 式を書くのをめんどくさがった末路
const int8_t sin4[8] = {0, 2, 0, -2, 0, 2, 0, -2};
// いや 3 周してる!!!!!!!!! (いちおう、if を減らしたほうが性能上がるんじゃいかな～みたいな淡い期待がある)
const int8_t sin12[36] = {
     0,  1,  2,
     2,  2,  1,
     0, -1, -2,
    -2, -2, -1,
    
     0,  1,  2,
     2,  2,  1,
     0, -1, -2,
    -2, -2, -1,
    
     0,  1,  2,
     2,  2,  1,
     0, -1, -2,
    -2, -2, -1,
};


// 同期信号用に、一番端に 750Ω を置いているためダルいことなった...
// 無駄な if の発生を抑制するために、ホンマは 10 段階でいいところを上下それぞれ 3 段階ずつ足す
const uint8_t amp2out[16] = {
    0b0000,
    0b0000,
    0b0000,
    0b0000,
    0b0010,
    0b0001,
    0b0011,
    0b0101,
    0b0111,
    0b1001,
    0b1011,
    0b1101,
    0b1111,
    0b1111,
    0b1111,
    0b1111,
};
constexpr const uint8_t AMPINDEX_0IRE = 5;

uint8_t flip = 0;
void hndirq0(void){
    
    dma_hw->ints0 = 1u << dma_chan;
    dma_channel_abort(dma_chan);
    
    //   3 Before Porch 
    //   3 Vsync
    //  14 After Porch
    // 240 Video
    //   2 inactive video
    
    if(lineno <= 3){
        //   3 Before Porch
        dma_channel_set_read_addr(dma_chan, linebuf_vblank, false);
    }else if(lineno <= 6){
        //   3 Vsync
        dma_channel_set_read_addr(dma_chan, linebuf_vsync, false);
    }else if(lineno <= 20 + 8){
        //  14 After Porch + 8 no video
        dma_channel_set_read_addr(dma_chan, linebuf_vblank, false);
    }else if(lineno <= 20 + 8 + 240){
        // 240 Video
        dma_channel_set_read_addr(dma_chan, ptr_linebuf[flip], false);
    }else{
        // 10 after video
        dma_channel_set_read_addr(dma_chan, linebuf_vblank, false);
    }
    
    dma_channel_set_trans_count(dma_chan, LINEBUF_LEN / sizeof(uint8_t), false);
        
    dma_channel_start(dma_chan);
    
    flip = (flip + 1) & 1;
    if(lineno > (28 - 1) && lineno <= 20 + 8 + 240){
        // 次の flip に対して書込処理を行う
        constexpr const uint_fast16_t xindex_base = 79;
        const uint_fast16_t linenum = (lineno - 28);
        const uint_fast32_t lineoffset = linenum << 8;
        // 映像として有効な x 方向のlinebufの添字は、79～454(455は捨てる)の376バイト、188ピクセル。
        
        if(color_mode == SCREEN_PALETTE){
            // BEGIN LINE_DATA_CONSTRUCT WHEN SCREEN_PALETTE
            for(uint_fast16_t i = 0; i < 188; i += 1){
                #ifndef VIDEO_TEST_PTN_COLOR
                const uint_fast8_t pxdat = framebuf[lineoffset + i];
                #else
                // 以下、テストパターンジェネレータ
                const uint_fast8_t pxdat = (i >> 2) + (linenum >= 120 ? 48 : 0);
                // 以上、テストパターンジェネレータ
                #endif
                const int_fast8_t pxvalue = pxdat & 0b00000011;
                const int_fast8_t pxcolorphase =  (pxdat & 0b00111100) >> 2;
                const uint_fast8_t pxcolorvalue = (pxcolorphase >= 12 ? 0 : 1);
                
                const uint_fast8_t subpx[4] = {
                    amp2out[(pxvalue << 1) + sin12[(pxcolorphase) + 0] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue << 1) + sin12[(pxcolorphase) + 3] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue << 1) + sin12[(pxcolorphase) + 6] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue << 1) + sin12[(pxcolorphase) + 9] * pxcolorvalue + AMPINDEX_0IRE]
                };
                (ptr_linebuf[flip])[xindex_base + (i << 1) + 0] = (subpx[0] << 4) + (subpx[1]);
                (ptr_linebuf[flip])[xindex_base + (i << 1) + 1] = (subpx[2] << 4) + (subpx[3]);
            }
            // END LINE_DATA_CONSTRUCT WHEN SCREEN_PALETTE
        }else if(color_mode == SCREEN_GRAYSCALE){
            // BEGIN LINE_DATA_CONSTRUCT WHEN SCREEN_GRAYSCALE
            for(uint_fast16_t i = 0; i < 188; i += 1){
                #ifndef VIDEO_TEST_PTN_COLOR
                const uint_fast8_t pxdat = framebuf[lineoffset + i];
                #else
                // 以下、テストパターンジェネレータ
                const uint_fast8_t pxdat = (i >> 2) + (linenum >= 120 ? 48 : 0);
                // 以上、テストパターンジェネレータ
                #endif
                const int_fast8_t pxvalue[2] = {(int_fast8_t)((pxdat >> 6) & 0b00000011), (int_fast8_t)(pxdat & 0b00000011)};
                
                const uint_fast8_t subpx[4] = {
                    amp2out[(pxvalue[0] << 1) + AMPINDEX_0IRE],
                    amp2out[(pxvalue[0] << 1) + AMPINDEX_0IRE],
                    amp2out[(pxvalue[1] << 1) + AMPINDEX_0IRE],
                    amp2out[(pxvalue[1] << 1) + AMPINDEX_0IRE]
                };
                (ptr_linebuf[flip])[xindex_base + (i << 1) + 0] = (subpx[0] << 4) + (subpx[1]);
                (ptr_linebuf[flip])[xindex_base + (i << 1) + 1] = (subpx[2] << 4) + (subpx[3]);
            }
            // END LINE_DATA_CONSTRUCT WHEN SCREEN_GRAYSCALE
        }else{
            // BEGIN LINE_DATA_CONSTRUCT WHEN SCREEN_FULLWIDTH_COLOR
            for(uint_fast16_t i = 0; i < 188; i += 1){
                const uint_fast8_t pxdat = framebuf[lineoffset + i];
                const int_fast8_t pxvalue[2] = {(int_fast8_t)((pxdat >> 6) & 0b00000011), (int_fast8_t)(pxdat & 0b00000011)};
                const int_fast8_t pxcolorphase =  (pxdat & 0b00111100) >> 2;
                const uint_fast8_t pxcolorvalue = (pxcolorphase >= 12 ? 0 : 1);
                
                const uint_fast8_t subpx[4] = {
                    amp2out[(pxvalue[0] << 1) + sin12[(pxcolorphase) + 0] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue[0] << 1) + sin12[(pxcolorphase) + 3] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue[1] << 1) + sin12[(pxcolorphase) + 6] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue[1] << 1) + sin12[(pxcolorphase) + 9] * pxcolorvalue + AMPINDEX_0IRE]
                };
                (ptr_linebuf[flip])[xindex_base + (i << 1) + 0] = (subpx[0] << 4) + (subpx[1]);
                (ptr_linebuf[flip])[xindex_base + (i << 1) + 1] = (subpx[2] << 4) + (subpx[3]);
            }
            // END LINE_DATA_CONSTRUCT WHEN SCREEN_FULLWIDTH_COLOR
        }
    }
    
    lineno = (lineno + 1) % 262;
    if(lineno == 0){
        frame += 1;
    }
}


void main_program_init(PIO pio, uint sm, uint offset, uint pin) {
    // Initialize ROW_PINS amount of pins, starting from function input value pin
    for(int i = 0; i < ROW_PINS; i++) {
        pio_gpio_init(pio, pin + i);
    }
    pio_sm_set_consecutive_pindirs(pio, sm, pin, ROW_PINS, true);
    
    // Start building PIO config
    pio_sm_config c = main_program_get_default_config(offset);
    
    // NOTE: You have to config out and set pins separately, or they just won't write anything
    sm_config_set_out_pins(&c, pin, ROW_PINS);
    sm_config_set_out_shift(&c, false, true, 8);
    
    // Begin PIO state machine
    pio_sm_init(pio, sm, offset, &c);
}

void pset(int16_t xpos, int16_t ypos){
    const int16_t x = xpos + ((color_mode == SCREEN_GRAYSCALE) ? drawing_x_offset * 2: drawing_x_offset);
    const int16_t y = ypos;
    if(x < 0 || x >= _display_size_x){
        return;
    }
    if(y < 0 || y >= _display_size_y){
        return;
    }
    if(color_mode == SCREEN_PALETTE){
        const int32_t c = (((int32_t)y) << 8) + x;
        framebuf[c] = framebuf[c] & 0b11000000 + current_color & 0b00111111;
    }else if(color_mode == SCREEN_GRAYSCALE){
        const int32_t c = (((int32_t)y) << 8) + (x >> 1);
        if((x & 1) == 0){
            // 0: 左
            framebuf[c] = framebuf[c] & 0b00111111 + ((current_color & 0b00000011) << 6);
        }else{
            // 1: 右
            framebuf[c] = framebuf[c] & 0b11111100 + ((current_color & 0b00000011));
        }
    }else if(color_mode == SCREEN_FULLWIDTH_COLOR){
        const int32_t c = (((int32_t)y) << 8) + (x >> 1);
        if((x & 1) == 0){
            // 0: 左
            framebuf[c] = framebuf[c] & 0b00000011 + ((current_color & 0b00000011) << 6) + (current_color & 0b00111100);
        }else{
            // 1: 右
            framebuf[c] = framebuf[c] & 0b11000000 + ((current_color & 0b00000011))      + (current_color & 0b00111100);
        }
    }
}

// 有効なのは 0 から 63 まで
void palcolor(uint8_t palno){
    current_color = palno;
}


// core0 でも init_framedata と init_dma を呼べば動く。USB割り込みで荒れるけど
void init_framedata(){    
    // NTSCカラーの場合:
    for(uint16_t i = 0; i < LINEBUF_LEN; i += 1){
        if(i < 10){
            // front porch
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
            linebuf_vsync[i] = 0b00010001;
        }else if(i == 10){
            // front porch + hsync
            linebuf_a[i] = 0b00010000;
            linebuf_b[i] = 0b00010000;
            linebuf_vblank[i] = 0b00010000;
            linebuf_vsync[i] = 0b00000000;
        }else if(i <= 44){
            // hsync
            linebuf_a[i] = 0b00000000;
            linebuf_b[i] = 0b00000000;
            linebuf_vblank[i] = 0b00000000;
            linebuf_vsync[i] = 0b00000000;
        }else if(i <= 48){
            // back porch(before burst)
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
            linebuf_vsync[i] = 0b00000000;
        }else if(i == 49){
            // back porch(before burst) + COLOR BURST(真ん中スタートとする)
            // つまるところ、カラーバーストの0度位相は、奇数添字の後半要素。
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
            linebuf_vsync[i] = 0b00000000;
        }else if(i <= 67){
            linebuf_vsync[i] = 0b00000000;
            // COLOR BURST
            // カラーバーストは、[0 1 0 -1]のような感じになる
            if((i & 1) == 0){// i % 2 == 0
                linebuf_a[i] = 0b00110001;
                linebuf_b[i] = 0b00110001;
                linebuf_vblank[i] = 0b00110001;
            }else{
                linebuf_a[i] = 0b00100001;
                linebuf_b[i] = 0b00100001;
                linebuf_vblank[i] = 0b00100001;
            }
        }else if(i == 68){
            // COLOR BURST + back porch(after burst)
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
            linebuf_vsync[i] = 0b00000000;
        }else if(i <= 78){
            // back porch(after burst)
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
            linebuf_vsync[i] = 0b00000000;
        }else{// 79...
            // active video
            linebuf_vblank[i] = 0b00100001;
            linebuf_vsync[i] = 0b00000000;
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
        }
    }
    // END NTSCカラーの場合
}

void _remove_colorburst(){
    for(uint16_t i = 49; i < 69; i += 1){
        linebuf_a[i] = 0b00010001;
        linebuf_b[i] = 0b00010001;
        linebuf_vblank[i] = 0b00010001;
    }
    
}

void _restore_colorburst(){
    // init_framedata から転記している。そっちを修正の場合はこちらの修正も忘れないこと!
    for(uint16_t i = 49; i < 69; i += 1){
        if(i == 49){
            // back porch(before burst) + COLOR BURST(真ん中スタートとする)
            // つまるところ、カラーバーストの0度位相は、奇数添字の後半要素。
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
        }else if(i <= 67){
            // COLOR BURST
            // カラーバーストは、[0 1 0 -1]のような感じになる
            if((i & 1) == 0){// i % 2 == 0
                linebuf_a[i] = 0b00110001;
                linebuf_b[i] = 0b00110001;
                linebuf_vblank[i] = 0b00110001;
            }else{
                linebuf_a[i] = 0b00100001;
                linebuf_b[i] = 0b00100001;
                linebuf_vblank[i] = 0b00100001;
            }
        }else if(i == 68){
            // COLOR BURST + back porch(after burst)
            linebuf_a[i] = 0b00010001;
            linebuf_b[i] = 0b00010001;
            linebuf_vblank[i] = 0b00010001;
        }
    }
}

void setDisplayMode(uint16_t mode){
    const uint16_t previous_color_mode = color_mode;
    if(mode == SCREEN_PALETTE){
        // 2: SCREEN_PALETTE。1バイトで色を表し、有効色は 52 色
        if(previous_color_mode == SCREEN_GRAYSCALE){
            _restore_colorburst();
        }
        color_mode = SCREEN_PALETTE;
        current_color = 1;
        _display_size_x = DISP_RES_X - drawing_x_offset;
    }
    if(mode == SCREEN_GRAYSCALE){
        // 1024: SCREEN_PALETTE。1バイトで濃淡を表し、有効色は 4 色
        if(previous_color_mode == SCREEN_PALETTE || previous_color_mode == SCREEN_FULLWIDTH_COLOR){
            _remove_colorburst();
        }
        color_mode = SCREEN_GRAYSCALE;
        current_color = 1;
        _display_size_x = DISP_RES_X_GRAYSCALE - drawing_x_offset * 2;
        
    }
    if(mode == SCREEN_FULLWIDTH_COLOR){
        if(previous_color_mode == SCREEN_GRAYSCALE){
            _restore_colorburst();
        }
        color_mode = SCREEN_FULLWIDTH_COLOR;
        current_color = 1;
        _display_size_x = DISP_RES_X_GRAYSCALE - drawing_x_offset * 2;
    }
}

void init_dma(){
    // PIO の設定
    set_sys_clock_khz(157500, true);
    
    uint offset = pio_add_program(pio, &main_program);
    sm = pio_claim_unused_sm(pio, true);
    main_program_init(pio, sm, offset, START_PIN);
    // システムクロックを 157.5MHz で動かし、PIO はその 1/11 で動かうす。157.5MHz / 44 = 3579545.45...Hz なので、4サンプルで 1ピクセルというわけだ。
    pio_sm_set_clkdiv(pio, sm, 11);
    pio_sm_set_enabled(pio, sm, true);
    
    // DMA の設定
    dma_chan = dma_claim_unused_channel(true);
    dc = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dc, DMA_SIZE_8);
    channel_config_set_dreq(&dc, pio_get_dreq(pio, sm, true));
    channel_config_set_read_increment(&dc, true);
    channel_config_set_write_increment(&dc, false);
    dma_channel_configure(
        dma_chan,
        &dc,
        &pio->txf[sm],
        linebuf_a,
        LINEBUF_LEN,
        true
    );
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, hndirq0);
    irq_set_enabled(DMA_IRQ_0, true);
}


void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    bool steep = std::abs(x1 - x2) < std::abs(y1 - y2);
    if(steep){
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    
    if (x1 > x2){
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    int16_t y = y1;
    int ierror = 0;
    for(int16_t x = x1; x <= x2; x += 1){
        if(steep){
            pset(y, x);
        }else{
            pset(x, y);
        }
        
        ierror += 2 * std::abs(y2 - y1);
        if (ierror > x2 - x1){
            y += y2 > y1 ? 1 : -1;
            ierror -= 2 * (x2 - x1);
        }
        
    }
}

void boxf(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    for(int16_t yc = y1; yc < y1 + (y2 - y1 + 1); yc += 1){
        for(int16_t xc = x1; xc < x1 + (x2 - x1 + 1); xc += 1){
            pset(xc, yc);
        }
    }
}

// ま、正確には待ってる対象は vblank なんだけどね
void wait_for_vsync(){
    const auto f = frame;
    while(f == frame){ /*asm("wfi"); ←core1 で動かしてる以上core0にライン割り込みは飛ばないため*/ }
    return;
}

// 画面クリア
inline uint8_t __clrgraph_pattern(uint8_t clr_mode){
    if(clr_mode == 0){
        return 0b00000000;
    }
    if(color_mode == SCREEN_PALETTE){
        return 0b00110011;
    }
    if(color_mode == SCREEN_FULLWIDTH_COLOR){
        return 0b11110011;
    }
    if(color_mode == SCREEN_GRAYSCALE){
        return 0b11000011;
    }
    return 0b00000000;
};
void clrgraph(uint8_t clr_mode){
    const uint8_t c = __clrgraph_pattern(clr_mode);
    
    for(int32_t i = 0; i < sizeof(framebuf) / sizeof(framebuf[0]); i += 1){
        framebuf[i] = c;
    }
}

void triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
    line(x1, y1, x2, y2);
    line(x2, y2, x3, y3);
    line(x3, y3, x1, y1);
}

void trianglef(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
    
    // bubble sort
    if(y2 > y3){
        std::swap(x2, x3);
        std::swap(y2, y3);
    }
    
    if(y1 > y2){
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    
    if(y2 > y3){
        std::swap(x2, x3);
        std::swap(y2, y3);
    }
    
    for(int16_t ycnt = y1; ycnt < y3; ycnt += 1){
        int16_t xleft;
        if (ycnt < y2){
            xleft = x1 + (((x2 - x1) * 16) * ((ycnt - y1) * 16) / ((y2 - y1) * 16) + 8) / 16;
        }else{
            xleft = x2 + (((x3 - x2) * 16) * ((ycnt - y2) * 16) / ((y3 - y2) * 16) + 8) / 16;
        }
        int xright = x1 + (((x3 - x1) * 16) * ((ycnt - y1) * 16) / ((y3 - y1) * 16) + 8) / 16;
        
        line(xleft, ycnt, xright, ycnt);
    }
    
}


uint8_t is_core1_initialized = 0;
void core1_main(){
    sleep_ms(10);
    init_framedata();
    init_dma();
    is_core1_initialized = 1;
    while(1){
        asm("wfi");
    }
}

// core1 で映像を駆動したい場合はこれ「のみを」呼ぶ
void init_video_on_core1(){
    sleep_ms(100);
    multicore_launch_core1(core1_main);
    sleep_ms(100);
    while(is_core1_initialized == 0){}
}


#endif // __NASTTY_RCA_1BIT__