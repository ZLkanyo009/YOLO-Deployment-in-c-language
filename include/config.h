#ifndef __CONFIG_H
#define __CONFIG_H

#define INP_BUF_ADDR 0x00000000
#define INP_BUF_ADDR2 0x0000018c
#define KERNEL_BUF_ADDR 0x00000000
#define KERNEL_BUF_ADDR2  0x00000009 //9
#define BIAS_BUF_ADDR 0x00000000
#define BIAS_BUF_ADDR2 0x00000001 //256
#define OUR_BUF_ADDR 0x00000000
#define PSRAM_ADDR  0x00000000
#define PSRAM_ADDR2 0x0004b000    //307200 : 160×120×16
#define PSRAM_MAX 0x00800000 //由于本demo所有层都可以完整放在psram中，故不再考虑

#define NO_PADDING      0x00
#define PADDING_UP      0x01
#define PADDING_DOWN    0x02
#define PADDING_LEFT    0x04
#define PADDING_RIGHT   0x08

//#define DEBUG 1
//#define SET_SCALE_DEBUG 1
//#define INP_DEBUG 1
//#define NORM_DEBUG 1
//#define WI_DEBUG 1
//#define WEIGHT_DEBUG 1
//#define BIAS_DEBUG 1
//#define SET_TILE_DETAIL_DEBUG 1
//#define OUT_DEBUG 1
//#define OUT_DEBUG1 1
//#define OUT_DEBUG2 1
//#define OUT_DEBUG3 1
//#define OUT_DEBUG4 1
//#define OUT_DEBUG5 1
//#define POST_DEBUG
//#define CONF_DEBUG
//#define TXTYTWTH_DEBUG_ALL
//#define TXTYTWTH_DEBUG
//#define DRAW_DEBUG

//#define PSRAM_START_ADDR_DEBUG 1
//#define PSRAM_WI_DEBUG 1
//#define PSRAM_WI_DEBUG2 1

extern int *vga_bram;
extern const float anchor_size[][2];
extern const char stride;

#endif