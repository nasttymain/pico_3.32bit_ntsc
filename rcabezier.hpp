#include "rca.hpp"

#ifndef __NASTTY_RCA_BEZIER__
#define __NASTTY_RCA_BEZIER__

namespace tvbezier{
    #include <cstdint>
    typedef struct bezier_s{
        int16_t x1;
        int16_t y1;
        int16_t xc;
        int16_t yc;
        int16_t x2;
        int16_t y2;
    } bezier_t;
    void bezier(int16_t x1, int16_t y1, int16_t xc, int16_t yc, int16_t x2, int16_t y2, uint16_t lines){
        const int_fast16_t _x1 = x1;
        const int_fast16_t _y1 = y1;
        const int_fast16_t _xc = xc;
        const int_fast16_t _yc = yc;
        const int_fast16_t _x2 = x2;
        const int_fast16_t _y2 = y2;
        if(lines == 0){
            return;
        }
        int_fast16_t prev_x = _x1;
        int_fast16_t prev_y = _y1;
        for(uint_fast16_t pcnt = 0; pcnt < lines; pcnt += 1){
            const float t = (float)(pcnt + 1) / lines;
            const int_fast16_t x = (int_fast16_t)(t * t * (_x1 - 2 * _xc + _x2) + t * (-2 * _x1 + 2 * _xc) + _x1);
            const int_fast16_t y = (int_fast16_t)(t * t * (_y1 - 2 * _yc + _y2) + t * (-2 * _y1 + 2 * _yc) + _y1);
            line(prev_x, prev_y, x, y);
            prev_x = x;
            prev_y = y;
        }
    }
    
    void qbezier(bezier_t target, uint16_t lines){
        bezier(target.x1, target.y1, target.xc, target.yc, target.x2, target.y2, lines);
    }
    
    void get_divided_bezier(bezier_t& __restrict  target, bezier_t& __restrict result, float t){
        const int16_t xnewc = target.x1 + t * (target.xc - target.x1);
        const int16_t ynewc = target.y1 + t * (target.yc - target.y1);
        const int16_t x = (int16_t)(t * t * (target.x1 - 2 * target.xc + target.x2) + t * (-2 * target.x1 + 2 * target.xc) + target.x1);
        const int16_t y = (int16_t)(t * t * (target.y1 - 2 * target.yc + target.y2) + t * (-2 * target.y1 + 2 * target.yc) + target.y1);
        
        result.x1 = target.x1;
        result.y1 = target.y1;
        result.xc = xnewc;
        result.yc = ynewc;
        result.x2 = x;
        result.y2 = y;
    }

    void get_divided_bezier_r(bezier_t& __restrict target, bezier_t& __restrict result, float t){
        bezier_t b;
        b.x1 = target.x2;
        b.xc = target.xc;
        b.x2 = target.x1;
        b.y1 = target.y2;
        b.yc = target.yc;
        b.y2 = target.y1;
        get_divided_bezier(b, result, t);
    }
}

#endif