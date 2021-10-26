#include "data_carrier.h"

void camera_to_inpBuf(short int *camera_bram_pointer, int inp_buf_addr, \
                      const char TRow, const char TCol, \
                      int inp_w, \
                      const char scale_a_single, \
                      bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad){
    char TRow_no_pad = TRow;
    char TCol_no_pad = TCol;
    
    int pixel_all_channel_q = 0;
    char *pixel_all_channel_q_pointer = &pixel_all_channel_q;
    //printf("pixel_all_channel_q_pointer!!! : %d \n",  *pixel_all_channel_q_pointer);
    //col 22 
    //row 18
    //最后几层的时候会出现上下都有pad
    if(is_up_pad == 1){TRow_no_pad -= 1;}
    if(is_down_pad == 1){TRow_no_pad -= 1;}
    if(is_left_pad == 1){TCol_no_pad -= 1;}
    if(is_right_pad == 1){TCol_no_pad -= 1;}
    
    //如果是上padding, inp_buf的地址需要在一开始多加一行
    //如果是下padding, 直接使用原地址即可，因为循环到最后时不会去写最后一行的inp_buf
    if(is_up_pad == 1){inp_buf_addr += TCol;}
    for(char h = 0; h < TRow_no_pad; h++){
        if(is_left_pad == 1){inp_buf_addr += 1;} //可以尝试使用这个代码
        for(char w = 0; w < TCol_no_pad; w ++){
            //如果是左padding, inp_buf的地址需要在每行开头多加1
            //如果是右padding, 直接使用原地址即可，因为循环到最后时不会去写最后一个的inp_buf
            //if(is_left_pad == 1 && (inp_buf_addr - inp_buf_start) % TCol == 0){inp_buf_addr += 1;}

            pixel_norm_quantize(*camera_bram_pointer, scale_a_single, pixel_all_channel_q_pointer);            
            write_input_buffer((uint32_t *) pixel_all_channel_q_pointer, (uint32_t) (inp_buf_addr), 0);
            
            inp_buf_addr += 1;
            camera_bram_pointer += 1;//地址+2，转跳到下一个像素位置

        }
        if(is_right_pad == 1){inp_buf_addr += 1;}
        camera_bram_pointer += (inp_w - TCol_no_pad);
    }
}

void camera_to_inpBuf_test1(short int *camera_bram_pointer, int inp_buf_addr, \
                      const char TRow, const char TCol, \
                      int inp_w, \
                      const char scale_a_single, \
                      bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                      int tile_index){
    char TRow_no_pad = TRow;
    char TCol_no_pad = TCol;

    #ifdef WI_DEBUG
        int inp_buf_start = inp_buf_addr;
        int test = 0;
        int bak1 = 0, bak2 = 0, bak3 = 0;
        if(inp_buf_addr == INP_BUF_ADDR2){test = 1;}
    #endif
    
    int pixel_all_channel_q = 0;
    char *pixel_all_channel_q_pointer = &pixel_all_channel_q;
    //printf("pixel_all_channel_q_pointer!!! : %d \n",  *pixel_all_channel_q_pointer);
    //col 22 
    //row 18
    //最后几层的时候会出现上下都有pad
    if(is_up_pad == 1){TRow_no_pad -= 1;}
    if(is_down_pad == 1){TRow_no_pad -= 1;}
    if(is_left_pad == 1){TCol_no_pad -= 1;}
    if(is_right_pad == 1){TCol_no_pad -= 1;}
    
    //如果是上padding, inp_buf的地址需要在一开始多加一行
    //如果是下padding, 直接使用原地址即可，因为循环到最后时不会去写最后一行的inp_buf
    if(is_up_pad == 1){inp_buf_addr += TCol;}
    for(char h = 0; h < TRow_no_pad; h++){
        if(is_left_pad == 1){inp_buf_addr += 1;} //可以尝试使用这个代码
        for(char w = 0; w < TCol_no_pad; w ++){
            //如果是左padding, inp_buf的地址需要在每行开头多加1
            //如果是右padding, 直接使用原地址即可，因为循环到最后时不会去写最后一个的inp_buf
            //if(is_left_pad == 1 && (inp_buf_addr - inp_buf_start) % TCol == 0){inp_buf_addr += 1;}

            pixel_norm_quantize(*camera_bram_pointer, scale_a_single, pixel_all_channel_q_pointer);            
            write_input_buffer((uint32_t *) pixel_all_channel_q_pointer, (uint32_t) (inp_buf_addr), 0);
            
            #ifdef WI_DEBUG
                // int flag = 0;
                // if(1){ // 7 8 tile_index == 16 * (n-1) + (m-1)
                //     if (bak1 != ibuf[inp_buf_addr][0]){flag = 1;bak1 = ibuf[inp_buf_addr][0];printf("ibuf1 :%d \n", bak1);}
                //     if (bak2 != ibuf[inp_buf_addr][1]){flag = 1;bak2 = ibuf[inp_buf_addr][1];printf("ibuf2 :%d \n", bak2);}
                //     if (bak3 != ibuf[inp_buf_addr][2]){flag = 1;bak3 = ibuf[inp_buf_addr][2];printf("ibuf3 :%d \n", bak3);}
                //     if (flag == 1){
                //         printf("tile index: %d \n", tile_index);
                //         printf("h: %d \n", h); 
                //         printf("w: %d \n", w); 
                //         printf("up: %d, down: %d, left: %d, right: %d \n\n",is_up_pad,  is_down_pad,  is_left_pad,  is_right_pad);
                //     }
                // }
            #endif
            inp_buf_addr += 1;
            camera_bram_pointer += 1;//地址+2，转跳到下一个像素位置

        }
        if(is_right_pad == 1){inp_buf_addr += 1;}
        camera_bram_pointer += (inp_w - TCol_no_pad);
    }
    #ifdef WI_DEBUG
        // if(1){
        //     printf("=====tile index: %d ======== \n", tile_index);
        //     printf("up: %d, down: %d, left: %d, right: %d \n\n",is_up_pad,  is_down_pad,  is_left_pad,  is_right_pad);
        //     if(test == 1){
        //         printf("=========== 2 ==================\n");
        //     }
        //     else{
        //         printf("=========== 1 ==================\n");
        //     }
        //     for(int channel = 0; channel < 16; channel++){
        //         for(int test_inp = inp_buf_start; test_inp < inp_buf_start + 396; test_inp++){
        //             printf("%d   ", ibuf[test_inp][channel]);
        //             if((test_inp+1) % 22 == 0){
        //                 printf("\n");
        //             }
        //         }
        //         printf("\n");
        //     }
        // }
    #endif
}

void camera_to_inpBuf_test2(short int *camera_bram_pointer, int inp_buf_addr, \
                      const char TRow, const char TCol, \
                      int inp_w, \
                      const char scale_a_single, \
                      bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                      int tile_index){
                    
    int inp_buf_start = inp_buf_addr;

    #ifdef WI_DEBUG
        int test = 0;
        int bak1 = 0, bak2 = 0, bak3 = 0;
        if(inp_buf_addr == INP_BUF_ADDR2){test = 1;}
    #endif
    #ifdef WI_DEBUG
        if(1){
            //printf("camera_to_inpBuf_test\n");
            printf("=====tile index: %d ======== \n", tile_index);
            // printf("up: %d, down: %d, left: %d, right: %d \n\n",is_up_pad,  is_down_pad,  is_left_pad,  is_right_pad);
            // if(test == 1){
            //     printf("=========== 2 ==================\n");
            // }
            // else{
            //     printf("=========== 1 ==================\n");
            // }
            for(int channel = 0; channel < 3; channel++){
                for(int test_inp = inp_buf_start; test_inp < inp_buf_start + 396; test_inp++){
                    printf("%d\t", ibuf[test_inp][channel]);
                    if((test_inp+1) % 22 == 0){
                        printf("\n");
                    }
                }
                printf("\n");
            }
        }
    #endif
}

void psram_to_inpBuf(int psram_addr, int inp_buf_addr, \
                     const char TRow, const char TCol, \
                     int inp_w, \
                     int Tn, int pre_Tm, \
                     bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad){
    char TRow_no_pad = TRow;
    char TCol_no_pad = TCol;
    
    int pixel_all_channel_q[4] = {0, 0, 0, 0};
    uint32_t *pixel_all_channel_q_pointer = &pixel_all_channel_q;

    //col 22 
    //row 18
    //最后几层的时候会出现上下都有pad
    if(is_up_pad == 1){TRow_no_pad -= 1;}
    if(is_down_pad == 1){TRow_no_pad -= 1;}
    if(is_left_pad == 1){TCol_no_pad -= 1;}
    if(is_right_pad == 1){TCol_no_pad -= 1;}

    //如果是上padding, inp_buf的地址需要在一开始多加一行
    //如果是下padding, 直接使用原地址即可，因为循环到最后时不会去写最后一行的inp_buf
    if(is_up_pad == 1){inp_buf_addr += TCol;}

    for(char h = 0; h < TRow_no_pad; h++){
        if(is_left_pad == 1){inp_buf_addr += 1;} //验证正确
        for(char w = 0; w < TCol_no_pad; w ++){
            //如果是左padding, inp_buf的地址需要在每行开头多加1
            //如果是右padding, 直接使用原地址即可，因为循环到最后时不会去写最后一个的inp_buf
            //if(is_left_pad == 1 && (inp_buf_addr - INP_BUF_ADDR) % TCol == 0){inp_buf_addr += 1;}
            read_psram((uint32_t *) pixel_all_channel_q_pointer, (uint32_t) (psram_addr), (int) (Tn - 1));//此处size是byte
            write_input_buffer((uint32_t *) pixel_all_channel_q_pointer, (uint32_t) (inp_buf_addr), (int) (Tn * 8 / 32 - 1));//此处size是word

            inp_buf_addr += 1;
            psram_addr += pre_Tm;//转跳到下一个像素位置

        }    
        if(is_right_pad == 1){inp_buf_addr += 1;}
        psram_addr += pre_Tm * (inp_w - TCol_no_pad);
    }
}

void psram_to_inpBuf_test1(int psram_addr, int inp_buf_addr, \
                     const char TRow, const char TCol, \
                     int inp_w, \
                     int Tn, int pre_Tm, \
                     bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                     int tile_index){
    char TRow_no_pad = TRow;
    char TCol_no_pad = TCol;
    
    int pixel_all_channel_q[4] = {0, 0, 0, 0};
    uint32_t *pixel_all_channel_q_pointer = &pixel_all_channel_q;

    #ifdef PSRAM_WI_DEBUG
        int inp_buf_start = inp_buf_addr;
        int test = 0;
        if(inp_buf_addr == INP_BUF_ADDR2){test = 1;}
    #endif
    //col 22 
    //row 18
    //最后几层的时候会出现上下都有pad
    if(is_up_pad == 1){TRow_no_pad -= 1;}
    if(is_down_pad == 1){TRow_no_pad -= 1;}
    if(is_left_pad == 1){TCol_no_pad -= 1;}
    if(is_right_pad == 1){TCol_no_pad -= 1;}

    //如果是上padding, inp_buf的地址需要在一开始多加一行
    //如果是下padding, 直接使用原地址即可，因为循环到最后时不会去写最后一行的inp_buf
    if(is_up_pad == 1){inp_buf_addr += TCol;}

    for(char h = 0; h < TRow_no_pad; h++){
        if(is_left_pad == 1){inp_buf_addr += 1;} //验证正确
        for(char w = 0; w < TCol_no_pad; w ++){
            //如果是左padding, inp_buf的地址需要在每行开头多加1
            //如果是右padding, 直接使用原地址即可，因为循环到最后时不会去写最后一个的inp_buf
            //if(is_left_pad == 1 && (inp_buf_addr - INP_BUF_ADDR) % TCol == 0){inp_buf_addr += 1;}
            read_psram((uint32_t *) pixel_all_channel_q_pointer, (uint32_t) (psram_addr), (int) (Tn - 1));//此处size是byte
            #ifdef PSRAM_WI_DEBUG
                // if(tile_index == 63){
                //     printf("tile_index: %d \n", tile_index);
                //     printf("psram_addr: %d \n", psram_addr);
                //     char * test_pointer;
                //     test_pointer = (char *)pixel_all_channel_q_pointer;
                //     for(int test_i = 0; test_i < Tn; test_i++){
                //         printf("%d ", test_pointer[test_i]);
                //     }
                //     printf("\n\n");
                // }
            #endif
            write_input_buffer((uint32_t *) pixel_all_channel_q_pointer, (uint32_t) (inp_buf_addr), (int) (Tn * 8 / 32 - 1));//此处size是word
            #ifdef PSRAM_WI_DEBUG
                // if(tile_index == 63){
                //     for(int test_i = 0; test_i < Tn; test_i++){
                //         printf("%d ", ibuf[inp_buf_addr][test_i]);
                //     }
                //     printf("\n\n");
                // }
            #endif
            inp_buf_addr += 1;
            psram_addr += pre_Tm;//转跳到下一个像素位置

        }    
        if(is_right_pad == 1){inp_buf_addr += 1;}
        psram_addr += pre_Tm * (inp_w - TCol_no_pad);
        //printf("line changed \n");
    }
    #ifdef PSRAM_WI_DEBUG
        // if(1){
        //     printf("=====tile index: %d ======== \n", tile_index);
        //     printf("up: %d, down: %d, left: %d, right: %d \n\n",is_up_pad,  is_down_pad,  is_left_pad,  is_right_pad);
        //     if(test == 1){
        //         printf("=========== 2 ==================\n");
        //     }
        //     else{
        //         printf("=========== 1 ==================\n");
        //     }
        //     for(int channel = 0; channel < 16; channel++){
        //         for(int test_inp = inp_buf_start; test_inp < inp_buf_start + 396; test_inp++){
        //             printf("%d\t", ibuf[test_inp][channel]);
        //             if((test_inp+1) % 22 == 0){
        //                 printf("\n");
        //             }
        //         }
        //         printf("\n");
        //     }
        // }
    #endif
}

void psram_to_inpBuf_test2(int psram_addr, int inp_buf_addr, \
                     char TRow, char TCol, \
                     int inp_w, \
                     int Tn, \
                     bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                     int tile_index, int tile_remain_start_index, char layer_index){
    char TRow_no_pad = TRow;
    char TCol_no_pad = TCol;
    
    int pixel_all_channel_q[4] = {0, 0, 0, 0};
    uint32_t *pixel_all_channel_q_pointer = &pixel_all_channel_q;

    int inp_buf_start = inp_buf_addr;
    int test = 0;
    if(inp_buf_addr == INP_BUF_ADDR2){test = 1;}

    if(layer_index == 5){
        printf("=====tile index: %d ======== \n", tile_index);
        // printf("up: %d, down: %d, left: %d, right: %d \n",is_up_pad,  is_down_pad,  is_left_pad,  is_right_pad);
        // if(test == 1){
        //     printf("=========== 2 ==================\n");
        // }
        // else{
        //     printf("=========== 1 ==================\n");
        // }
        for(int channel = 0; channel < 16; channel++){
            for(int test_inp = inp_buf_start; test_inp < inp_buf_start + TRow * TCol; test_inp++){
                printf("%d\t", ibuf[test_inp][channel]);
                if((test_inp+1) % 22 == 0){
                    printf("\n");
                }
            }
            printf("\n");
        }
        //printf("final :%d \n", ibuf[inp_buf_start + 88][0]);
        //printf("inp_buf_start: %d \n", inp_buf_start);
    }
}

void outBuf_to_psRam(int out_buf_addr, int psram_addr, \
                     int *tile_output_save_pointer,\
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc){

    for(char h = 0; h < out_Tr; h++){
        for(char w = 0; w < out_Tc; w++){
            read_output_buffer((uint32_t*) tile_output_save_pointer, (uint32_t) (out_buf_addr), \
                        (int) (Tm * 8 / 32 - 1));//一个像素8位，所以乘8

            write_psram((uint32_t *) tile_output_save_pointer, (uint32_t) (psram_addr), \
                        (int) (Tm - 1)); 

            out_buf_addr += 1;
            psram_addr += Tm;
        }
        psram_addr += Tm * (out_w - out_Tc);
    }
}

void outBuf_to_psRam_test(int out_buf_addr, int psram_addr, \
                     int *tile_output_save_pointer,\
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc, \
                     int tile_index, int kernel_group_index, char layer_index){
    int psram_start = psram_addr;
    int tile_plus = 0;
    if(kernel_group_index > 0){tile_plus += 1;}

    for(char h = 0; h < out_Tr; h++){
        for(char w = 0; w < out_Tc; w++){
            read_output_buffer((uint32_t*) tile_output_save_pointer, (uint32_t) (out_buf_addr), \
                        (int) (Tm * 8 / 32 - 1));//一个像素8位，所以乘8
            #ifdef OUT_DEBUG
                // if(tile_index - 1 + tile_plus == 0){
                //     printf("tile index : %d\n", (tile_index - 1));
                //     for(int channel = 0; channel < Tm; channel++){
                //         char * test_pointer = (char *) tile_output_save_pointer;
                //         printf("out save: %d \n", test_pointer[channel]);
                //     }
                //     printf("\n");
                // }
            #endif

            write_psram((uint32_t *) tile_output_save_pointer, (uint32_t) (psram_addr), \
                        (int) (Tm - 1)); 
            
            #ifdef OUT_DEBUG
                // if(tile_index == 0){
                //     for(int channel2 = 0; channel2 < Tm; channel2++){
                //         printf("psram: %d \n", psram[psram_addr + channel2]);
                //     }
                //     printf("\n");
                // }
            #endif

            out_buf_addr += 1;
            psram_addr += Tm;
        }
        psram_addr += Tm * (out_w - out_Tc);
    }
    #ifdef OUT_DEBUG
        // if(layer_index == 9){
        //     // printf("psram_start: %d \n", psram_start);
        //     // printf("psram_end: %d \n", psram_start + Tm - 1 + (out_Tc-1)*Tm +(out_Tr-1)*out_w*Tm);
        //     printf("================ tile_index %d ============\n", (tile_index - 1 + tile_plus));
        //     for(int channel2 = 0; channel2 < Tm; channel2++){
        //         for(char h = 0; h < out_Tr; h++){
        //             for(char w = 0; w < out_Tc; w++){
        //                 printf("%d\t", psram[psram_start + channel2 + w*Tm + h*out_w*Tm]);
        //             }
        //             printf("\n");
        //         }
        //         printf("\n");
        //     }
        // }
    #endif
}

void outBuf_to_array(int out_buf_addr, char *net_output, int *tile_output_save_pointer, \
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc, \
                     int c_out){
    int save_word_num = Tm * 8 / 32;
    if(Tm * 8 - save_word_num * 32 > 0){save_word_num += 1;}

    for(char h = 0; h < out_Tr; h++){
        for(char w = 0; w < out_Tc; w++){
            read_output_buffer((uint32_t*) tile_output_save_pointer, (uint32_t) (out_buf_addr), \
                        (int) (save_word_num - 1));//一个像素8位，所以乘8
            
            memcpy(net_output, tile_output_save_pointer, Tm);

            out_buf_addr += 1;
            net_output += c_out;
        }
        net_output += Tm * (out_w - out_Tc);//正常来说Tm * (out_w - out_Tc) = 0
    }
}

void outBuf_to_array_test(int out_buf_addr, char *net_output, int *tile_output_save_pointer, \
                     int out_w, \
                     char Tm, char out_Tr, char out_Tc, \
                     int c_out, int tile_index, int kernel_group_index, char layer_index){
    int save_word_num = Tm * 8 / 32;
    if(Tm * 8 - save_word_num * 32 > 0){save_word_num += 1;}
    
    char *net_output_start = net_output;
    int tile_plus = 0;
    if(kernel_group_index > 0){tile_plus += 1;}
    

    for(char h = 0; h < out_Tr; h++){
        for(char w = 0; w < out_Tc; w++){
            read_output_buffer((uint32_t*) tile_output_save_pointer, (uint32_t) (out_buf_addr), \
                        (int) (save_word_num - 1));//一个像素8位，所以乘8
            
            memcpy(net_output, tile_output_save_pointer, Tm);

            #ifdef OUT_DEBUG
                // if(tile_index - 1 + tile_plus == -1){
                //     printf("tile index : %d\n", (tile_index - 1 + tile_plus));
                //     for(int channel = 0; channel < Tm; channel++){
                //         char * test_pointer = net_output;
                //         printf("out save: %d \n", test_pointer[channel]);
                //         printf("out buf: %d \n", obuf[w + h*out_Tc][channel]);
                //         printf("out buf2: %d \n", obuf[out_buf_addr][channel]);
                //     }
                //     printf("\n");
                // }
            #endif
            

            out_buf_addr += 1;
            net_output += c_out;
        }
        net_output += Tm * (out_w - out_Tc);//正常来说Tm * (out_w - out_Tc) = 0
    }
    #ifdef OUT_DEBUG
        if(1){
            // printf("psram_start: %d \n", psram_start);
            // printf("psram_end: %d \n", psram_start + Tm - 1 + (out_Tc-1)*Tm +(out_Tr-1)*out_w*Tm);
            printf("================ tile_index %d ============\n", (tile_index - 1 + tile_plus));
            for(int channel2 = 0; channel2 < Tm; channel2++){
                for(char h = 0; h < out_Tr; h++){
                    for(char w = 0; w < out_Tc; w++){
                        printf("%d\t", net_output_start[channel2 + w*35 + h*out_w*35]);
                    }
                    printf("\n");
                }
                printf("\n");
            }
        }
    #endif
}