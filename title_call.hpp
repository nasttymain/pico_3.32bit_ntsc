#include "rca.hpp"
#include "rcavt.hpp"
#include <stdio.h>

namespace title_call{
    void draw(){
        clrgraph(1);
        palcolor(COLOR_BLACK);
        tvvt::pos(8, 13);
        tvvt::puts("@2025-2026");
        tvvt::pos(22, 13);
        tvvt::puts("/\\/asTTY");
        
        tvvt::pos(8, 15);
        tvvt::puts("@2025-2026");
        tvvt::pos(22, 15);
        tvvt::puts("Raspberry Pi Pico");
        
        
        tvvt::pos(8, 17);
        tvvt::puts("@2025-2026");
        tvvt::pos(22, 17);
        tvvt::puts("NTSC Video Output");
    }
}