#ifndef __DATA_LOADER_H
#define __DATA_LOADER_H

#include "simulator.h"

void load_weight(char kernel_buf_addr, \
                 char *weight, char kernel_size, 
                 int c_in, int c_out);

void load_bias(char bias_buf_addr, char *bias, char bias_num);

#endif