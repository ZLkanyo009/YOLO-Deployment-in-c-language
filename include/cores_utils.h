#ifndef __CORES_UTILS_H
#define __CORES_UTILS_H

#include "simulator.h"

void pixel_norm_quantize(short int pixel_0bgr, const char scale_a_single,  char *pixel_all_channel_q_pointer);

bool pingpong_invert(bool pingpong);

void set_quantize_scale(const char scale_a_single_i, const char scale_w_single, const char scale_b_single, \
                        const char retune_single, const char scale_a_single_o);

void config_ctl_reg(bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                    bool pingpong_i, bool pingpong_w, bool pingpong_b, \
                    bool first_channel_group, bool last_channel_group, bool activ, bool pool);



#endif