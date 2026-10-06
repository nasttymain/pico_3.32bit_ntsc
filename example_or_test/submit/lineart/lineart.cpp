static constexpr const uint8_t MAX_LINES = 127;
static uint8_t lines;
static int16_t x[MAX_LINES * 2];
static int16_t y[MAX_LINES * 2];
static int16_t xs[2];
static int16_t ys[2];


namespace lineart{
    void lineart_init(){
        lines = 64;
        x[0] = 20;
        y[0] = 90;
        x[1] = 160;
        y[1] = 60;
        xs[0] = -4;
        ys[0] = -4;
        xs[1] = -4;
        ys[1] = -4;
        for(uint8_t i = 1; i < MAX_LINES; i += 1){
            x[i * 2 + 0] = 0;
            y[i * 2 + 0] = 0;
            x[i * 2 + 1] = 0;
            y[i * 2 + 1] = 0;
        }
    }
    void lineart_frame(){
        for(uint_fast8_t i = 0; i < 2; i += 1){
            x[i] += xs[i];
            y[i] += ys[i];
            if(x[i] <= 0 && xs[i] <= 0){
                xs[i] = -xs[i];
            }
            if(y[i] <= 0 && ys[i] <= 0){
                ys[i] = -ys[i];
            }
            if(x[i] >= VIEWPORT_RES_X && xs[i] >= 0){
                xs[i] = -xs[i];
            }
            if(y[i] >= VIEWPORT_RES_Y && ys[i] >= 0){
                ys[i] = -ys[i];
            }
        }

        for(uint_fast8_t cnt = 0; cnt < lines - 1; cnt += 1){
            uint_fast8_t lcnt = lines - 1 - cnt;
            x[lcnt * 2 + 0] = x[lcnt * 2 - 2];
            y[lcnt * 2 + 0] = y[lcnt * 2 - 2];
            x[lcnt * 2 + 1] = x[lcnt * 2 - 1];
            y[lcnt * 2 + 1] = y[lcnt * 2 - 1];
        }
        for(uint_fast8_t cnt = 0; cnt < lines; cnt += 1){
            uint_fast8_t lcnt = lines - 1 - cnt;
            uint_fast8_t n = 3 - lcnt * 3 / lines;
            palcolor((0xC << 2) + n);
            line(x[lcnt * 2 + 0], y[lcnt * 2 + 0], x[lcnt * 2 + 1], y[lcnt * 2 + 1]);
        }
    }
}