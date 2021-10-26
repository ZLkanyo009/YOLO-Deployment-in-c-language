#include "cores.h"

struct INFO first_conv(int inp_w, int inp_h, \
                       char Tr, char Tc, char Tm, char Tn, \
                       char TRow, char TCol, char layer_index, \
                       short int *camera_bram_pointer, int *tile_output_save_pointer, \
                       char *b_conv, char *w_conv, \
                       int psram_addr, \
                       bool activ, bool pool, \
                       bool pingpong[], \
                       char kernel_size){
    struct INFO layer_info = {0, 0, 0, 0, 0};
    int out_w = inp_w;
    int out_h = inp_h;
    char out_Tr = Tr;
    char out_Tc = Tc;

    char delay_out_Tr = Tr;

    int tile_col_num = inp_w / Tc;
    int tile_row_num_for_remain = inp_h / Tr; //向下取整
    int tile_row_num = tile_row_num_for_remain; //向下取整
    if(inp_h % Tr != 0){
        tile_row_num += 1;
    }
    int tile_total = tile_col_num * tile_row_num;

    int tile_remain_start_index = tile_col_num * tile_row_num_for_remain;
    char tile_row_remain_Tr = inp_h - tile_row_num_for_remain * Tr; //由于16的引入，行会存在remain的情况
    //printf("tile_row_num_for_remain: %d \n", tile_row_num_for_remain);
    char out_tile_row_remain_Tr = tile_row_remain_Tr;

    bool first_channel_group, last_channel_group;

    bool is_up_pad, is_down_pad;
    bool is_left_pad, is_right_pad;

    first_channel_group = 1, last_channel_group = 1;

    if(pool == 1){//在这里添加output的tile并在下方添加进来
        out_w /= 2;
        out_h /= 2;
        out_Tr /= 2;
        delay_out_Tr /= 2;
        out_Tc /= 2;
        out_tile_row_remain_Tr /= 2;
    }

    //设置tile基本信息
    set_tile_info(Tn, Tm, Tc, Tr);
    //设置每层的量化scale指数
    const char scale_a_single_i = scale_a[layer_index];
    const char scale_w_single = scale_w[layer_index];
    const char scale_b_single = scale_b[layer_index];
    const char retune_single = retune[layer_index];
    const char scale_a_single_o = scale_a[layer_index + 1];


    set_quantize_scale(scale_a_single_i, scale_w_single, scale_b_single, \
                       retune_single, scale_a_single_o);

    load_bias(BIAS_BUF_ADDR, (char *)b_conv, Tm); //第一层的bias可以一次性全部送入

    if(pingpong[1] == 0){
        load_weight(KERNEL_BUF_ADDR, (char *)w_conv, kernel_size, TN, TM);//第一、二层的weight可以一次性全部送入
    }
    else if(pingpong[1] == 1){
        load_weight(KERNEL_BUF_ADDR2, (char *)w_conv, kernel_size, TN, TM);//第一、二层的weight可以一次性全部送入
    }

    for(int tile_index = 0; tile_index < tile_total; tile_index++){
        if(tile_index % tile_col_num == 0){is_left_pad = 1;}
        else{is_left_pad = 0;}

        if(tile_index % tile_col_num == tile_col_num - 1){is_right_pad = 1;}
        else{is_right_pad = 0;}

        if(tile_index / tile_col_num == 0){is_up_pad = 1;} //tile_index < tile_col_num
        else{is_up_pad = 0;}

        if(tile_index / tile_col_num == tile_row_num - 1){is_down_pad = 1;}
        else{is_down_pad = 0;}

        if(tile_index == tile_remain_start_index){
            Tr = tile_row_remain_Tr;
            TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            out_Tr = out_tile_row_remain_Tr;
        }

        else if(tile_index == tile_remain_start_index + 1){
            //delay_Tr = tile_row_remain_Tr;
            //delay_TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            delay_out_Tr = out_tile_row_remain_Tr;
        }
        
        //需要第一层的scale_a用于int 8量化
        if(pingpong[0] == 0){
            #ifdef WI_DEBUG
                camera_to_inpBuf_test1(camera_bram_pointer, INP_BUF_ADDR, TRow, TCol, inp_w, scale_a_single_i, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index);
            #else 
                camera_to_inpBuf(camera_bram_pointer, INP_BUF_ADDR, TRow, TCol, inp_w, scale_a_single_i, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad);
            #endif
        }
        else if(pingpong[0] == 1){
            #ifdef WI_DEBUG
                camera_to_inpBuf_test1(camera_bram_pointer, INP_BUF_ADDR2, TRow, TCol, inp_w, scale_a_single_i, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index);
            #else
                camera_to_inpBuf(camera_bram_pointer, INP_BUF_ADDR2, TRow, TCol, inp_w, scale_a_single_i, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad);
            #endif
        }

        
        //判断下一个tile_index时取inp的地址应当如何变化
        //有些情况（虽然不在我们的模型里出现过这种情况）中左pad和右pad会同时出现，但上pad和下pad不同时出现，这种情况暂时不考虑
        //最后几层是四种pad都同时出现，这时tile只有一个，不需要考虑在平面上移动取值地址了
        //这里的Tc和Tr不可以使用camera_to_inpBuf函数里的no pad来代替
        //因为camera_to_inpBuf里考虑的是TRow和TCol,跟这边不太一样的
        //camera内一个像素int4
        //camera_pointer完全正确
        if(is_right_pad == 1 && is_up_pad == 1){camera_bram_pointer += (Tc + 1) + (Tr - 2) * inp_w;}//加上Tc后已经主动加上一行，再考虑pad，故-2
        else if(is_right_pad == 1 && is_up_pad == 0){camera_bram_pointer += (Tc + 1) + (Tr - 1) * inp_w;}
        else if(is_left_pad == 1){camera_bram_pointer += Tc - 1;}
        else{camera_bram_pointer += Tc;}

        
        if(tile_index != 0){
            wait_cal_done();
            #ifdef OUT_DEBUG1
                // psram_addr 考虑空间大小和存放feature map的方式，和如何能够准确的找出来对应tile的对应channel
                outBuf_to_psRam_test(OUR_BUF_ADDR, psram_addr, tile_output_save_pointer, \
                                out_w, Tm, delay_out_Tr, out_Tc, tile_index, 0, layer_index);
            #else
                outBuf_to_psRam(OUR_BUF_ADDR, psram_addr, tile_output_save_pointer, \
                                out_w, Tm, delay_out_Tr, out_Tc);
            #endif
            // 最后几层不能使用这种is_right_pad == 1的方式来判断
            // 由于这一部分代码是把上一次tile的output写到psram，所以上次是right pad时
            // 需要psram_addr变到新的一行去，而上次是right pad，这次就是left pad
            if(is_left_pad == 1){psram_addr += Tm * out_Tc + (delay_out_Tr - 1) * (Tm * out_w);}
            else{psram_addr += Tm * out_Tc;}

            #ifdef PSRAM_START_ADDR_DEBUG
                if(is_left_pad == 1){printf("right!!!!!!!!!!!");}
                printf("Tm * out_Tc + (delay_out_Tr - 1) * (Tm * out_w): %d \n", Tm * out_Tc + (delay_out_Tr - 1) * (Tm * out_w));
                printf("Tm * out_Tc: %d \n", Tm * out_Tc);
                printf("psram_addr: %d \n\n", psram_addr);
            #endif
        }
        
        if(tile_index == tile_remain_start_index){
            set_tile_info(Tn, Tm, Tc, Tr);
        }
        //配置tile细节
        config_ctl_reg(is_up_pad, is_down_pad, is_left_pad, is_right_pad, \
                       pingpong[0], pingpong[1], pingpong[2], \
                       first_channel_group, last_channel_group, activ, pool);
        #ifdef WI_DEBUG
            if(pingpong[0] == 0){
                camera_to_inpBuf_test2(camera_bram_pointer, INP_BUF_ADDR, TRow, TCol, inp_w, scale_a_single_i, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index);
            }
            else if(pingpong[0] == 1){
                camera_to_inpBuf_test2(camera_bram_pointer, INP_BUF_ADDR2, TRow, TCol, inp_w, scale_a_single_i, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index);
            }
        #endif

        start_calculate(tile_index, layer_index);

        pingpong[0] = pingpong_invert(pingpong[0]);
    }
    pingpong[1] = pingpong_invert(pingpong[1]);
    pingpong[2] = pingpong_invert(pingpong[2]);
    
    layer_info.psram_addr = psram_addr;
    layer_info.out_w = out_w;
    layer_info.Tm = Tm;
    layer_info.out_Tr = out_Tr;
    layer_info.out_Tc = out_Tc;

    return layer_info; //返回最后一个tile需要write到psram的addr，给下一层的一开始使用
}

struct INFO second_conv(int inp_w, int inp_h, \
                        char Tr, char Tc, char Tm, char Tn, \
                        char TRow, char TCol, char layer_index, \
                        int *tile_output_save_pointer, \
                        char *b_conv, char *w_conv, \
                        int psram_in_addr, int psram_out_addr, \
                        struct INFO pre_layer_info, \
                        bool activ, bool pool, \
                        bool pingpong[], \
                        char kernel_size){
    struct INFO layer_info = {0, 0, 0, 0, 0};

    int out_w = inp_w;
    int out_h = inp_h;
    char out_Tr = Tr;
    char out_Tc = Tc;

    char delay_out_Tr = Tr;

    int tile_col_num = inp_w / Tc;
    int tile_row_num_for_remain = inp_h / Tr; //向下取整
    int tile_row_num = tile_row_num_for_remain; //向下取整
    if(inp_h % Tr != 0){
        tile_row_num += 1;
    }
    int tile_total = tile_col_num * tile_row_num;

    int tile_remain_start_index = tile_col_num * tile_row_num_for_remain;
    char tile_row_remain_Tr = inp_h - tile_row_num_for_remain * Tr; //由于16的引入，行会存在remain的情况
    char out_tile_row_remain_Tr = tile_row_remain_Tr;
    

    bool first_channel_group, last_channel_group;

    bool is_up_pad, is_down_pad;
    bool is_left_pad, is_right_pad;

    first_channel_group = 1, last_channel_group = 1;

    if(pool == 1){//在这里添加output的tile并在下方添加进来
        out_w /= 2;
        out_h /= 2;
        out_Tr /= 2;
        delay_out_Tr /= 2;
        out_Tc /= 2;
        out_tile_row_remain_Tr /= 2;
    }

    //设置每层的量化scale指数
    const char scale_a_single_i = scale_a[layer_index];
    const char scale_w_single = scale_w[layer_index];
    const char scale_b_single = scale_b[layer_index];
    const char retune_single = retune[layer_index];
    const char scale_a_single_o = scale_a[layer_index + 1];


    if(pingpong[2] == 0){
        load_bias(BIAS_BUF_ADDR, (char *)b_conv, Tm); //第二层的bias可以一次性全部送入
    }
    else if(pingpong[2] == 1){
        load_bias(BIAS_BUF_ADDR2, (char *)b_conv, Tm); //第二层的bias可以一次性全部送入
    }

    //weight需要load 
    if(pingpong[1] == 0){
        load_weight(KERNEL_BUF_ADDR, (char *)w_conv, kernel_size, TN, TM);//第一、二层的weight可以一次性全部送入
    }
    else if(pingpong[1] == 1){
        load_weight(KERNEL_BUF_ADDR2, (char *)w_conv, kernel_size, TN, TM);//第一、二层的weight可以一次性全部送入
    }

    for(int tile_index = 0; tile_index < tile_total; tile_index++){
        //写入channel group
        if(tile_index % tile_col_num == 0){is_left_pad = 1;}
        else{is_left_pad = 0;}

        if(tile_index % tile_col_num == tile_col_num - 1){is_right_pad = 1;}
        else{is_right_pad = 0;}

        if(tile_index / tile_col_num == 0){is_up_pad = 1;} //tile_index < tile_col_num
        else{is_up_pad = 0;}

        if(tile_index / tile_col_num == tile_row_num - 1){is_down_pad = 1;}//未debug存疑
        else{is_down_pad = 0;}

        if(tile_index == tile_remain_start_index){
            Tr = tile_row_remain_Tr;
            TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            out_Tr = out_tile_row_remain_Tr;
        }
        else if(tile_index == tile_remain_start_index + 1){
            //delay_Tr = tile_row_remain_Tr;
            //delay_TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            delay_out_Tr = out_tile_row_remain_Tr;
        }
        
        //需要第一层的scale_a用于int 8量化
        if(pingpong[0] == 0){
            #ifdef PSRAM_WI_DEBUG
                psram_to_inpBuf_test1(psram_in_addr, INP_BUF_ADDR, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index);
            #else
                psram_to_inpBuf(psram_in_addr, INP_BUF_ADDR, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad);
            #endif
        }
        else if(pingpong[0] == 1){
            #ifdef PSRAM_WI_DEBUG
                psram_to_inpBuf_test1(psram_in_addr, INP_BUF_ADDR2, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index);
            #else
                psram_to_inpBuf(psram_in_addr, INP_BUF_ADDR2, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad);
            #endif
        }
        
        //判断下一个tile_index时取inp的地址应当如何变化
        //有些情况（虽然不在我们的模型里出现过这种情况）中左pad和右pad会同时出现，但上pad和下pad不同时出现，这种情况暂时不考虑
        //最后几层是四种pad都同时出现，这时tile只有一个，不需要考虑在平面上移动取值地址了
        //psram内一个像素int8
        if(is_right_pad == 1 && is_up_pad == 1){psram_in_addr += pre_layer_info.Tm * ((Tc + 1) + (Tr - 2) * inp_w);}//加上Tc后已经主动加上一行，再考虑pad，故-2
        else if(is_right_pad == 1 && is_up_pad == 0){psram_in_addr += pre_layer_info.Tm * ((Tc + 1) + (Tr - 1) * inp_w);}
        else if(is_left_pad == 1){psram_in_addr += pre_layer_info.Tm * (Tc - 1);}
        else{psram_in_addr += pre_layer_info.Tm * Tc;}
        
        wait_cal_done();
        // psram_addr 考虑空间大小和存放feature map的方式，和如何能够准确的找出来对应tile的对应channel
        //还需要考虑上一层的最后一个output的write
        if(tile_index == 0){
            #ifdef OUT_DEBUG2
                outBuf_to_psRam_test(OUR_BUF_ADDR, pre_layer_info.psram_addr, tile_output_save_pointer,\
                                pre_layer_info.out_w, pre_layer_info.Tm, \
                                pre_layer_info.out_Tr, pre_layer_info.out_Tc, tile_index, 0, layer_index);
            #else
                outBuf_to_psRam(OUR_BUF_ADDR, pre_layer_info.psram_addr, tile_output_save_pointer,\
                                pre_layer_info.out_w, pre_layer_info.Tm, \
                                pre_layer_info.out_Tr, pre_layer_info.out_Tc);
            #endif  
            //设置tile基本信息
            set_tile_info(Tn, Tm, Tc, Tr);
            set_quantize_scale(scale_a_single_i, scale_w_single, scale_b_single, \
                               retune_single, scale_a_single_o);
                               
        }
        else{
            #ifdef OUT_DEBUG2
                outBuf_to_psRam_test(OUR_BUF_ADDR, psram_out_addr, tile_output_save_pointer,\
                                out_w, Tm, delay_out_Tr, out_Tc, tile_index, 0, layer_index);
            #else
                outBuf_to_psRam(OUR_BUF_ADDR, psram_out_addr, tile_output_save_pointer,\
                                out_w, Tm, delay_out_Tr, out_Tc);
            #endif  
            if(is_left_pad == 1){psram_out_addr += Tm * out_Tc + (delay_out_Tr - 1) * (Tm * out_w);}
            else{psram_out_addr += Tm * out_Tc;}
        }
        
        //配置tile细节
        if(tile_index == tile_remain_start_index){
            set_tile_info(Tn, Tm, Tc, Tr);
        }
        #ifdef SET_TILE_DETAIL_DEBUG
            printf("tile_index: %d \n", tile_index);
            printf("is_right_pad: %d ,", is_right_pad);
            printf("is_down_pad: %d ,", is_down_pad);
            printf("is_left_pad: %d ,", is_left_pad);
            printf("is_up_pad: %d ,", is_up_pad);
            printf("pingpong[0]: %d ,", pingpong[0]);
            if(pingpong[1] == 0){
                printf("pingpong[1]: %d ,", pingpong[1]);}
            if(pingpong[2] == 0){
            printf("pingpong[2]: %d ,", pingpong[2]);}
            if(first_channel_group == 0){
                printf("first_channel_group: %d ,", first_channel_group);}
            if(last_channel_group == 0){
            printf("last_channel_group: %d ,", last_channel_group);}
            if(activ == 0){
            printf("activ: %d ,", activ);}
            if(pool == 0){
                printf("pool: %d \n\n", pool);}
            else{printf("\n\n");}
        #endif

        config_ctl_reg(is_up_pad, is_down_pad, is_left_pad, is_right_pad, \
                        pingpong[0], pingpong[1], pingpong[2], \
                        first_channel_group, last_channel_group, activ, pool);
        
        #ifdef PSRAM_WI_DEBUG
            if(pingpong[0] == 0){
                psram_to_inpBuf_test2(psram_in_addr, INP_BUF_ADDR, TRow, TCol, inp_w, Tn, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index, \
                                tile_remain_start_index, layer_index);
            }
            else if(pingpong[0] == 1){
                psram_to_inpBuf_test2(psram_in_addr, INP_BUF_ADDR2, TRow, TCol, inp_w, Tn, \
                                is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index, \
                                tile_remain_start_index, layer_index);
            }
        #endif

        start_calculate(tile_index, layer_index);

        pingpong[0] = pingpong_invert(pingpong[0]);
    }
    pingpong[1] = pingpong_invert(pingpong[1]);
    pingpong[2] = pingpong_invert(pingpong[2]);

    layer_info.psram_addr = psram_out_addr;
    layer_info.out_w = out_w;
    layer_info.Tm = Tm;
    layer_info.out_Tr = out_Tr;
    layer_info.out_Tc = out_Tc;

    return layer_info; //返回最后一个tile需要write到psram的addr，给下一层的一开始使用
}


struct INFO conv_normal(int inp_w, int inp_h, \
                        int c_in, int c_out, \
                        char Tr, char Tc, char Tm, char Tn, \
                        char TRow, char TCol, char layer_index, \
                        int *tile_output_save_pointer, \
                        char *b_conv, char *w_conv, \
                        int psram_in_addr2d, int psram_out_addr2d, struct INFO pre_layer_info, \
                        bool activ, bool pool, \
                        bool pingpong[], \
                        char kernel_size){
    struct INFO layer_info = {0, 0, 0, 0, 0};
    char *b_conv_start = b_conv;
    char *w_conv_start = w_conv;

    int out_w = inp_w;
    int out_h = inp_h;
    char out_Tr = Tr;
    char out_Tc = Tc;

    //char delay_Tr = Tr;   //delay是用于out psram的延时性而声明的
    char delay_out_Tr = Tr;
    //char delay_TRow = TRow;

    int psram_in_addr3d = psram_in_addr2d;
    int psram_out_addr3d = psram_out_addr2d;
    bool flag_psram_in = 0;//用于判断input_psram是应该加到下一页去还是加到下16个channel去

    char channel_repeat = c_in / Tn; //每层硬编码的input_channel数 / Tn;
    char kernel_num_repeat = c_out / Tm;

    int tile_col_num = inp_w / Tc;

    int tile_row_num_for_remain = inp_h / Tr; //向下取整
    int tile_remain_start_index = tile_col_num * tile_row_num_for_remain;
    char tile_row_remain_Tr = inp_h - tile_row_num_for_remain * Tr; //由于16的引入，行会存在remain的情况
    char out_tile_row_remain_Tr = tile_row_remain_Tr;
    
    int tile_row_num = tile_row_num_for_remain; //向下取整
    if(tile_row_remain_Tr != 0){
        tile_row_num += 1;
    }
    int tile_total = tile_col_num * tile_row_num;
    

    bool first_channel_group, last_channel_group;

    bool is_up_pad, is_down_pad;
    bool is_left_pad, is_right_pad;

    if(pool == 1){//在这里添加output的tile并在下方添加进来
        out_w /= 2;
        out_h /= 2;
        out_Tr /= 2;
        delay_out_Tr /= 2;
        out_Tc /= 2;
        out_tile_row_remain_Tr /= 2;
    }

    //设置每层的量化scale指数
    const char scale_a_single_i = scale_a[layer_index];
    const char scale_w_single = scale_w[layer_index];
    const char scale_b_single = scale_b[layer_index];
    const char retune_single = retune[layer_index];
    const char scale_a_single_o = scale_a[layer_index + 1];

    for(int tile_index = 0; tile_index < tile_total; tile_index++){
        w_conv = w_conv_start;
        b_conv = b_conv_start;
        
        //tile index指的是平面的、二维的tile index
        //写入channel group
        if(tile_index % tile_col_num == 0){is_left_pad = 1;}
        else{is_left_pad = 0;}

        if(tile_index % tile_col_num == tile_col_num - 1){is_right_pad = 1;}
        else{is_right_pad = 0;}

        if(tile_index / tile_col_num == 0){is_up_pad = 1;} //tile_index < tile_col_num
        else{is_up_pad = 0;}

        if(tile_index / tile_col_num == tile_row_num - 1){is_down_pad = 1;}//存疑
        else{is_down_pad = 0;}

        //这里改了之后还需要考虑out2psram的延时性，是不能完全对应上的。-->已修改
        if(tile_index == tile_remain_start_index){
            Tr = tile_row_remain_Tr;
            TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            out_Tr = out_tile_row_remain_Tr;
        }
        else if(tile_index == tile_remain_start_index + 1){
            //delay_Tr = tile_row_remain_Tr;
            //delay_TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            delay_out_Tr = out_tile_row_remain_Tr;
        }

        flag_psram_in = 0;
        for(char kernel_group_index = 0; kernel_group_index < kernel_num_repeat; kernel_group_index++){
            if(tile_index == tile_remain_start_index && kernel_group_index != 0){
                delay_out_Tr = out_tile_row_remain_Tr;
            }
            
            psram_in_addr3d = psram_in_addr2d; //进入channel循环前先用2d给3d赋值，将初始inp地址变成channel = 0的地方

            if(pingpong[2] == 0){
                load_bias(BIAS_BUF_ADDR, (char *)b_conv, Tm);
                b_conv += Tm;
            }
            else if(pingpong[2] == 1){
                load_bias(BIAS_BUF_ADDR2, (char *)b_conv, Tm);
                b_conv += Tm;
            }

            for(char channel_group_index = 0; channel_group_index < channel_repeat; channel_group_index++){    
                if(channel_group_index == 0){
                    first_channel_group = 1;
                    last_channel_group = 0;
                } 
                else if(channel_group_index == channel_repeat - 1 && channel_repeat != 1){
                    first_channel_group = 0;
                    last_channel_group = 1;                
                }
                // else if(channel_group_index == channel_repeat - 1 && channel_repeat == 1){
                //     first_channel_group = 1;
                //     last_channel_group = 1;                
                // }
                else{
                    first_channel_group = 0;
                    last_channel_group = 0;                
                }

                //w_conv的addr需要更改
                //h w c*n
                if(pingpong[1] == 0){
                    //第一二层因为c in和c out都不大于tile的Tn和Tm，所以就直接填入即可(防止传参消耗)
                    //第三层开始需要填c in和c out
                    load_weight(KERNEL_BUF_ADDR, (char *)w_conv, kernel_size, c_in, c_out);
                    w_conv += Tn * Tm;
                }
                else if(pingpong[1] == 1){
                    load_weight(KERNEL_BUF_ADDR2, (char *)w_conv, kernel_size, c_in, c_out);
                    w_conv += Tn * Tm;
                }
                if(pingpong[0] == 0){
                    psram_to_inpBuf(psram_in_addr3d, INP_BUF_ADDR, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                    is_up_pad, is_down_pad, is_left_pad, is_right_pad);
                }
                else if(pingpong[0] == 1){
                    psram_to_inpBuf(psram_in_addr3d, INP_BUF_ADDR2, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                    is_up_pad, is_down_pad, is_left_pad, is_right_pad);
                }
                if(flag_psram_in == 0){ //针对Tn和上一层的Tm不相等的问题作出解决
                    psram_in_addr3d += Tn;
                }
                else if(flag_psram_in == 1){
                    psram_in_addr3d += pre_layer_info.Tm * (inp_w * inp_h) - Tn;
                }
                wait_cal_done();
                // psram_addr 考虑空间大小和存放feature map的方式，和如何能够准确的找出来对应tile的对应channel
                if(tile_index == 0 && kernel_group_index == 0 && channel_group_index == 0){
                    #ifdef OUT_DEBUG3
                        outBuf_to_psRam_test(OUR_BUF_ADDR, pre_layer_info.psram_addr, tile_output_save_pointer,\
                                        pre_layer_info.out_w, pre_layer_info.Tm, \
                                        pre_layer_info.out_Tr, pre_layer_info.out_Tc, tile_index, 0, layer_index);
                    #else
                        outBuf_to_psRam(OUR_BUF_ADDR, pre_layer_info.psram_addr, tile_output_save_pointer,\
                                        pre_layer_info.out_w, pre_layer_info.Tm, \
                                        pre_layer_info.out_Tr, pre_layer_info.out_Tc);
                    #endif 
                    //设置tile基本信息
                    set_tile_info(Tn, Tm, Tc, Tr);
                    set_quantize_scale(scale_a_single_i, scale_w_single, scale_b_single, \
                                       retune_single, scale_a_single_o);
                }
                else if(channel_group_index == 0){
                    #ifdef OUT_DEBUG3
                        outBuf_to_psRam_test(OUR_BUF_ADDR, psram_out_addr3d, tile_output_save_pointer,\
                                    out_w, Tm, delay_out_Tr, out_Tc, tile_index, kernel_group_index, layer_index);
                    #else
                        outBuf_to_psRam(OUR_BUF_ADDR, psram_out_addr3d, tile_output_save_pointer,\
                                    out_w, Tm, delay_out_Tr, out_Tc);
                    #endif 
                    //下面代码是因为在kernel_group_index == 0时，存的是上一个tile的最后一组channel的output
                    //当在kernel_group_index > 0时才会使用这一次的tile 2d地址
                    //逻辑判断来看，这个psram_out_addr2d和3d的逻辑应该能在kernel和channel group repeat各自或同时=1时使用
                    if(kernel_group_index == 0){psram_out_addr3d = psram_out_addr2d;}
                    else{psram_out_addr3d += Tm * (out_w * out_h);}//存完一组channel连续放的out_tile后会跳转到后一张图
                }


                if(tile_index == tile_remain_start_index){
                    set_tile_info(Tn, Tm, Tc, Tr);
                }
                //配置tile细节
                #ifdef SET_TILE_DETAIL_DEBUG
                    printf("is_right_pad: %d ,", is_right_pad);
                    printf("is_down_pad: %d ,", is_down_pad);
                    printf("is_left_pad: %d ,", is_left_pad);
                    printf("is_up_pad: %d ,", is_up_pad);
                    printf("pingpong[0]: %d ,", pingpong[0]);
                    printf("pingpong[1]: %d ,", pingpong[1]);
                    printf("pingpong[2]: %d ,", pingpong[2]);
                    printf("first_channel_group: %d ,", first_channel_group);
                    printf("last_channel_group: %d ,", last_channel_group);
                    printf("activ: %d ,", activ);
                    printf("pool: %d \n\n", pool);
                #endif
                config_ctl_reg(is_up_pad, is_down_pad, is_left_pad, is_right_pad, \
                               pingpong[0], pingpong[1], pingpong[2], \
                               first_channel_group, last_channel_group, activ, pool);
                
                #ifdef PSRAM_WI_DEBUG2
                    if(pingpong[0] == 0){
                        psram_to_inpBuf_test2(psram_in_addr3d, INP_BUF_ADDR, TRow, TCol, inp_w, Tn, \
                                        is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index, \
                                        tile_remain_start_index, layer_index);
                    }
                    else if(pingpong[0] == 1){
                        psram_to_inpBuf_test2(psram_in_addr3d, INP_BUF_ADDR2, TRow, TCol, inp_w, Tn, \
                                        is_up_pad, is_down_pad, is_left_pad, is_right_pad, tile_index, \
                                        tile_remain_start_index, layer_index);
                    }
                #endif

                start_calculate(tile_index, layer_index);

                pingpong[0] = pingpong_invert(pingpong[0]);
                pingpong[1] = pingpong_invert(pingpong[1]);
                flag_psram_in = pingpong_invert(flag_psram_in);
            }
            pingpong[2] = pingpong_invert(pingpong[2]);
        }
        //判断下一个tile_index时取inp的地址应当如何变化
        //有些情况（虽然不在我们的模型里出现过这种情况）中左pad和右pad会同时出现，但上pad和下pad不同时出现，这种情况暂时不考虑
        //最后几层是四种pad都同时出现，这时tile只有一个，不需要考虑在平面上移动取值地址了
        if(is_right_pad == 1 && is_up_pad == 1){psram_in_addr2d += pre_layer_info.Tm * ((Tc + 1) + (Tr - 2) * inp_w);}//加上Tc后已经主动加上一行，再考虑pad，故-2
        //最后一列时不用tile倒退两格，而是进到下一行，所以+2，但因为右pad，所以又-1
        else if(is_right_pad == 1 && is_up_pad == 0){psram_in_addr2d += pre_layer_info.Tm * ((Tc + 1) + (Tr - 1) * inp_w);}
        else if(is_left_pad == 1){psram_in_addr2d += pre_layer_info.Tm * (Tc - 1);}
        else{psram_in_addr2d += pre_layer_info.Tm * Tc;}
        
        //下方条件判断是对于kernel_repeat做出的分类
        //当kernenl_repeat == 1，也就是只有一组output时，在下一个2d的tile时才会去存上一个tile的output
        //但当kernel_repeat > 1，在当前2d tile的kernel group num > 0时，就会开始存这个tile的output
        //所以会有下面的right pad和left pad的区别
        if((is_right_pad == 1 && kernel_num_repeat > 1) || (is_left_pad == 1 && kernel_num_repeat == 1)){
            psram_out_addr2d += Tm * out_Tc + (delay_out_Tr - 1) * (Tm * out_w);
        }
        else{psram_out_addr2d += Tm * out_Tc;}
    }
    layer_info.psram_addr = psram_out_addr3d;
    layer_info.out_w = out_w;
    layer_info.Tm = Tm;
    layer_info.out_Tr = out_Tr;
    layer_info.out_Tc = out_Tc;

    return layer_info; //返回最后一个tile需要write到psram的addr，给下一层的一开始使用
}

struct INFO conv_last(int inp_w, int inp_h, \
                        int c_in, int c_out, \
                        char Tr, char Tc, char Tm, char Tn, \
                        char TRow, char TCol, char layer_index, \                        
                        int *tile_output_save_pointer, \
                        char *b_conv, char *w_conv, \
                        int psram_in_addr2d, char *net_output, struct INFO pre_layer_info, \
                        bool activ, bool pool, \
                        bool pingpong[], \
                        char kernel_size){
    struct INFO layer_info = {0, 0, 0, 0, 0};
    char *b_conv_start = b_conv;
    char *w_conv_start = w_conv;

    int out_w = inp_w;
    int out_h = inp_h;
    char out_Tr = Tr;
    char out_Tc = Tc;

    char delay_out_Tr = Tr;
    bool delay_Tm_flag = 0;
    char ori_Tm = Tm;
    char delay_Tm = ori_Tm;

    int psram_in_addr3d = psram_in_addr2d;
    bool flag_psram_in = 0;//用于判断input_psram是应该加到下一页去还是加到下16个channel去

    char channel_repeat = c_in / Tn; //每层硬编码的input_channel数 / Tn;

    char kernel_num_repeat_for_remain = c_out / Tm;//最后一层存在余数
    char kernel_num_remain = c_out - kernel_num_repeat_for_remain * Tm;
    char kernel_num_remain_start_index = kernel_num_repeat_for_remain;

    char kernel_num_repeat = kernel_num_repeat_for_remain;
    if(kernel_num_remain != 0){
        kernel_num_repeat += 1;
    }   
    if(kernel_num_remain_start_index == 0){
        ori_Tm = kernel_num_remain;
        delay_Tm = ori_Tm;
    }

    int tile_col_num = inp_w / Tc;
    
    int tile_row_num_for_remain = inp_h / Tr; //向下取整
    int tile_remain_start_index = tile_col_num * tile_row_num_for_remain;
    char tile_row_remain_Tr = inp_h - tile_row_num_for_remain * Tr; //由于16的引入，行会存在remain的情况
    char out_tile_row_remain_Tr = tile_row_remain_Tr;

    int tile_row_num = tile_row_num_for_remain; //向下取整
    if(tile_row_remain_Tr != 0){
        tile_row_num += 1;
    }
    int tile_total = tile_col_num * tile_row_num;


    bool first_channel_group, last_channel_group;

    bool is_up_pad, is_down_pad;
    bool is_left_pad, is_right_pad;

    if(pool == 1){//在这里添加output的tile并在下方添加进来
        out_w /= 2;
        out_h /= 2;
        out_Tr /= 2;
        delay_out_Tr /= 2;
        out_Tc /= 2;
        out_tile_row_remain_Tr /= 2;
    }

    //设置每层的量化scale指数
    const char scale_a_single_i = scale_a[layer_index];
    const char scale_w_single = scale_w[layer_index];
    const char scale_b_single = scale_b[layer_index];
    const char retune_single = retune[layer_index];
    const char scale_a_single_o = scale_a[layer_index + 1];

    for(int tile_index = 0; tile_index < tile_total; tile_index++){
        w_conv = w_conv_start;
        b_conv = b_conv_start;
        //tile index指的是平面的、二维的tile index
        //写入channel group
        if(tile_index % tile_col_num == 0){is_left_pad = 1;}
        else{is_left_pad = 0;}

        if(tile_index % tile_col_num == tile_col_num - 1){is_right_pad = 1;}
        else{is_right_pad = 0;}

        if(tile_index / tile_col_num == 0){is_up_pad = 1;} //tile_index < tile_col_num
        else{is_up_pad = 0;}

        if(tile_index / tile_col_num == tile_row_num - 1){is_down_pad = 1;}
        else{is_down_pad = 0;}

        if(tile_index == tile_remain_start_index){
            Tr = tile_row_remain_Tr;
            TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            out_Tr = out_tile_row_remain_Tr;
        }
        else if(tile_index == tile_remain_start_index + 1){
            //由于delay_Tr和delay_TRow下文不会用到, 只用到delay_out_Tr,故注释掉
            //delay_Tr = tile_row_remain_Tr;  
            //delay_TRow = tile_row_remain_Tr + 2; //如果kernel size和stride改变，这边会需要更改
            delay_out_Tr = out_tile_row_remain_Tr;
        }
        
        flag_psram_in = 0;
        for(char kernel_group_index = 0; kernel_group_index < kernel_num_repeat; kernel_group_index++){
            if(tile_index == tile_remain_start_index && kernel_group_index != 0){
                delay_out_Tr = out_tile_row_remain_Tr;
            }

            psram_in_addr3d = psram_in_addr2d; //进入channel循环前先用2d给3d赋值，将初始地址变成channel = 0的地方
            
            if(kernel_group_index == kernel_num_remain_start_index){
                Tm = kernel_num_remain;
                delay_Tm_flag = 1;
            }
            else if(kernel_group_index == 0){ //当kernel_num_remain_start_index = 0时，因为是else if，也不会进这个分支，所以是安全的
                Tm = ori_Tm;
            }
            
            if(pingpong[2] == 0){
                load_bias(BIAS_BUF_ADDR, (char *)b_conv, Tm);
                b_conv += Tm;
            }
            else if(pingpong[2] == 1){
                load_bias(BIAS_BUF_ADDR2, (char *)b_conv, Tm);
                b_conv += Tm;
            }
            
            for(char channel_group_index = 0; channel_group_index < channel_repeat; channel_group_index++){    
                if(channel_group_index == 0){
                    first_channel_group = 1;
                    last_channel_group = 0;
                } 
                else if(channel_group_index == channel_repeat - 1 && channel_repeat != 1){
                    first_channel_group = 0;
                    last_channel_group = 1;                
                }
                else{
                    first_channel_group = 0;
                    last_channel_group = 0;                
                }

                //h w c*n
                if(pingpong[1] == 0){
                    //第一二层因为c in和c out都不大于tile的Tn和Tm，所以就直接填入即可(防止传参消耗)
                    //第三层开始需要填c in和c out
                    load_weight(KERNEL_BUF_ADDR, (char *)w_conv, kernel_size, c_in, c_out);
                    w_conv += Tn * Tm;
                }
                else if(pingpong[1] == 1){
                    load_weight(KERNEL_BUF_ADDR2, (char *)w_conv, kernel_size, c_in, c_out);
                    w_conv += Tn * Tm;
                }

                if(pingpong[0] == 0){
                    psram_to_inpBuf(psram_in_addr3d, INP_BUF_ADDR, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                    is_up_pad, is_down_pad, is_left_pad, is_right_pad);
                }
                else if(pingpong[0] == 1){
                    psram_to_inpBuf(psram_in_addr3d, INP_BUF_ADDR2, TRow, TCol, inp_w, Tn, pre_layer_info.Tm, \
                                    is_up_pad, is_down_pad, is_left_pad, is_right_pad);
                }
                
                if(flag_psram_in == 0){ //针对Tn和上一层的Tm不相等的问题作出解决
                    psram_in_addr3d += Tn;
                }
                else if(flag_psram_in == 1){
                    psram_in_addr3d += pre_layer_info.Tm * (inp_w * inp_h) - Tn;
                }
                
                wait_cal_done();
                // psram_addr 考虑空间大小和存放feature map的方式，和如何能够准确的找出来对应tile的对应channel
                if(tile_index == 0 && kernel_group_index == 0 && channel_group_index == 0){
                    #ifdef OUT_DEBUG4
                        outBuf_to_psRam_test(OUR_BUF_ADDR, pre_layer_info.psram_addr, tile_output_save_pointer,\
                                        pre_layer_info.out_w, pre_layer_info.Tm, \
                                        pre_layer_info.out_Tr, pre_layer_info.out_Tc, tile_index, 0, layer_index);
                    #else
                        outBuf_to_psRam(OUR_BUF_ADDR, pre_layer_info.psram_addr, tile_output_save_pointer,\
                                        pre_layer_info.out_w, pre_layer_info.Tm, \
                                        pre_layer_info.out_Tr, pre_layer_info.out_Tc);
                    #endif 

                    //设置tile基本信息
                    set_tile_info(Tn, Tm, Tc, Tr);
                    set_quantize_scale(scale_a_single_i, scale_w_single, scale_b_single, \
                                       retune_single, scale_a_single_o);
                }
                else if(channel_group_index == 0){
                    #ifdef OUT_DEBUG4
                        outBuf_to_array_test(OUR_BUF_ADDR, net_output, tile_output_save_pointer, \
                                             out_w, delay_Tm, delay_out_Tr, out_Tc, \
                                             c_out, tile_index, kernel_group_index, layer_index);
                    #else
                        outBuf_to_array(OUR_BUF_ADDR, net_output, tile_output_save_pointer, \
                                        out_w, delay_Tm, delay_out_Tr, out_Tc, c_out);
                    #endif 
                    net_output += delay_Tm; //存完一组channel连续放的out_tile后不会跳一张图，
                                            //最后一层将所有channel都连续放
                    if(delay_Tm_flag == 1 && delay_Tm != ori_Tm){delay_Tm = ori_Tm; delay_Tm_flag = 0;}
                }

                if(tile_index == tile_remain_start_index){
                    set_tile_info(Tn, Tm, Tc, Tr);
                }

                if(kernel_group_index == kernel_num_remain_start_index){
                    set_tile_info(Tn, Tm, Tc, Tr);
                }

                //配置tile细节
                config_ctl_reg(is_up_pad, is_down_pad, is_left_pad, is_right_pad, \
                               pingpong[0], pingpong[1], pingpong[2], \
                               first_channel_group, last_channel_group, activ, pool);

                start_calculate(tile_index, layer_index);

                pingpong[0] = pingpong_invert(pingpong[0]);
                pingpong[1] = pingpong_invert(pingpong[1]);
                flag_psram_in = pingpong_invert(flag_psram_in);
            }
            pingpong[2] = pingpong_invert(pingpong[2]);
        }
        //判断下一个tile_index时取inp的地址应当如何变化
        //有些情况（虽然不在我们的模型里出现过这种情况）中左pad和右pad会同时出现，但上pad和下pad不同时出现，这种情况暂时不考虑
        //最后几层是四种pad都同时出现，这时tile只有一个，不需要考虑在平面上移动取值地址了
        if(is_right_pad == 1 && is_up_pad == 1){psram_in_addr2d += pre_layer_info.Tm * ((Tc + 1) + (Tr - 2) * inp_w);}//加上Tc后已经主动加上一行，再考虑pad，故-2
        //最后一列时不用tile倒退两格，而是进到下一行，所以+2，但因为右pad，所以又-1
        else if(is_right_pad == 1 && is_up_pad == 0){psram_in_addr2d += pre_layer_info.Tm * ((Tc + 1) + (Tr - 1) * inp_w);}
        else if(is_left_pad == 1){psram_in_addr2d += pre_layer_info.Tm * (Tc - 1);}
        else{psram_in_addr2d += pre_layer_info.Tm * Tc;}

        if(delay_Tm_flag == 1){delay_Tm = kernel_num_remain;}
    }
    if(out_tile_row_remain_Tr != 0){
        delay_out_Tr = out_tile_row_remain_Tr;
    }
    wait_cal_done();
    //因为最后一层所以最后还是等
    #ifdef OUT_DEBUG4
        outBuf_to_array_test(OUR_BUF_ADDR, net_output, tile_output_save_pointer, \
                            out_w, delay_Tm, delay_out_Tr, out_Tc, \
                            c_out, 0, 0, layer_index);
    #else
        outBuf_to_array(OUR_BUF_ADDR, net_output, tile_output_save_pointer, \
                        out_w, delay_Tm, delay_out_Tr, out_Tc, c_out);
    #endif 

    //layer_info.psram_addr = net_output; 不需要这个信息了
    layer_info.out_w = out_w;
    layer_info.Tm = Tm;
    layer_info.out_Tr = out_Tr;
    layer_info.out_Tc = out_Tc;

    return layer_info; //返回最后一个tile需要write到psram的addr，给下一层的一开始使用
}
