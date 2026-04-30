#include "rca.hpp"
#include "rcavt.hpp"
#include <stdio.h>

namespace color_palette{
    void draw(){
        clrgraph(1);
        const int left = _display_size_x / 2 - 160;
        const int top = _display_size_y / 2 - 80;
        for(int i = 0; i < 16; i += 1){
            char s[2];
            snprintf(s, 2, "%.1X", i);
            palcolor(0);
            tvvt::put_char_graphic(s[0], left + 20 * i + 6, top - 10);
        }
        for(int i = 0; i < 4; i += 1){
            char s[2];
            snprintf(s, 2, "%.1X", i);
            palcolor(0);
            tvvt::put_char_graphic(s[0], left - 10, top + 20 * i + 6);
        }
        for(int i = 0; i < 64; i += 1){
            const int x = (i / 4) % 16 * 20 + left;
            const int y = (i % 4) * 20 + top;
            palcolor(i);
            boxf(x, y, x + 20, y + 20);
        }
        const int left2 = _display_size_x / 2 - 110;
        const int top2 = _display_size_y / 2 + 40;
        const int preset_colors[11] = {COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN, COLOR_SKYBLUE, COLOR_BLUE, COLOR_PURPLE, COLOR_BLACK, COLOR_DARKGRAY, COLOR_LIGHTGRAY, COLOR_WHITE};
        for(int i = 0; i < 10; i += 1){
            palcolor(preset_colors[i]);
            boxf(left2 + 20 * i, top2, left2 + 20 * i + 20, top2 + 20);
        }

    }
}