#ifndef __CORES_H
#define __CORES_H

#include <stdbool.h>
#include "weight.h"
#include "config.h"
#include "data_carrier.h"
#include "data_loader.h"
#include "simulator.h"

struct INFO{
    int psram_addr;
    int out_w;
    char Tm;
    char out_Tr;
    char out_Tc;
};

struct INFO first_conv(int inp_w, int inp_h, \
                       char Tr, char Tc, char Tm, char Tn, \
                       char TRow, char TCol, char layer_index, \
                       short int *camera_bram_pointer, int *tile_output_save_pointer, \
                       char *b_conv, char *w_conv, \
                       int psram_addr, \
                       bool activ, bool pool, \
                       bool pingpong[], \
                       char kernel_size);

struct INFO second_conv(int inp_w, int inp_h, \
                        char Tr, char Tc, char Tm, char Tn, \
                        char TRow, char TCol, char layer_index, \
                        int *tile_output_save_pointer, \
                        char *b_conv, char *w_conv, \
                        int psram_in_addr, int psram_out_addr, \
                        struct INFO pre_layer_info, \
                        bool activ, bool pool, \
                        bool pingpong[], \
                        char kernel_size);


struct INFO conv_normal(int inp_w, int inp_h, \
                        int c_in, int c_out, \
                        char Tr, char Tc, char Tm, char Tn, \
                        char TRow, char TCol, char layer_index, \
                        int *tile_output_save_pointer, \
                        char *b_conv, char *w_conv, \
                        int psram_in_addr2d, int psram_out_addr2d, struct INFO pre_layer_info, \
                        bool activ, bool pool, \
                        bool pingpong[], \
                        char kernel_size);


struct INFO conv_last(int inp_w, int inp_h, \
                      int c_in, int c_out, \
                      char Tr, char Tc, char Tm, char Tn, \
                      char TRow, char TCol, char layer_index, \                        
                      int *tile_output_save_pointer, \
                      char *b_conv, char *w_conv, \
                      int psram_in_addr2d, char *net_output, struct INFO pre_layer_info, \
                      bool activ, bool pool, \
                      bool pingpong[], \
                      char kernel_size);

#endif