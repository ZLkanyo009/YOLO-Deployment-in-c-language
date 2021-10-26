#include "cores_utils.h"

void pixel_norm_quantize(short int pixel_0bgr, const char scale_a_single,  char *pixel_all_channel_q_pointer){

    float pixel_norm;
    double scale_a_single_pow = pow(2, (double) scale_a_single);
    #ifdef NORM_DEBUG
        //printf("scale_a_single_pow : %f \n",  scale_a_single_pow);
    #endif

    //red channel
    pixel_norm = (float)(pixel_0bgr & 0x000f);
    pixel_norm /= 15.;
    pixel_norm -= 0.485;
    pixel_norm /= 0.229;

    *pixel_all_channel_q_pointer = (char) (pixel_norm * scale_a_single_pow);//需要把第一层的scale变成2的指数幂
    
    #ifdef NORM_DEBUG
        // printf("pixel_norm1.11 : %d \n",  (pixel_0bgr & 0x000f));
        // printf("pixel_norm1.12 : %f \n",  pixel_norm);
        printf("%d\t",  (pixel_0bgr & 0x000f));
    #endif

    //green channel
    pixel_norm = (float)((pixel_0bgr & 0x00f0) >> 4);
    pixel_norm /= 15.;
    pixel_norm -= 0.456;
    pixel_norm /= 0.224;

    *(pixel_all_channel_q_pointer + 1) = (char) (pixel_norm * scale_a_single_pow);//需要把第一层的scale变成2的指数幂
    
    #ifdef NORM_DEBUG
        // printf("pixel_norm2.11 : %d \n",  (pixel_0bgr & 0x00f0) >> 4);
        // printf("pixel_norm2.12 : %f \n",  pixel_norm);
        // printf("pixel_norm2.2 : %d \n",  *(pixel_all_channel_q_pointer + 1));
    #endif
    
    //blue channel
    pixel_norm = (float)((pixel_0bgr & 0x0f00) >> 8);
    pixel_norm /= 15.; //应该是 / 16，因为是 4 4 4
    pixel_norm -= 0.406;
    pixel_norm /= 0.225;

    *(pixel_all_channel_q_pointer + 2) = (char) (pixel_norm * scale_a_single_pow);//需要把第一层的scale变成2的指数幂
    
    #ifdef NORM_DEBUG
        // printf("pixel_norm3.11 : %d \n",  (pixel_0bgr & 0x0f00) >> 8);
        // printf("pixel_norm3.12 : %f \n",  pixel_norm);
        // printf("pixel_norm3.2 : %d \n",  *(pixel_all_channel_q_pointer + 2));
        // printf("\n");
    #endif
}

bool pingpong_invert(bool pingpong){
    if(pingpong == 0){
        pingpong = 1;
    }
    else if(pingpong == 1){
        pingpong = 0;
    }
    return pingpong;
}

void set_quantize_scale(const char scale_a_single_i, const char scale_w_single, const char scale_b_single, \
                        const char retune_single, const char scale_a_single_o){
    char iofs = scale_a_single_i + scale_w_single - retune_single;
    char bofs = scale_b_single - retune_single;
    char oofs = retune_single - scale_a_single_o;

    char idir = 0;
    char bdir = 0;
    char odir = 0;

    if(iofs < 0){
        idir = 1;
        iofs = 0 - iofs;
    }
    if(bofs < 0){
        bdir = 1;
        bofs = 0 - bofs;
    }
    if(oofs < 0){
        odir = 1;
        oofs = 0 - oofs;
    }

    set_offset(iofs, idir, bofs,  bdir,  oofs, odir);

    #ifdef SET_SCALE_DEBUG
        printf("scale :%d, %d, %d, %d, %d \n", scale_a_single_i, scale_w_single, scale_b_single,
                                               retune_single, scale_a_single_o);
        printf("offset :%d, %d, %d, %d, %d, %d \n", iofs, idir, bofs,
                                                    bdir, oofs, odir);
    #endif
}

void config_ctl_reg(bool is_up_pad, bool is_down_pad, bool is_left_pad, bool is_right_pad, \
                    bool pingpong_i, bool pingpong_w, bool pingpong_b, \
                    bool first_channel_group, bool last_channel_group, bool activ, bool pool){
    char pad_pos = NO_PADDING;
    if(is_up_pad == 1){
        pad_pos = pad_pos | PADDING_UP;
    }
    if(is_down_pad == 1){
        pad_pos = pad_pos | PADDING_DOWN;
    }
    if(is_left_pad == 1){
        pad_pos = pad_pos | PADDING_LEFT;
    }
    if(is_right_pad == 1){
        pad_pos = pad_pos | PADDING_RIGHT;
    }

    #ifdef SET_TILE_DETAIL_DEBUG
    // printf("addr :%d \n", 24);
    //     for(int test_i = 0; test_i < 16; test_i++){
    //         printf("ibuf :%d \n",ibuf[24][test_i]);
    //     }
    //     printf("end \n");
    #endif
    
    set_tile_detail(pad_pos, pingpong_i, pingpong_w, pingpong_b, first_channel_group, last_channel_group, activ, pool);
    
    #ifdef SET_TILE_DETAIL_DEBUG
    // printf("addr :%d \n", 24);
    //     for(int test_i = 0; test_i < 16; test_i++){
    //         printf("ibuf :%d \n",ibuf[24][test_i]);
    //     }
    //     printf("end \n");
    #endif
}