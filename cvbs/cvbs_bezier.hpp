#include "cvbs_core.hpp"
#include "cvbs_draw.hpp"

#ifndef __NASTTY_CVBS_BEZIER_H__
#define __NASTTY_CVBS_BEZIER_H__

#include <cstdint>

namespace tvbezier{
    typedef struct bezier_s{
        int16_t x1;
        int16_t y1;
        int16_t xc;
        int16_t yc;
        int16_t x2;
        int16_t y2;
    } bezier_t;
    void bezier(int16_t x1, int16_t y1, int16_t xc, int16_t yc, int16_t x2, int16_t y2, uint16_t lines);
    
    void qbezier(const bezier_t& target, uint16_t lines);
    
    void get_divided_bezier(const bezier_t& __restrict target, bezier_t& __restrict result, float t);

    void get_divided_bezier_r(const bezier_t& __restrict target, bezier_t& __restrict result, float t);
}

#endif//__NASTTY_CVBS_BEZIER_H__