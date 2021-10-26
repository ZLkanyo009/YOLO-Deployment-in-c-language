#ifndef SIMULATOR_H 
#define SIMULATOR_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define TM 32
#define TN 16
#define TR 16
#define TC 20
#define TROW 18
#define TCOL 22
#define KERNEL 3

#define NO_PADDING        0x00
#define PADDING_UP         0x01
#define PADDING_DOWN  0x02
#define PADDING_LEFT      0x04
#define PADDING_RIGHT   0x08


char ibuf[TROW*TCOL*2][TN];

char kbuf[KERNEL*KERNEL*2][TM*TN];

short psbuf[TC*TR][TM];

char bbuf[2][TM];

char obuf[TC*TR][TM];

char psram[8388608];


#define write_input_buffer(p, addr, size) memcpy(ibuf[addr], p, (size+1)*4)

#define write_kernel_buffer(p, addr, size) memcpy(kbuf[addr], p, (size+1)*4)

#define write_bias_buffer(p, addr, size) memcpy(bbuf[addr], p, (size+1)*4)

#define read_output_buffer(p, addr, size) memcpy(p, obuf[addr], (size+1)*4)

#define write_psram(p, addr, size) memcpy(psram + addr, p, (size+1))

#define read_psram(p, addr, size) memcpy(p, psram + addr, (size+1))

char busy;

typedef struct tile_ofs{
    char input_ofs;
    char input_dir;
    char bias_ofs;
    char bias_dir;
    char output_ofs;
    char output_dir;
} tile_ofs;

typedef struct tile_info{
    char tn;
    char tm;
    char tc;
    char tr;
} tile_info;

typedef struct tile_detail{
    char pad_pos;
    bool pingpong_i;
    bool pingpong_k;
    bool pingpong_b;
    bool first;
    bool last;
    bool activ;
    bool pool;
} tile_detail;



tile_ofs global_tile_ofs;
tile_info global_tile_info;
tile_detail global_tile_detail;


void set_offset(char iofs, char idir, char bofs, char bdir, char oofs, char odir);
void set_tile_detail(char pad_pos, bool pingpong_i, bool pingpong_k, bool pingpong_b, bool first, bool last, bool activ, bool pool);
void set_tile_info(char tn, char tm, char tc, char tr);
void start_calculate(int tile_index, int layer_index);
void wait_cal_done();

#endif