#include "cvbs.hpp"
#include "cvbs_vt.hpp"

namespace fonts_show{
    void draw(){
        clrgraph(1);
        for(int i = 0; i < 96; i += 1){
            int x = _display_size_x / 2 - 64 + (i % 16) * 8;
            int y = _display_size_y / 2 - 24 + (i / 16) * 8;
            palcolor(COLOR_BLACK);
            tvvt::put_ascii_graphic((char)(i + 32), x, y);
        }
    }
}