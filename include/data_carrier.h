#ifndef __DATA_CARRIER_H
#define __DATA_CARRIER_H

#include "simulator.h"
#include "cores_utils.h"
#include "cores.h"

void camera_to_inpBuf(short int *camera_bram_pointer, int inp_buf_addr, \
                      const char TRow, const char TCol, \
                      int inp_w, \
                      const char scale_a_single, \
                      bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad);

void camera_to_inpBuf_test1(short int *camera_bram_pointer, int inp_buf_addr, \
                      const char TRow, const char TCol, \
                      int inp_w, \
                      const char scale_a_single, \
                      bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                      int tile_index);

void camera_to_inpBuf_test2(short int *camera_bram_pointer, int inp_buf_addr, \
                      const char TRow, const char TCol, \
                      int inp_w, \
                      const char scale_a_single, \
                      bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                      int tile_index);

void psram_to_inpBuf(int psram_addr, int inp_buf_addr, \
                     const char TRow, const char TCol, \
                     int inp_w, \
                     int Tn, int pre_Tm, \
                     bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad);

void psram_to_inpBuf_test1(int psram_addr, int inp_buf_addr, \
                     const char TRow, const char TCol, \
                     int inp_w, \
                     int Tn, int pre_Tm, \
                     bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                     int tile_index);

void psram_to_inpBuf_test2(int psram_addr, int inp_buf_addr, \
                     char TRow, char TCol, \
                     int inp_w, \
                     int Tn, \
                     bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                     int tile_index, int tile_remain_start_index, char layer_index);

void outBuf_to_psRam(int out_buf_addr, int psram_addr, \
                     int *tile_output_save_pointer,\
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc);

void outBuf_to_psRam_test(int out_buf_addr, int psram_addr, \
                     int *tile_output_save_pointer,\
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc, \
                     int tile_index, int kernel_group_index, char layer_index);

void outBuf_to_array(int out_buf_addr, char *net_output, int *tile_output_save_pointer, \
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc, \
                     int c_out);

void outBuf_to_array_test(int out_buf_addr, char *net_output, int *tile_output_save_pointer, \
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc, \
                     int c_out, int tile_index, int kernel_group_index, char layer_index);

#endif