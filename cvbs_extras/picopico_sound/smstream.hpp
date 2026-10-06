/**
3-Clause BSD License

Copyright 2025 nasTTY

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#ifndef INCLUDE_SMSTREAMHPP
#define INCLUDE_SMSTREAMHPP

#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint8_t __get_smmsg_error_reason();

typedef struct type_smsdat{
    FILE* fp;
    uint8_t  is_valid_file;
    uint8_t  is_supported_file;
    uint8_t  header_chunk_len;
    uint8_t  format;
    uint8_t  track_count;
    uint32_t time_base;
    uint32_t track_len;
    uint8_t  reached_end;
} smsdat_t;

typedef struct type_smmsg{
    uint32_t delta;
    uint8_t  status;
    uint8_t  param1;
    uint8_t  param2;
    uint8_t  meta_data[4];
    uint8_t  has_extra_data;
} smmsg_t;

static uint32_t __u8be_to_u32(const uint8_t* buf, const uint8_t len);
void __dump_smsdat(smsdat_t* dat);
void __dump_smmsg(smmsg_t* msg);
uint8_t mstream_set_file(smsdat_t* dat, FILE* f);
uint8_t mstream_next_message(smmsg_t* msg, smsdat_t* dat);

#endif