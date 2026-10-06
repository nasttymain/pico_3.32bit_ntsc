#include <cstdint>
#include <stddef.h>

#ifndef __NASTTY_CVBS_CORE_H__
#define __NASTTY_CVBS_CORE_H__

#define  COLOR_RED        __TV_PAL_COLOR6(0x7, 0x1)
#define  COLOR_ORANGE     __TV_PAL_COLOR6(0x7, 0x2)
#define  COLOR_YELLOW     __TV_PAL_COLOR6(0x8, 0x2)
#define  COLOR_GREEN      __TV_PAL_COLOR6(0xB, 0x2)
#define  COLOR_SKYBLUE    __TV_PAL_COLOR6(0x1, 0x2)
#define  COLOR_BLUE       __TV_PAL_COLOR6(0x2, 0x0)
#define  COLOR_PURPLE     __TV_PAL_COLOR6(0x5, 0x2)
#define  COLOR_BLACK      __TV_PAL_COLOR6(0xC, 0x0)
#define  COLOR_DARKGRAY   __TV_PAL_COLOR6(0xC, 0x1)
#define  COLOR_LIGHTGRAY  __TV_PAL_COLOR6(0xC, 0x2)
#define  COLOR_WHITE      __TV_PAL_COLOR6(0xC, 0x3)


void video_pointer_set(int16_t x, int16_t y, bool appear = true);
void pset(int16_t xpos, int16_t ypos);
uint8_t __pget(int16_t xpos, int16_t ypos);
uint8_t pget(int16_t xpos, int16_t ypos);
void __fast_hline(int_fast16_t y, int_fast16_t x1, int_fast16_t x2);
void palcolor(uint8_t palno);
void init_framedata();
void _remove_colorburst();
void _restore_colorburst();
void setDisplayMode(uint16_t mode);
void init_dma();
void set_flip_mode(uint8_t flag);
void do_flip();
void wait_for_vsync();
void vsync_mode(uint8_t mode);
void core1_main();
void init_video_on_core1();


extern uint8_t __clrgraph_pattern(uint8_t clr_mode);

extern volatile uint16_t lineno;
extern volatile uint32_t frame;
extern uint16_t color_mode;
extern uint8_t ginfo_paluse;
extern int8_t drawing_x_offset;
extern volatile uint8_t flip_mode;
extern uint8_t current_color;
extern int16_t ginfo_cx;
extern int16_t ginfo_cy;


#define SCREEN_GRAYSCALE 1024
#define SCREEN_PALETTE 2
#define SCREEN_FULLWIDTH_COLOR 2

#define __TV_PAL_COLOR6(c, y) ((y & 3) + (((c) & 15) << 2))

#ifdef NASTTY_CVBS_DEBUG_OUT
    #define NASTTY_CVBS_DEBUG_PIN 0
#endif



#define VIEWPORT_RES_X 360
#define VIEWPORT_RES_Y 240




extern uint32_t flip_offset;

typedef void (* fptr_void_void_t)(void);
extern volatile fptr_void_void_t core1_loop;

extern uint8_t flip;


// タイミング一覧
constexpr const uint16_t LEN_FRONT_PORCH            = 21;
constexpr const uint16_t LEN_SYNC_PULSE             = 67;
constexpr const uint16_t LEN_BACK_PORCH             = 68;
constexpr const uint16_t LEN_ACTIVE_VIDEO           = 754; // was 756
constexpr const uint16_t LEN_BEFORE_ACTIVE_VIDEO    = LEN_FRONT_PORCH + LEN_SYNC_PULSE + LEN_BACK_PORCH; //156
constexpr const uint16_t LEN_LINE_LENGTH            = LEN_FRONT_PORCH + LEN_SYNC_PULSE + LEN_BACK_PORCH + LEN_ACTIVE_VIDEO; // 910


constexpr const uint16_t LINEBUF_LEN = LEN_LINE_LENGTH / 2;


// X 方向に 360 pixel。縦は 240 ラインを使う
constexpr const uint16_t DISP_RES_X = VIEWPORT_RES_X;
constexpr const uint16_t DISP_RES_Y = VIEWPORT_RES_Y;

constexpr const uint16_t _display_size_x = VIEWPORT_RES_X;
constexpr const uint16_t _display_size_y = VIEWPORT_RES_Y;



constexpr const size_t FRAMEBUF_MEM_SIZE = 192 * DISP_RES_Y;
extern uint8_t framebuf[FRAMEBUF_MEM_SIZE * 2];


#endif