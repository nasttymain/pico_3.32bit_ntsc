#ifndef __PICOPICO_HPP__
#define __PICOPICO_HPP__

#include "picopico.pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"

#define PIO_PICOPICO pio1

// for RP2040 Compatibility... (I did not verify)
#define IRQ_PICOPICO_MIDI PIO1_IRQ_0

#include <cstdint>

namespace picopico{
    // ピコピコソフトウェア音源
    // mode
    // 0: 8和音モード
    uint8_t ss_mode = 0;

    constexpr const uint8_t CH_COUNT = 8;
    // 1出力サンプル毎に進む位相カウント数。
    uint32_t ss_phase_step          [CH_COUNT];
    // チャンネルボリューム。0・1・2 の 3段階
    uint8_t  ss_volume              [CH_COUNT];
    // 発音開始からのサンプル数
    uint16_t ss_smps_since_toneon   [CH_COUNT];
    // オートオフするサンプル数。0で指定解除
    uint16_t ss_smps_auto_toneoff   [CH_COUNT];
    // 音色番号。
    // 0: 50 Square
    // 1: Triangle
    // 2: LFSR
    uint8_t  ss_program             [CH_COUNT];
    // 発音中かどうか。0/1
    uint8_t  ss_toneon              [CH_COUNT];
    // 現在の位相カウント。これが 1 周すると音の 1 周期が出力されたことになる。2^28 で1周。
    uint32_t ss_phase_current       [CH_COUNT];
    
    // スイープ設定。スイープとは、ピッチのエンベロープみたいなもの
    uint8_t  ss_sweep_avail         [CH_COUNT];
    uint8_t  ss_sweep_running       [CH_COUNT];
     int8_t  ss_sweep_amount        [CH_COUNT];
    uint32_t ss_sweep_until         [CH_COUNT];
    
    // LFSR
    uint16_t lfsr = 0xACE1u;
    inline uint lfsr_step_and_get(){
        lfsr = (lfsr >> 1) ^ (-(uint16_t)(lfsr & 1u) & 0xB400u); 
        return lfsr & 1;
    }
    inline uint lfsr_get(){
        return lfsr & 1;
    }
    
    constexpr const uint32_t samp2mem_patn[33] = {
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000001,
    0b00000000000000010000000000000001,
    0b00000000001000000000010000000001,
    0b00000001000000010000000100000001,
    0b00000010000010000001000001000001,
    0b00000100001000010000010000100001,
    0b00001000010001000010001000010001,
    0b00010001000100010001000100010001,
    0b00010001001000100100010010001001,
    0b00010010010010010001001001001001,
    0b00100100100100100100100100100101,
    0b00100101001001010010010100100101,
    0b00101001010010100101001010010101,
    0b00101010010101010010101001010101,
    0b00101010101010100101010101010101,
    0b01010101010101010101010101010101,
    0b01010101010101011010101010101011,
    0b01010101101010110101010110101011,
    0b01010110101101011010110101101011,
    0b01011011010110110101101101011011,
    0b01011011011011011011011011011011,
    0b01101101101101110110110110110111,
    0b01101110110111011011101101110111,
    0b01110111011101110111011101110111,
    0b01110111101110111101110111101111,
    0b01111011110111110111101111011111,
    0b01111101111101111110111110111111,
    0b01111111011111110111111101111111,
    0b01111111110111111111101111111111,
    0b01111111111111110111111111111111,
    0b01111111111111111111111111111111,
    0b11111111111111111111111111111111
    };
    
    void init(){
        for(uint ch = 0; ch < CH_COUNT; ch += 1){
            ss_toneon[ch] = 0;
            ss_program[ch] = 0;
            ss_volume[ch] = 1;
            ss_sweep_avail[ch] = 0;
            ss_smps_since_toneon[ch] = 0;
        }
    }
    
    void feed(uint32_t* buf){
        for(uint i = 0; i < 64; i += 1){
            int_fast16_t outsamp = 0;
            for(uint ch = 0; ch < CH_COUNT; ch += 1){
                int_fast16_t ch_outsamp = 0;
                // start
                if(ss_toneon[ch] == 1){
                    // start 発音中
                    const auto ss_phase_previous = ss_phase_current[ch];
                    ss_phase_current[ch] += ss_phase_step[ch];
                    ss_smps_since_toneon[ch] += 1;
                    if(ss_smps_auto_toneoff[ch] <= ss_smps_since_toneon[ch] && ss_smps_auto_toneoff[ch] != 0){
                        ss_toneon[ch] = 0;
                    }
                    if(ss_sweep_avail[ch] == 1 && ss_sweep_running[ch] == 1){
                        ss_phase_step[ch] += ss_sweep_amount[ch];
                        if((ss_sweep_amount[ch] > 0 && ss_phase_step[ch] >= ss_sweep_until[ch]) || (ss_sweep_amount[ch] < 0 && ss_phase_step[ch] <= ss_sweep_until[ch])){
                            ss_sweep_running[ch] = 0;
                            ss_phase_step[ch] = ss_sweep_until[ch];
                        }
                    }
                    if(ss_program[ch] == 0){
                        // Square 50
                        if(ss_phase_current[ch] & 8388608){
                            ch_outsamp =  1 * ss_volume[ch];
                        }else{
                            ch_outsamp = -1 * ss_volume[ch];
                        }
                    }else if(ss_program[ch] == 1){
                        // bad triangle
                            const uint p = (ss_phase_current[ch] / 2097152) & 7;
                            constexpr const int wt[8] = {0, 1, 2, 1, 0, -1, -2, -1};
                            ch_outsamp =  wt[p] * (ss_volume[ch] >> 1);
                    }else if(ss_program[ch] == 2){
                        // LFSR
                        if((ss_phase_previous & 1048576) != (ss_phase_current[ch] & 1048576)){
                            ch_outsamp = (lfsr_step_and_get() == 1) ? (ss_volume[ch]) : (-ss_volume[ch]);
                        }else{
                            ch_outsamp = (         lfsr_get() == 1) ? (ss_volume[ch]) : (-ss_volume[ch]);
                        }
                    }
                    
                    
                    //   end 発音中
                }
                // end
                outsamp += ch_outsamp;
            }
            if(outsamp < -16){
                outsamp = -16;
            }else if(outsamp > 16){
                outsamp = 16;
            }
            buf[i] = samp2mem_patn[outsamp + 16];
        }
    }
    //
    //
    constexpr const uint32_t thtable[12] = {
        2194675,
        2325176,
        2463438,
        2609922,
        2765116,
        2929539,
        3103738,
        3288296,
        3483829,
        3690988,
        3910465,
        4142993
    };
    
    // bend: -32 ～ 32
    inline uint32_t tone2step(uint8_t t, int32_t bend){
        if(bend == 0){
            const uint oct = t / 12;
            return thtable[(t % 12)] >> (8 - oct);
        }
        const uint td = (bend > 0) ? t : t - 1;
        const uint tu = (bend > 0) ? t + 1 : t;
        const uint bp = (bend > 0) ? bend : (32 + bend);
        const uint octd = td / 12;
        const uint octu = tu / 12;
        const uint d = thtable[(td % 12)] >> (8 - octd);
        const uint u = thtable[(tu % 12)] >> (8 - octu);
        return (d * (32 - bp) + u * bp) / 32;
    }
    void toneon(uint8_t channel, uint8_t tone_height, int32_t pitch_bend){
        ss_phase_step[channel] = tone2step(tone_height, pitch_bend);
        ss_smps_since_toneon[channel] = 0;
        ss_toneon[channel] = 1;
        if(ss_sweep_avail[channel] == 1){
            ss_sweep_running[channel] = 1;
        }
    }
    void toneoff(uint8_t channel){
        ss_toneon[channel] = 0;
        ss_sweep_running[channel] = 0;
    }
    void set_channel_volume(uint8_t channel, uint8_t volume){
        ss_volume[channel] = (volume > 8) ? 8 : volume;
    }
    void set_channel_program(uint8_t channel, uint8_t program){
        ss_program[channel] = program;
    }
    
    void sfx_kick(uint8_t channel){
        set_channel_volume(channel, 4);
        set_channel_program(channel, 1);
        ss_sweep_avail[channel] = 1;
        ss_sweep_amount[channel] = -80;
        ss_sweep_until[channel] = 100;
        ss_smps_auto_toneoff[channel] = 32 * 75;
        toneon(channel, 43, 0);
    }
    
    void sfx_snare(uint8_t channel){
        set_channel_volume(channel, 1);
        set_channel_program(channel, 2);
        ss_sweep_avail[channel] = 1;
        ss_sweep_amount[channel] = -40;
        ss_sweep_until[channel] = tone2step(53, 0);
        ss_smps_auto_toneoff[channel] = 32 * 70;
        toneon(channel, 60, 0);
    }
    
    void sfx_hat(uint8_t channel){
        set_channel_volume(channel, 1);
        set_channel_program(channel, 2);
        ss_sweep_avail[channel] = 1;
        ss_sweep_amount[channel] = -80;
        ss_sweep_until[channel] = tone2step(70, 0);
        ss_smps_auto_toneoff[channel] = 32 * 65;
        toneon(channel, 86, 0);
    }
}


void picopico_program_init(PIO pio, uint sm, uint offset, uint pin) {
   pio_gpio_init(pio, pin);
   pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
   pio_sm_config c = picopico_program_get_default_config(offset);
   sm_config_set_out_pins(&c, pin, 1);
   sm_config_set_out_shift(&c, true, true, 32);
   pio_sm_init(pio, sm, offset, &c);
}

namespace picoaout{
    PIO pio = PIO_PICOPICO;
    uint sm;
    dma_channel_config dc;
    int dma_chan;
    constexpr const uint8_t PICOPICO_START_PIN = 20;
    constexpr const int ADMABUF_LEN = 64;
    uint32_t ADMABUF[ADMABUF_LEN * 2];
    uint32_t* const ptr_adma[2] = {&ADMABUF[0], &ADMABUF[ADMABUF_LEN]};
    volatile uint flip = 0;
    volatile uint irqcounter = 0;
    volatile uint debug_irq_c = 0;
    volatile uint8_t midi_irq_setup = 0;
    
    void hndirq0(void){
        dma_hw->ints0 = 1u << dma_chan;
        dma_channel_abort(dma_chan);
        dma_channel_set_read_addr(dma_chan, ptr_adma[flip], false);
        dma_channel_set_trans_count(dma_chan, ADMABUF_LEN, false);
        dma_channel_start(dma_chan);
        flip = (flip + 1) % 2;
        debug_irq_c += 1;
        irqcounter += 1;
        
        // construct buffer
        uint32_t* const next_buff = ptr_adma[flip];
        picopico::feed(next_buff);
        
        if(midi_irq_setup == 1){
            irq_set_pending(IRQ_PICOPICO_MIDI);
        }
    }

    void init(){
        uint offset = pio_add_program(pio, &picopico_program);
        sm = pio_claim_unused_sm(pio, true);
        picopico_program_init(pio, sm, offset, PICOPICO_START_PIN);
        // 量子化ビット数 5 (← im sad)、サンプリング周波数 32000.0 Hz
        pio_sm_set_clkdiv(pio, sm, 153.80859375);
        pio_sm_set_enabled(pio, sm, true);

        dma_chan = dma_claim_unused_channel(true);
        dc = dma_channel_get_default_config(dma_chan);
        channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
        channel_config_set_dreq(&dc, pio_get_dreq(pio, sm, true));
        channel_config_set_read_increment(&dc, true);
        channel_config_set_write_increment(&dc, false);
        channel_config_set_high_priority(&dc, false);
        dma_channel_configure(
            dma_chan,
            &dc,
            &pio->txf[sm],
            ADMABUF,
            ADMABUF_LEN,
            true
        );
        dma_channel_set_irq1_enabled(dma_chan, true);
        irq_set_exclusive_handler(DMA_IRQ_1, hndirq0);
        // Middle Priority
        irq_set_priority(DMA_IRQ_1, 0xC0);
        irq_set_enabled(DMA_IRQ_1, true);
    }

}

#ifndef PICOPICO_NO_MIDI
    #include "smstream.cpp"
    namespace picopicomidi{
        smsdat_t smf_data;
        volatile smmsg_t  msg;
        constexpr const uint8_t CH_COUNT = 6;
        uint8_t last_th[CH_COUNT];
        int8_t last_bend[CH_COUNT];
        
        // 120BPM、96Delta/Beat
        uint8_t stamp256_by_irq = 96 * 120 * 256 / 30000;
        uint8_t is_playing = 0;
        uint32_t smf_last_timestamp = 0;
        
        // 256掛け
        uint32_t currentstamp256 = 0;
        void hnd_midi_irq(){
            if(is_playing != 1){
                return;
            }
            if(smf_data.is_valid_file == 1 && smf_data.is_supported_file == 1 && smf_data.reached_end == 0){
                
            }else{
                return;
            }
            currentstamp256 += stamp256_by_irq;
            while(1){
                if(smf_data.reached_end == 1){
                    for(uint i = 0; i < CH_COUNT; i += 1){
                        picopico::toneoff(i);
                    }
                    break;
                }
                if((currentstamp256 / 256) >= smf_last_timestamp + msg.delta){
                    // 再生する
                    smf_last_timestamp += msg.delta;
                    const uint8_t ch = msg.status & 0x0F;
                    const uint8_t st = msg.status & 0xF0;
                    if(st == 0x90 && msg.param2 != 0){
                        // ノートオン
                        picopico::set_channel_volume(ch, (msg.param2 / 43));
                        last_th[ch] = msg.param1 - 12;
                        picopico::toneon(ch, last_th[ch], last_bend[ch]);
                    }else if(st == 0x80 || (st == 0x90 && msg.param2 == 0)){
                        if(last_th[ch] == msg.param1 - 12){
                            // ノートオフ
                            picopico::toneoff(ch);
                            last_th[ch] = 128;
                        }
                    }else if(st == 0xE0){
                        if(last_th[ch] != 128){
                            last_bend[ch] = (msg.param2 >> 1) - 32;
                            picopico::toneon(ch, last_th[ch], last_bend[ch]);
                        }
                    }else if(msg.status == 0xFF){
                        // メタ
                        if(msg.param1 == 0x51){
                            const uint32_t beat_ms = 65536 * msg.meta_data[0] + 256 * msg.meta_data[1] + msg.meta_data[2];
                            stamp256_by_irq = 256 * smf_data.time_base * 2 * 1000 / beat_ms;
                        }
                    }
                    mstream_next_message((smmsg_t*)(&msg), &smf_data);
                }else{
                    break;
                }
            }
        }
        
        void init(){
            irq_set_exclusive_handler(IRQ_PICOPICO_MIDI, hnd_midi_irq);
            irq_set_priority(DMA_IRQ_1, 0xFF);
            irq_set_enabled(IRQ_PICOPICO_MIDI, true);
            msg.delta = 0;
            picoaout::midi_irq_setup = 1;
        }
        
        uint8_t load_from_file(FILE* f){
            mstream_set_file(&smf_data, f);
            if(smf_data.is_valid_file == 1 && smf_data.is_supported_file == 1 && smf_data.reached_end == 0){
                stamp256_by_irq = smf_data.time_base * 120 * 256 / 30000;
                smf_last_timestamp = 0;
                for(uint i = 0; i < CH_COUNT; i += 1){
                    last_th[i] = 128;
                    last_bend[i] = 0;
                }
                is_playing = 0;
                return 1;
            }else{
                return 0;
            }
        }
    }
#endif

void picopico_all_init(){
    picopico::init();
    picoaout::init();
    #ifndef PICOPICO_NO_MIDI
        picopicomidi::init();
    #endif
}

#endif