#include "rca.hpp"

namespace swimming_triangle{
    uint32_t c = 0;
    uint32_t f = 0;
    
    int16_t x[3] = {20, 120, 70};
    int16_t y[3] = {20, 120, 180};
    int8_t ys[3] = {3, 3, 3};
    int8_t xs[3] = {3, 3, 3};
    void frame(){
        
        f = ::frame % (64 * 224);
        c += 1;
        palcolor(f >> 6);
        for(uint8_t i = 0; i < 3; i += 1){
            x[i] += xs[i];
            y[i] += ys[i];
            if(x[i] <= 0 && xs[i] < 0){
                xs[i] = -xs[i];
            }
            if(y[i] <= 0 && ys[i] < 0){
                ys[i] = -ys[i];
            }
            if(x[i] >= ::_display_size_x - 1 && xs[i] > 0){
                xs[i] = -xs[i];
            }
            if(y[i] >= ::_display_size_y - 1 && ys[i] > 0){
                ys[i] = -ys[i];
            }
        }
        trianglef(x[0], y[0], x[1], y[1], x[2], y[2]);
    }
}
