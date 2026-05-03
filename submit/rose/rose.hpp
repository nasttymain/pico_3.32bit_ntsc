#include <math.h>
#include <stdint.h>
#include <rca.hpp> // ← ...?

extern int pattern_variation;


namespace rose{
    static uint32_t rnd(uint32_t m){
        return rand() % (m);
    }
    static uint8_t zoom = 100;
    float theta;
    float r;
    float x;
    float y;
    float xpast;
    float ypast;
    int16_t xbase = VIEWPORT_RES_X / 2;
    int16_t ybase = VIEWPORT_RES_Y / 2;
    uint8_t k = 6;
    uint8_t n = 5;
    uint32_t f = 0;
    
    uint16_t points = 200;
    
    constexpr const float pi = 3.14159265359;
    
    void rose_init(){
        f = 0;
    }
    void rose_frame(){
        f += 1;
        points = 64 + 32.0 * sinf(1.0 * f / 56);
        int v = pattern_variation % 3 + 1;
        for(uint_fast16_t i = 0; i < points; i += 1){
            theta = 2.0 * pi * k * i / points;
            r = sinf(theta * n / k) * zoom;
            x = xbase + r * cosf(theta);
            y = ybase + r * sinf(theta);
            if(i != 0){
                //color((49 * (pattern_variation + 1)) % 64);
                palcolor((((38 * pattern_variation + pattern_variation * pattern_variation) % 12) << 2) + v);
                line(xpast, ypast, x, y);
            }
            xpast = x;
            ypast = y;
        }
    }
}