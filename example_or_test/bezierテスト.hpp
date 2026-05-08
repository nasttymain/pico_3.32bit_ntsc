#include "rca.hpp"
#include <cstdint>

namespace test_bezier{
    tvbezier::bezier_t b;
    tvbezier::bezier_t bdiv;
    uint is_initialized = 0;
    
    void frame(){
        if(is_initialized == 0){
            is_initialized = 1;
            b.x1 =  60;
            b.xc =   5;
            b.x2 = 240;
            b.y1 =  20;
            b.yc = 223;
            b.y2 = 180;
        }
        pset(b.xc, b.yc);

        tvbezier::get_divided_bezier(b, bdiv, 0.5);
        tvbezier::qbezier(bdiv, (uint16_t)( sinf((float)(::frame) / 60) * 20 + 20 ));
        pset(bdiv.xc, bdiv.yc);

        tvbezier::get_divided_bezier_r(b, bdiv, 0.5);
        tvbezier::qbezier(bdiv, (uint16_t)( sinf((float)(::frame) / 60) * 20 + 20 ));
        pset(bdiv.xc, bdiv.yc);
    }

}