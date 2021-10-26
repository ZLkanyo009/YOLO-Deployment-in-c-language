#include "data_loader.h"

void load_weight(char kernel_buf_addr, \
                 char *weight, char kernel_size, 
                 int c_in, int c_out){
    for(char pixel_index = 0; pixel_index < kernel_size * kernel_size; pixel_index++){
        //a[h + k*2 + j*2*2 + i*2*2*2]
        //printf("test: %d \n", (TM * TN * 8 / 32 - 1));
        write_kernel_buffer((uint32_t*) (weight + pixel_index * c_in * c_out), \
                            (uint32_t) (kernel_buf_addr + pixel_index), \
                            (int) (TM * TN * 8 / 32 - 1)); //-1是因为从0开始
        
        #ifdef WEIGHT_DEBUG
            for(int test_weight = 0; test_weight < 16*32; test_weight++){
                printf("weight : %d \n", *(weight + pixel_index * c_in * c_out + test_weight));
                printf("kbuf: %d \n", kbuf[pixel_index][test_weight]);
            }
        #endif
        
    }//需要更改！模拟器内是固定kernel channel=16，num=32的，所以对于第一层的权重来说，需要分开放
     //或者直接用python把weight.h改掉.
}

void load_bias(char bias_buf_addr, char *bias, char bias_num){
    write_bias_buffer((uint32_t*) bias, (uint32_t) bias_buf_addr, bias_num * 8 / 32 - 1);
    #ifdef BIAS_DEBUG
        for(int test_bias = 0; test_bias < bias_num; test_bias++){
            printf("bias : %d \n", *(bias + test_bias));
            printf("bbuf: %d \n", bbuf[0][test_bias]);
            printf("bbuf: %d \n\n", bias_num * 8 / 32 - 1);
        }
    #endif
}