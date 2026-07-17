#include <cstdint>


#ifndef __NASTTY_CVBS_PIO__
#define __NASTTY_CVBS_PIO__

void pset(int16_t xpos, int16_t ypos);
uint8_t __pget(int16_t xpos, int16_t ypos);
uint8_t pget(int16_t xpos, int16_t ypos);
void __fast_hline(int_fast16_t y, int_fast16_t x1, int_fast16_t x2);
void palcolor(uint8_t palno);
void lcscolor(uint8_t chroma, uint8_t luma, uint8_t saturation);
void init_framedata();
void _remove_colorburst();
void _restore_colorburst();
void setDisplayMode(uint16_t mode);
void init_dma();
void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void boxf(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void box(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void fill(int16_t x, int16_t y);
void wait_for_vsync();
void clrgraph(uint8_t clr_mode);
void triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);
void trianglef(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);
void pos(int16_t x, int16_t y);
void gcopy(uint8_t window_id, int16_t x1, int16_t y1, int16_t xsize, int16_t ysize);
void circle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t fill_mode);
void set_flip_mode(uint8_t flag);
void do_flip();
void vsync_mode(uint8_t mode);
void core1_main();
void init_video_on_core1();

extern uint16_t color_mode;
extern volatile uint16_t lineno;
extern volatile uint32_t frame;

extern volatile uint8_t flip_mode;

extern uint8_t current_color;

extern int16_t ginfo_cx;
extern int16_t ginfo_cy;

typedef void (* fptr_void_void_t)(void);
extern volatile fptr_void_void_t core1_loop;

extern uint8_t flip;


//#define SCREEN_PALETTE 2
#define SCREEN_GRAYSCALE 1024
#define SCREEN_PALETTE 2
#define SCREEN_FULLWIDTH_COLOR 2

#define __TV_PAL_COLOR6(c, y) ((y & 3) + (((c) & 15) << 2))

#ifdef NASTTY_CVBS_DEBUG_OUT
    #define NASTTY_CVBS_DEBUG_PIN 0
#endif


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


#define VIEWPORT_RES_X 360
#define VIEWPORT_RES_Y 232


// タイミング一覧
constexpr const uint16_t LEN_FRONT_PORCH            = 21;
constexpr const uint16_t LEN_SYNC_PULSE             = 67;
constexpr const uint16_t LEN_BACK_PORCH             = 68;
constexpr const uint16_t LEN_ACTIVE_VIDEO           = 756;
constexpr const uint16_t LEN_BEFORE_ACTIVE_VIDEO    = LEN_FRONT_PORCH + LEN_SYNC_PULSE + LEN_BACK_PORCH; //156
constexpr const uint16_t LEN_LINE_LENGTH            = LEN_FRONT_PORCH + LEN_SYNC_PULSE + LEN_BACK_PORCH + LEN_ACTIVE_VIDEO; // 912


constexpr const uint16_t LINEBUF_LEN = LEN_LINE_LENGTH / 2;


// X 方向に 360 pixel。縦は 232 ラインを使う
constexpr const uint16_t DISP_RES_X = (LEN_ACTIVE_VIDEO) / 4;
constexpr const uint16_t DISP_RES_X_GRAYSCALE = LEN_ACTIVE_VIDEO / 2;
constexpr const uint16_t DISP_RES_Y = VIEWPORT_RES_Y;

constexpr const uint16_t _display_size_x = 360;
constexpr const uint16_t _display_size_y = DISP_RES_Y;


#endif // __NASTTY_CVBS_PIO__