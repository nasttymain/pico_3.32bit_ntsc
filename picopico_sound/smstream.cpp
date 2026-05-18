/**
3-Clause BSD License

Copyright 2025 nasTTY

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#include "smstream.hpp"

static uint8_t __smmsg_error_reason;
uint8_t __get_smmsg_error_reason(){
    return __smmsg_error_reason;
}

static uint32_t __u8be_to_u32(const uint8_t* buf, const uint8_t len){
    uint32_t out = 0;
    for(uint8_t i = 0; i < len; i += 1){
        out = out * 256 + buf[i];
    }
    return out;
}

void __dump_smsdat(smsdat_t* dat){
    printf("--- smstream dump(start) smsdat\n");
    printf("fp %ld\n", (long)dat->fp);
    printf("is_valid_file %d\n", dat->is_valid_file);
    printf("is_supported_file %d\n", dat->is_supported_file);
    printf("header_chunk_len %d\n", dat->header_chunk_len);
    printf("format %d\n", dat->format);
    printf("track_count %d\n", dat->track_count);
    printf("time_base %d\n", dat->time_base);
    printf("track_len %d\n", dat->track_len);
    printf("reached_end %d\n", dat->reached_end);
    printf("--- smstream dump(finish)\n");
}

void __dump_smmsg(smmsg_t* msg){
    printf("DUMP smmsg: ");
    printf("@ %6d: %.2X %.2X %.2X ", msg->delta, msg->status, msg->param1, msg->param2);
    if(msg->status == 255){
        printf("[");
        for(uint8_t i = 0; i < msg->param2; i += 1){
            printf("%.2X ", msg->meta_data[i]);
        }
        printf("]");
    }
    if(msg->has_extra_data == 1){
        printf(" *ex");
    }
    printf("\n");
}

uint8_t mstream_set_file(smsdat_t* dat, FILE* f){
    if(dat == NULL){
        __smmsg_error_reason = 1;
        return 0;
    }
    uint8_t ubuf[4] = {0, 0, 0, 0};
    
    dat->fp = f;
    dat->is_valid_file = 0;
    dat->is_supported_file = 0;
    if(f == 0){
        __smmsg_error_reason = 2;
        return 0;
    }
    // MThd チェック
    fread(ubuf, sizeof(uint8_t), 4, dat->fp);
    if(memcmp(ubuf, (uint8_t*)"MThd", 4) == 0){
    }else{
        __smmsg_error_reason = 3;
        return 0;
    }
    dat->is_valid_file = 1;
    // ヘッダチャンク長さチェック
    fread(ubuf, sizeof(uint8_t), 4, dat->fp);
    dat->header_chunk_len = __u8be_to_u32(ubuf, 4);
    if(dat->header_chunk_len != 6){
        __smmsg_error_reason = 4;
        return 0;
    }
    // フォーマットチェック
    fread(ubuf, sizeof(uint8_t), 2, dat->fp);
    dat->format = __u8be_to_u32(ubuf, 2);
    if(dat->format != 0){
        __smmsg_error_reason = 5;
        return 0;
    }
    // トラック数チェック
    fread(ubuf, sizeof(uint8_t), 2, dat->fp);
    dat->track_count = __u8be_to_u32(ubuf, 2);
    if(dat->track_count != 1){
        __smmsg_error_reason = 6;
        return 0;
    }
    // タイムベース取得
    fread(ubuf, sizeof(uint8_t), 2, dat->fp);
    dat->time_base = __u8be_to_u32(ubuf, 2);
    // MTrk チェック
    fread(ubuf, sizeof(uint8_t), 4, dat->fp);
    if(memcmp(ubuf, (uint8_t*)"MTrk", 4) != 0){
        __smmsg_error_reason = 7;
        return 0;
    }
    // track 1 データ長チェック
    fread(ubuf, sizeof(uint8_t), 4, dat->fp);
    dat->track_len = __u8be_to_u32(ubuf, 4);
    dat->is_supported_file = 1;
    if(dat->track_len == 0){
        dat->reached_end = 1;
    }else{
        dat->reached_end = 0;
    }
    return 1;
}
uint8_t mstream_next_message(smmsg_t* msg, smsdat_t* dat){
    if(dat == NULL || msg == NULL){
        __smmsg_error_reason = 0;
        return 0;
    }
    if(dat->is_valid_file == 0 || dat->is_supported_file == 0 || dat->reached_end == 1){
        __smmsg_error_reason = 1;
        return 0;
    }
    msg->delta = 0;
    msg->has_extra_data = 0;
    uint8_t ubuf[4] = {0, 0, 0, 0};
    // タイムデルタを取得
    for(uint8_t i = 0; i < 4; i += 1){
        fread(&ubuf[0], sizeof(uint8_t), 1, dat->fp);
        msg->delta = (msg->delta) * 128 + (ubuf[0] & 127);
        if((ubuf[0] & 128) == 0){
            break;
        }
    }
    // ステータスバイト or ランニングステータスの第1バイト
    fread(ubuf, sizeof(uint8_t), 1, dat->fp);
    if((ubuf[0] & 128) == 128){
        // ステータスバイト
        msg->status = ubuf[0];
        fread(ubuf, sizeof(uint8_t), 1, dat->fp);
        msg->param1 = ubuf[0];
    }else{
        // ランニングステータスの第1バイト
        msg->param1 = ubuf[0];
    }
    
    // 第2バイト(プログラムチェンジ・チャンネルプレッシャーでは不要)
    if(msg->status / 16 != 12 && msg->status / 16 != 13){
        fread(ubuf, sizeof(uint8_t), 1, dat->fp);
        msg->param2 = ubuf[0];
    }else{
        msg->param2 = 0;
    }
    

    if((msg->status) & 128 != 128){
        // ステータスバイトが不正(例: unset)
        dat->is_valid_file = 0;
        __smmsg_error_reason = 3;
        return 0;
    }
    if((msg->status) == 255){
        // メタイベント。追加データがある
        if(msg->param2 >= 5){
            msg->has_extra_data = 1;
            for(uint16_t i = 0; i < msg->param2; i += 1){
                // 読み捨てる。fseek を使えや
                fread(ubuf, sizeof(uint8_t), 1, dat->fp);
            }
        }else{
            msg->has_extra_data = 0;
            fread(&(msg->meta_data[0]), sizeof(uint8_t), msg->param2, dat->fp);
        }
        if(msg->param1 == 0x2F && msg->param2 == 0x00){
            // トラック終端記号
            dat->reached_end = 1;
        }
    }else{
        // メタイベントではないので追加データをクリア
        msg->meta_data[0] = 0;
        msg->meta_data[1] = 0;
        msg->meta_data[2] = 0;
        msg->meta_data[3] = 0;
        msg->has_extra_data = 0;
    }
    return 1;
}


