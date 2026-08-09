#include "cvbs.hpp"

// ----------------------------------------------------------------

#include <cstdint>

#include "hardware/gpio.h"

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
#include "hardware/structs/bus_ctrl.h"

#ifndef START_PIN
    #define START_PIN 16
#endif
#define ROW_PINS 4

// PIO program is a simple pull and shift out
// simply copy & paste the following into main.pio file
// .program main
//     pull
//     out pins, 4
int cvbs_dma_chan[2];


// 画面モードに関する変数
uint16_t color_mode = SCREEN_PALETTE;
uint8_t ginfo_paluse = 1;
int8_t drawing_x_offset = 8;

volatile uint16_t lineno = 0;
volatile uint32_t frame = 0;
// もともとのコードでは 2bit だったが、今回から 4bit (同期信号のほかに 3bit 分の信号がある。モノクロなら 8 階調分)
// 色数は (有彩色 12 + 無彩色 1) x 4 = 52 色。なんとやり方がファミコンといっしょでびっくり!
// でも、DAC の階調が少ない関係で輝度 100% ではカラーバーストの波形を正しく現せず、実際はもう少し少ない。
alignas(4) uint8_t linebuf_a[LINEBUF_LEN];
alignas(4) uint8_t linebuf_b[LINEBUF_LEN];
uint8_t* ptr_linebuf[2] = {linebuf_a, linebuf_b};
alignas(4) uint8_t linebuf_vblank[LINEBUF_LEN];
alignas(4) uint8_t linebuf_vsync[LINEBUF_LEN];
uint8_t* ptr_linebuf_vsync = &linebuf_vsync[0];


constexpr const size_t FRAMEBUF_MEM_SIZE = 192 * DISP_RES_Y;
alignas(4) uint8_t framebuf[FRAMEBUF_MEM_SIZE * 2];


uint32_t flip_offset = 0;
uint32_t flip_draw_offset = 0;

volatile uint8_t flip_mode = 0;


PIO cbvs_pio = pio0;
uint cvbs_sm;
dma_channel_config cvbs_dc;
uint8_t current_color = 1;


int16_t ginfo_cx = 0;
int16_t ginfo_cy = 0;

// いや 3 周してる!!!!!!!!! (いちおう、if を減らしたほうが性能上がるんじゃいかな～みたいな淡い期待がある)
__not_in_flash("") const int8_t sin12[36] = {
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
__not_in_flash("") const uint8_t amp2out[16] = {
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
#define AMPINDEX_0IRE 5

volatile fptr_void_void_t core1_loop = nullptr;


uint8_t flip = 0;
volatile uint8_t* ptr_next_dma_buf = linebuf_vblank;
void __not_in_flash_func(hndirq0)(void){
    
    dma_hw->ints0 = 1u << cvbs_dma_chan[flip];
    dma_channel_abort(cvbs_dma_chan[flip]);

    #ifdef NASTTY_CVBS_DEBUG_OUT
        gpio_put(NASTTY_CVBS_DEBUG_PIN, 1);
    #endif
    
    const auto prev_flip = flip; 
    flip = (flip + 1) & 1;
           
    if(lineno > (28 - 1) && lineno <= 20 + 8 + 240 - 8){
        // 次の flip に対して書込処理を行う
        constexpr const uint_fast16_t xindex_base = 79;
        const uint_fast16_t linenum = (lineno - 28);
        const uint_fast32_t lineoffset = (linenum << 7) + (linenum << 6);
        // 映像として有効な x 方向のlinebufの添字は、79～454(455は捨てる)の376バイト、188ピクセル。
        
        if(color_mode == SCREEN_GRAYSCALE){
            // BEGIN LINE_DATA_CONSTRUCT WHEN SCREEN_GRAYSCALE
            for(uint_fast16_t i = 0; i < 188; i += 1){
                #ifndef VIDEO_TEST_PTN_COLOR
                const uint_fast8_t pxdat = framebuf[flip_draw_offset + lineoffset + i];
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
            // BEGIN LINE_DATA_CONSTRUCT WHEN SCREEN_PALETTE
            uint8_t* framebufptr = &framebuf[flip_draw_offset + lineoffset];
            uint8_t* linebufptr  = &ptr_linebuf[flip][xindex_base];
            for(uint_fast16_t i = 0; i < 188; i += 1){
                const uint_fast8_t pxdat = *framebufptr;
                framebufptr += 1;

                const int_fast8_t pxvalue[2] = {
                    ((int_fast8_t)((pxdat >> 6) & 0b00000011)),
                    ((int_fast8_t)( pxdat       & 0b00000011))
                };
                const int_fast8_t pxcolorphase =  (pxdat & 0b00111100) >> 2;
                const uint_fast8_t pxcolorvalue = (pxcolorphase < 12);

                const uint_fast8_t subpx[4] = {
                    amp2out[(pxvalue[0] << 1) + sin12[(pxcolorphase) + 0] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue[0] << 1) + sin12[(pxcolorphase) + 3] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue[1] << 1) + sin12[(pxcolorphase) + 6] * pxcolorvalue + AMPINDEX_0IRE],
                    amp2out[(pxvalue[1] << 1) + sin12[(pxcolorphase) + 9] * pxcolorvalue + AMPINDEX_0IRE]
                };
                *linebufptr = (subpx[0] << 4) + (subpx[1]);
                linebufptr += 1;
                *linebufptr = (subpx[2] << 4) + (subpx[3]);
                linebufptr += 1;
            }
            // END LINE_DATA_CONSTRUCT WHEN SCREEN_PALETTE
        }
    }
    
    lineno = (lineno + 1) % 262;
    // BEGIN Next DMA Settings
    if(lineno == 0){
        frame += 1;
    }
    //   3 Before Porch 
    //   3 Vsync
    //  14 After Porch
    // 242 Video (front 8 + back 2 lines are inactive)
    if(lineno <= 3){
        //   3 Before Porch
        ptr_next_dma_buf = linebuf_vblank;
        //dma_channel_set_read_addr(dma_chan, linebuf_vblank, false);
    }else if(lineno <= 6){
        //   3 Vsync
        ptr_next_dma_buf = ptr_linebuf_vsync;
        //dma_channel_set_read_addr(dma_chan, ptr_linebuf_vsync, false);
    }else if(lineno <= 20 + 8){
        //  14 After Porch + 8 no video
        ptr_next_dma_buf =linebuf_vblank;
        //dma_channel_set_read_addr(dma_chan, linebuf_vblank, false);
    }else if(lineno <= 20 + 8 + 240 - 8){
        // 232 Video
        ptr_next_dma_buf = ptr_linebuf[flip];
        //dma_channel_set_read_addr(dma_chan, ptr_linebuf[flip], false);
    }else{
        //  2 after video
        ptr_next_dma_buf =linebuf_vblank;
        //dma_channel_set_read_addr(dma_chan, linebuf_vblank, false);
    }
    
    dma_channel_set_read_addr(cvbs_dma_chan[prev_flip], ptr_next_dma_buf, false);
    // END-- Next DMA Settings
    
    #ifdef NASTTY_CVBS_DEBUG_OUT
        gpio_put(NASTTY_CVBS_DEBUG_PIN, 0);
    #endif
}


void main_program_init(PIO pio, uint sm, uint offset, uint pin) {
    // Initialize ROW_PINS amount of pins, starting from function input value pin
    for(int i = 0; i < ROW_PINS; i++) {
        pio_gpio_init(pio, pin + i);
    }
    pio_sm_set_consecutive_pindirs(pio, sm, pin, ROW_PINS, true);
    
    // Start building PIO config
    pio_sm_config c = main_program_get_default_config(offset);
    
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    
    // NOTE: You have to config out and set pins separately, or they just won't write anything
    sm_config_set_out_pins(&c, pin, ROW_PINS);
    sm_config_set_out_shift(&c, false, true, 8);
    
    // Begin PIO state machine
    pio_sm_init(pio, sm, offset, &c);
}

void pset(int16_t xpos, int16_t ypos){
    const int_fast16_t x = xpos + drawing_x_offset;
    const int_fast16_t y = ypos;
    if(x < 0 || x >= _display_size_x + drawing_x_offset){
        return;
    }
    if(y < 0 || y >= _display_size_y){
        return;
    }
    
    if(color_mode == SCREEN_GRAYSCALE){
        const int32_t c = (((int32_t)y) << 7) + (((int32_t)y) << 6) + (x >> 1);
        if((x & 1) == 0){
            // 0: 左
            framebuf[flip_offset + c] = framebuf[flip_offset + c] & 0b00111111 + ((current_color & 0b00000011) << 6);
        }else{
            // 1: 右
            framebuf[flip_offset + c] = framebuf[flip_offset + c] & 0b11111100 + ((current_color & 0b00000011));
        }
    }else/* if(color_mode == SCREEN_PALETTE)*/{
        const int32_t c = (((int32_t)y) << 7) + (((int32_t)y) << 6) + (x >> 1);
        if((x & 1) == 0){
            // 0: 左
            framebuf[flip_offset + c] = (framebuf[flip_offset + c] & 0b00000011) + ((current_color & 0b00000011) << 6) + (current_color & 0b00111100);
        }else{
            // 1: 右
            framebuf[flip_offset + c] = (framebuf[flip_offset + c] & 0b11000000) + ((current_color & 0b00000011))      + (current_color & 0b00111100);
        }
    }
}

uint8_t __pget(int16_t xpos, int16_t ypos){
    const int_fast16_t x = xpos + drawing_x_offset;
    const int_fast16_t y = ypos;
    uint8_t cc;
    // SCREEN_GRAYSCALE or SCREEN_PALETTE
    const int32_t c = (((int32_t)y) << 7) + (((int32_t)y) << 6) + (x >> 1);
    if((x & 1) == 0){
        // 0: 左
        cc = (framebuf[flip_offset + c] >> 6) + (framebuf[flip_offset + c] & 0b00111100);
    }else{
        // 1: 右
        cc =  (framebuf[flip_offset + c] & 0b00111111);
    }
    return cc;
}

uint8_t pget(int16_t xpos, int16_t ypos){
    if(xpos < 0 || xpos >= _display_size_x){
        current_color = 0;
        return 0;
    }
    if(ypos < 0 || ypos >= _display_size_y){
        current_color = 0;
        return 0;
    }
    
    current_color = __pget(xpos, ypos);
    return current_color;
}

// SCREEN_PALETTE でしか使わないこと
void __fast_hline(int_fast16_t y, int_fast16_t x1, int_fast16_t x2){
    
    const int_fast16_t _x1 = (x1 > x2) ? x2 + drawing_x_offset: x1 + drawing_x_offset;
    const int_fast16_t _x2 = (x1 > x2) ? x1 + drawing_x_offset: x2 + drawing_x_offset;
   
    if(y < 0 || y >= _display_size_y){
        return;
    }
    
    if(_x2 < 0){
        return;
    }
    
    const int_fast16_t xr1 = _x1 > drawing_x_offset                   ? ((_x1 + 1) & (~1)) : drawing_x_offset;
    const int_fast16_t xr2 = _x2 < _display_size_x + drawing_x_offset ? ((_x2 - 1) & (~1)) : _display_size_x + drawing_x_offset;
    const uint8_t cc = current_color + (current_color << 6);
    const int32_t c = (((int32_t)y) << 7) + (((int32_t)y) << 6) + (xr1 >> 1);
    uint8_t* fbptr = &framebuf[flip_offset + c];
    for(int_fast16_t xcnt = xr1; xcnt <= xr2; xcnt += 2){
        *fbptr = cc;
        fbptr += 1;
    }
    
    if(xr1 != _x1){
        pset(_x1 - drawing_x_offset, y);
    }
    if(xr2 != _x2){
        pset(_x2 - drawing_x_offset, y);
    }
}

// 有効なのは 0 から 63 まで
void palcolor(uint8_t palno){
    current_color = palno;
}

void lcscolor(uint8_t chroma, uint8_t luma, uint8_t saturation){
    if(saturation == 0){
        current_color = (0xC << 2) + (luma & 3);
    }else{
        current_color = ((chroma % 12) << 2) + (luma & 3);
    }
}

// core0 でも init_framedata と init_dma を呼べば動く。USB割り込みで荒れるけど
void init_framedata(){    
    // NTSCカラーの場合:
    for(uint_fast16_t i = 0; i < LINEBUF_LEN; i += 1){
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
    for(uint_fast16_t i = 49; i < 69; i += 1){
        linebuf_a[i] = 0b00010001;
        linebuf_b[i] = 0b00010001;
        linebuf_vblank[i] = 0b00010001;
    }
    
}

void _restore_colorburst(){
    // init_framedata から転記している。そっちを修正の場合はこちらの修正も忘れないこと!
    for(uint_fast16_t i = 49; i < 69; i += 1){
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
    if(mode == SCREEN_GRAYSCALE){
        // 1024: SCREEN_PALETTE。1バイトで濃淡を表し、有効色は 4 色
        if(previous_color_mode == SCREEN_PALETTE){
            _remove_colorburst();
        }
        color_mode = SCREEN_GRAYSCALE;
        current_color = 1;
    }else if(mode == SCREEN_PALETTE){
        // 2: SCREEN_PALETTE。有効色52色。左右2ピクセル単位でクロマ信号を共有する
        if(previous_color_mode == SCREEN_GRAYSCALE){
            _restore_colorburst();
        }
        color_mode = SCREEN_PALETTE;
        current_color = 1;
    }
}

void init_dma(){
    // PIO の設定
    set_sys_clock_khz(157500, true);
    
    uint offset = pio_add_program(cbvs_pio, &main_program);
    cvbs_sm = pio_claim_unused_sm(cbvs_pio, true);
    main_program_init(cbvs_pio, cvbs_sm, offset, START_PIN);
    // システムクロックを 157.5MHz で動かし、PIO はその 1/11 で動かうす。157.5MHz / 44 = 3579545.45...Hz なので、4サンプルで 1ピクセルというわけだ。
    pio_sm_set_clkdiv(cbvs_pio, cvbs_sm, 11);
    pio_sm_set_enabled(cbvs_pio, cvbs_sm, true);
    
    // DMA の設定
    for(uint i = 0; i < 2; i += 1){
        cvbs_dma_chan[i] = dma_claim_unused_channel(true);
    }
    
    // for channel normal
    cvbs_dc = dma_channel_get_default_config(cvbs_dma_chan[0]);
    channel_config_set_transfer_data_size(&cvbs_dc, DMA_SIZE_8);
    channel_config_set_dreq(&cvbs_dc, pio_get_dreq(cbvs_pio, cvbs_sm, true));
    channel_config_set_read_increment(&cvbs_dc, true);
    channel_config_set_write_increment(&cvbs_dc, false);
    channel_config_set_high_priority(&cvbs_dc, true);
    channel_config_set_chain_to(&cvbs_dc, cvbs_dma_chan[1]);
    dma_channel_configure(
        cvbs_dma_chan[0],
        &cvbs_dc,
        &cbvs_pio->txf[cvbs_sm],
        linebuf_a,
        LINEBUF_LEN,
        false
    );
    dma_channel_set_irq0_enabled(cvbs_dma_chan[0], true);
    
    // for channel flip
    cvbs_dc = dma_channel_get_default_config(cvbs_dma_chan[1]);
    channel_config_set_transfer_data_size(&cvbs_dc, DMA_SIZE_8);
    channel_config_set_dreq(&cvbs_dc, pio_get_dreq(cbvs_pio, cvbs_sm, true));
    channel_config_set_read_increment(&cvbs_dc, true);
    channel_config_set_write_increment(&cvbs_dc, false);
    channel_config_set_high_priority(&cvbs_dc, true);
    channel_config_set_chain_to(&cvbs_dc, cvbs_dma_chan[0]);
    dma_channel_configure(
        cvbs_dma_chan[1],
        &cvbs_dc,
        &cbvs_pio->txf[cvbs_sm],
        linebuf_a,
        LINEBUF_LEN,
        false
    );
    dma_channel_set_irq0_enabled(cvbs_dma_chan[1], true);
    
    irq_set_exclusive_handler(DMA_IRQ_0, hndirq0);
    
    irq_set_priority(DMA_IRQ_0, 0x00);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_start(cvbs_dma_chan[0]);
    
    #ifdef NASTTY_CVBS_DEBUG_OUT
        gpio_init(NASTTY_CVBS_DEBUG_PIN);
        gpio_set_dir(NASTTY_CVBS_DEBUG_PIN, GPIO_OUT);
    #endif
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
    for(int_fast16_t x = x1; x <= x2; x += 1){
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
    const int_fast16_t xr1 = (x1 > 0              ) ? x1 : 0;
    const int_fast16_t xr2 = (x2 < _display_size_x) ? x2 : _display_size_x;
    const int_fast16_t yr1 = (y1 > 0              ) ? y1 : 0;
    const int_fast16_t yr2 = (y2 < _display_size_y) ? y2 : _display_size_y;
    for(int_fast16_t yc = yr1; yc < yr1 + (yr2 - yr1 + 1); yc += 1){
        __fast_hline(yc, xr1, xr2);
    }
}

void box(int16_t x1, int16_t y1, int16_t x2, int16_t y2){
    line(x1, y1, x1, y2);
    line(x2, y1, x2, y2);
    line(x1, y1, x2, y1);
    line(x1, y2, x2, y2);
}


// カスの実装なので閉空間じゃないと(おそらくスタックオーバーフローで)クラッシュするし、遅い
void fill(int16_t x, int16_t y){
    if(x < 0 || x >= _display_size_x + drawing_x_offset){
        return;
    }
    if(y < 0 || y >= _display_size_y){
        return;
    }
    if(__pget(x, y) == current_color){
        return;
    }
    const uint8_t target_color = __pget(x, y);

    int_fast16_t lx = x;
    int_fast16_t rx = x;
    while(1){
        if(lx == 0){
            break;
        }
        if(__pget(lx, y) != target_color){
            lx += 1;
            break;
        }
        lx -= 1;
    }
    while(1){
        if(rx == _display_size_x + drawing_x_offset - 1){
            break;
        }
        if(__pget(rx, y) != target_color){
            rx -= 1;
            break;
        }
        rx += 1;
    }
    __fast_hline(y, lx, rx);
    
    if(y >= 1){
        for(int_fast16_t xcnt = lx; xcnt < rx; xcnt += 1){
            if(__pget(xcnt, y - 1) == target_color){
                fill(xcnt, y - 1);
            }
        }
    }
    if(y <= _display_size_y - 1){
        for(int_fast16_t xcnt = lx; xcnt < rx; xcnt += 1){
            if(__pget(xcnt, y + 1) == target_color){
                fill(xcnt, y + 1);
            }
        }
    }
}

// ま、正確には待ってる対象は vblank なんだけどね
void wait_for_vsync(){
    const auto f = frame;
    while(f == frame){ tight_loop_contents();/*asm("wfi"); ←core1 で動かしてる以上core0にライン割り込みは飛ばないため*/ }
    return;
}

// 画面クリア
inline uint8_t __clrgraph_pattern(uint8_t clr_mode){
    if(clr_mode == 0){
        return 0b00110000;
    }
    if(color_mode == SCREEN_PALETTE){
        return 0b11110011;
    }
    if(color_mode == SCREEN_GRAYSCALE){
        return 0b11000011;
    }
    return 0b00000000;
};

void clrgraph(uint8_t clr_mode){
    const uint8_t c = __clrgraph_pattern(clr_mode);
    
    for(int_fast32_t i = 0; i < FRAMEBUF_MEM_SIZE; i += 1){
        framebuf[flip_offset + i] = c;
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
    
    for(int_fast16_t ycnt = y1; ycnt < y3; ycnt += 1){
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

void pos(int16_t x, int16_t y){
    ginfo_cx = x;
    ginfo_cy = y;
}

void gcopy(uint8_t window_id, int16_t x1, int16_t y1, int16_t xsize, int16_t ysize){
    const uint8_t cc = current_color;
    if(x1 >= _display_size_x + drawing_x_offset || y1 >= _display_size_y){
        return;
    }
    for(int_fast16_t ycnt = 0; ycnt < ysize; ycnt += 1){
        for(int_fast16_t xcnt = 0; xcnt < xsize; xcnt += 1){
            pget(x1 + xcnt, y1 + ycnt);
            pset(ginfo_cx + xcnt, ginfo_cy + ycnt);
        }
    }
    current_color = cc;
}

void circle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fill_mode){
    // そうだ! 全部「2 倍」で扱おう
    const int_fast16_t xbase = (x1 + x2);
    const int_fast16_t ybase = (y1 + y2);
    const int_fast16_t a = (int_fast16_t)abs(x2 - x1);
    const int_fast16_t b = (int_fast16_t)abs(y2 - y1);
    const int_fast16_t asq = a * a;
    const int_fast16_t bsq = b * b;
    
    int_fast16_t oldx = 0;
    
    if(fill_mode != 0){
        // 塗りつぶし
        for(int_fast16_t ycnt = - b; ycnt <= 0; ycnt += 1){
            if((ycnt % 2) != 0){
                continue;
            }
            // 2 の倍数ってことはスクリーン座標的に整数
            // 注意: xc は 2 倍になってない
            const int xc = (asq - ((int)ycnt * ycnt * asq / bsq));
            // たぶんテーブルにするより double の計算したほうが速い
            const int_fast16_t newx = (int)sqrt(xc);
            const int_fast16_t newy = (int)ycnt;
            
            __fast_hline((ybase - newy) / 2, (xbase - newx) / 2, (xbase + newx) / 2);
            __fast_hline((ybase + newy) / 2, (xbase - newx) / 2, (xbase + newx) / 2);
            oldx = newx;
        }
        
    }else /*if(fill_mode == 0)*/{
        // 輪郭
        for(int_fast16_t ycnt = - b; ycnt <= 0; ycnt += 1){
            if((ycnt % 2) != 0){
                continue;
            }
            // 2 の倍数ってことはスクリーン座標的に整数
            // 注意: xc は 2 倍になってない
            const int xc = (asq - ((int)ycnt * ycnt * asq / bsq));
            // たぶんテーブルにするより double の計算したほうが速い
            const int_fast16_t newx = (int)sqrt(xc);
            const int_fast16_t newy = (int)ycnt;
            
            // fast_hline より line のほうが速く、意味不明
            line((xbase - newx) / 2, (ybase - newy) / 2, (xbase - oldx) / 2, (ybase - newy) / 2);
            line((xbase + newx) / 2, (ybase - newy) / 2, (xbase + oldx) / 2, (ybase - newy) / 2);
            line((xbase - newx) / 2, (ybase + newy) / 2, (xbase - oldx) / 2, (ybase + newy) / 2);
            line((xbase + newx) / 2, (ybase + newy) / 2, (xbase + oldx) / 2, (ybase + newy) / 2);
            oldx = newx;
        }
    }
}

void set_flip_mode(uint8_t flag){
    flip_mode = (flag != 0) ? 1 : 0;
    if(flip_mode == 0){
        flip_offset = 0;
        flip_draw_offset = 0;
    }
    if(flip_mode == 1){
        flip_draw_offset = 0;
        flip_offset = FRAMEBUF_MEM_SIZE;
    }
}

void do_flip(){
    if(flip_mode == 0){
        return;
    }
    
    if(flip_offset != 0){
        flip_offset = 0;
        flip_draw_offset = FRAMEBUF_MEM_SIZE;
    }else{
        flip_offset = FRAMEBUF_MEM_SIZE;
        flip_draw_offset = 0;
    }
}

// vsync を送出しない場合は 1 にする
void vsync_mode(uint8_t mode){
    if(mode == 0){
        ptr_linebuf_vsync = &linebuf_vsync[0];
    }
    if(mode == 1){
        ptr_linebuf_vsync = &linebuf_vblank[0];
    }
}

volatile uint8_t is_core1_initialized = 0;


void core1_main(){
    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_R_BITS | BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_PROC1_BITS;
    init_framedata();
    init_dma();
    is_core1_initialized = 1;
    while(1){
        if(core1_loop != nullptr){
            (*core1_loop)();
        }
        //asm("wfi");
    }
}

// core1 で映像を駆動したい場合はこれ「のみを」呼ぶ
void init_video_on_core1(){
    sleep_ms(50);
    multicore_reset_core1();
    multicore_launch_core1(core1_main);
    while(is_core1_initialized == 0){}
}
