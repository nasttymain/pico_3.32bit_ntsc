#include "rca.hpp"

#ifndef __NASTTY_RCA_DIMZ__
#define __NASTTY_RCA_DIMZ__

#include <cstdint>
#include <cmath>
#include <cstdlib>

namespace tvdimz{
    typedef struct point_s{
        float x;
        float y;
        float z;
    }p_t;

    typedef struct vector_s{
        p_t p1;
        p_t p2;
    }vec_t;

    typedef struct triangle_s{
        p_t p1;
        p_t p2;
        p_t p3;
    }tri_t;
    
    float xyscale = 16.0f;
    
    
    inline int32_t to_screen_x(const p_t& p){
        return 180 + (int32_t)(p.x * xyscale / sqrtf(p.z));
    }
    
    inline int32_t to_screen_y(const p_t& p){
        return 116 + (int32_t)(p.y * xyscale / sqrtf(p.z));
    }
    
    void set_vec(vec_t& vec, float x1, float y1, float z1, float x2, float y2, float z2){
        vec.p1.x = x1;
        vec.p1.y = y1;
        vec.p1.z = z1;
        vec.p2.x = x2;
        vec.p2.y = y2;
        vec.p2.z = z2;
    }
    
    void draw_vec(const vec_t& vec){
        if(vec.p1.z <= 0.5 || vec.p2.z <= 0.5){
            return;
        }
        const int32_t x1 = to_screen_x(vec.p1);
        const int32_t y1 = to_screen_y(vec.p1);
        const int32_t x2 = to_screen_x(vec.p2);
        const int32_t y2 = to_screen_y(vec.p2);
        if(
            x1 < -16384 || x1 >= 16384
            ||
            y1 < -16384 || y1 >= 16384
            ||
            x2 < -16384 || x2 >= 16384
            ||
            y2 < -16384 || y2 >= 16384
        ){
            return;
        }
        line((int16_t)x1, (int16_t)y1, (int16_t)x2, (int16_t)y2);
    }
    
    void translate_vec(vec_t& vec, float xa, float ya, float za){
        vec.p1.x += xa;
        vec.p1.y += ya;
        vec.p1.z += za;
        vec.p2.x += xa;
        vec.p2.y += ya;
        vec.p2.z += za;
    }
    
    inline void xyrotate(p_t& p, float theta){
        auto const pastx = p.x;
        auto const pasty = p.y;
        p.x = pastx * cosf(theta) - pasty * sinf(theta);
        p.y = pasty * cosf(theta) + pastx * sinf(theta);
    }
    
    inline void xzrotate(p_t& p, float theta){
        auto const pastx = p.x;
        auto const pastz = p.z;
        p.x = pastx * cosf(theta) - pastz * sinf(theta);
        p.z = pastz * cosf(theta) + pastx * sinf(theta);
    }
    
    inline void yzrotate(p_t& p, float theta){
        auto const pasty = p.y;
        auto const pastz = p.z;
        p.y = pasty * cosf(theta) - pastz * sinf(theta);
        p.z = pastz * cosf(theta) + pasty * sinf(theta);
    }
    
    inline void xyrotate_vec(vec_t& vec, float theta){
        xyrotate(vec.p1, theta);
        xyrotate(vec.p2, theta);
    }
    
    inline void xzrotate_vec(vec_t& vec, float theta){
        xzrotate(vec.p1, theta);
        xzrotate(vec.p2, theta);
    }
    
    inline void yzrotate_vec(vec_t& vec, float theta){
        yzrotate(vec.p1, theta);
        yzrotate(vec.p2, theta);
    }
    
    class Wiremodel{
        public:
            Wiremodel(int32_t max_size);
            ~Wiremodel();
            void append(const vec_t& vec);
            void draw();
            void translate(float xa, float ya, float za);
            void xyrotate(float angle);
            void xzrotate(float angle);
            void yzrotate(float angle);
            void load_model(const float dat[], int32_t vec_count);
            int32_t capacity;
            int32_t size;
            vec_t* vecs;
    };
    
    Wiremodel::Wiremodel(int32_t max_size){
        capacity = max_size;
        size = 0;
        vecs = (vec_t*)malloc(max_size * sizeof(vec_t));
    }
    
    Wiremodel::~Wiremodel(){
        free(vecs);
    }
    
    void Wiremodel::append(const vec_t& vec){
        if(capacity <= size){
            return;
        }
        vecs[size].p1.x = vec.p1.x;
        vecs[size].p1.y = vec.p1.y;
        vecs[size].p1.z = vec.p1.z;
        vecs[size].p2.x = vec.p2.x;
        vecs[size].p2.y = vec.p2.y;
        vecs[size].p2.z = vec.p2.z;
        size += 1;
    }
    
    void Wiremodel::draw(){
        for(int32_t i = 0; i < size; i += 1){
            draw_vec(vecs[i]);
        }
    }
    
    void Wiremodel::translate(float xa, float ya, float za){
        for(int32_t i = 0; i < size; i += 1){
            translate_vec(vecs[i], xa, ya, za);
        }
    }
    
    void Wiremodel::xyrotate(float angle){
        for(int32_t i = 0; i < size; i += 1){
            xyrotate_vec(vecs[i], angle);
        }
    }
    
    void Wiremodel::xzrotate(float angle){
        for(int32_t i = 0; i < size; i += 1){
            xzrotate_vec(vecs[i], angle);
        }
    }
    
    void Wiremodel::yzrotate(float angle){
        for(int32_t i = 0; i < size; i += 1){
            yzrotate_vec(vecs[i], angle);
        }
    }
    
    void Wiremodel::load_model(const float dat[], int32_t vec_count){
        size = 0;
        const int32_t max_vec = (vec_count < capacity) ? vec_count : capacity;
        for(int32_t i = 0; i < max_vec; i += 1){
            vecs[i].p1.x = dat[6 * i + 0];
            vecs[i].p1.y = dat[6 * i + 1];
            vecs[i].p1.z = dat[6 * i + 2];
            vecs[i].p2.x = dat[6 * i + 3];
            vecs[i].p2.y = dat[6 * i + 4];
            vecs[i].p2.z = dat[6 * i + 5];
            size += 1;
        }
    }
    
    void wirecube(Wiremodel& model, float edge_len){
        const float el = edge_len / 2;
        model.size = 0;

        model.append(vec_t{{-el, -el, -el},{-el,  el, -el}});
        model.append(vec_t{{-el, -el, -el},{ el, -el, -el}});
        model.append(vec_t{{-el,  el, -el},{ el,  el, -el}});
        model.append(vec_t{{ el, -el, -el},{ el,  el, -el}});
        model.append(vec_t{{-el, -el,  el},{-el,  el,  el}});
        model.append(vec_t{{-el, -el,  el},{ el, -el,  el}});
        model.append(vec_t{{-el,  el,  el},{ el,  el,  el}});
        model.append(vec_t{{ el, -el,  el},{ el,  el,  el}});
        model.append(vec_t{{-el, -el, -el},{-el, -el,  el}});
        model.append(vec_t{{ el, -el, -el},{ el, -el,  el}});
        model.append(vec_t{{-el,  el, -el},{-el,  el,  el}});
        model.append(vec_t{{ el,  el, -el},{ el,  el,  el}});
    }
    
}

#endif