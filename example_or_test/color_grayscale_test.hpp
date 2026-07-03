#include "rca.hpp"
#include "rcavt.hpp"

namespace c_g_test{
    void draw(){
        for(int i = 0; i < 96; i += 1){
            int x = _display_size_x / 2 - 64 + (i % 16) * 8;
            int y = _display_size_y / 2 - 24 + (i / 16) * 8 - 64;
            palcolor(COLOR_BLACK);
            tvvt::put_ascii_graphic((char)(i + 32), x, y);
        }
        const int left = _display_size_x / 2 - 128;
        const int top = _display_size_y / 2;
        for(int i = 0; i < 64; i += 1){
            const int x = (i / 4) % 16 * 16 + left;
            const int y = (i % 4) * 16 + top;
            palcolor(i);
            boxf(x, y, x + 16, y + 16);
        }
    }
}