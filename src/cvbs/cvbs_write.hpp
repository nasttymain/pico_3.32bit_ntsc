#include "cvbs_core.hpp"
#include "cvbs_draw.hpp"

#ifndef __NASTTY_CVBS_WRITE_H__
#define __NASTTY_CVBS_WRITE_H__

#include <cstdint>

extern int16_t ginfo_mesx;
extern int16_t ginfo_mesy;

namespace videowrite{
    #ifndef uint
    typedef unsigned int uint;
    #endif
    extern uint fontsize_x;
    extern uint fontsize_y;
    constexpr const uint8_t hori_tab_size = 2;
    constexpr const uint8_t do_auto_cr = 1;


    void mes(const char* s, int mode_switch = 0);
    void font(const char* fontname = nullptr, uint16_t pt = 8, int style = 0);

}; // videowrite

using videowrite::mes;
using videowrite::font;

#endif//__NASTTY_CVBS_WRITE_H__
