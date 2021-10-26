#include "simulator.h"
//#define CALC_DEBUG 1

struct debug debug_info;
void set_offset(char iofs, char idir, char bofs, char bdir, char oofs, char odir){
    global_tile_ofs.input_ofs = iofs;
    global_tile_ofs.input_dir = idir;
    global_tile_ofs.bias_ofs = bofs;
    global_tile_ofs.bias_dir = bdir;
    global_tile_ofs.output_ofs = oofs;
    global_tile_ofs.output_dir = odir;
}

void set_tile_detail(char pad_pos, bool pingpong_i, bool pingpong_k, bool pingpong_b, bool first, bool last, bool activ, bool pool){
    global_tile_detail.pad_pos = pad_pos;
    global_tile_detail.pingpong_i = pingpong_i;
    global_tile_detail.pingpong_b = pingpong_b;
    global_tile_detail.pingpong_k = pingpong_k;
    global_tile_detail.first = first;
    global_tile_detail.last = last;
    global_tile_detail.activ = activ;
    global_tile_detail.pool = pool;

    int ibuf_base = pingpong_i ? TROW*TCOL : 0;

    int tr = global_tile_info.tr;
    int tc = global_tile_info.tc;
    int trow = tr + 2;
    int tcol = tc + 2;

    //printf("2 ?\n");

    //printf("pad_pos: %d\n", pad_pos);

    if(pad_pos & PADDING_UP){
        //printf("3 ? %d\n", ibuf_base);
        memset((char*)ibuf+ibuf_base*TN, 0, tcol*TN); 
        //printf("33 ?\n");
    }

    //printf("22 ?\n");

    if(pad_pos & PADDING_DOWN){
        memset((char*)ibuf+(ibuf_base+(trow-1)*tcol)*TN, 0, tcol*TN);
    }

    //printf("222 ?\n");

    if(pad_pos & PADDING_LEFT){
        for(int i = 0; i < trow; i++){
            //printf("left addr: %d\n", ibuf_base+i*tcol);
            memset((char*)ibuf+(ibuf_base+i*tcol)*TN, 0, TN);
        }
    }

    //printf("2222 ?\n");
            // printf("addr :%d \n", 24);
            //     for(int test_i = 0; test_i < 16; test_i++){
            //         printf("ibuf :%d \n",ibuf[24][test_i]);
            //     }
            //     printf("end \n");
    
    if(pad_pos & PADDING_RIGHT){
        for(int i = 0; i < trow; i++){
            // printf("right addr: %d\n", ibuf_base+i*tcol+tcol-1);
            memset((char*)ibuf+(ibuf_base+i*tcol+tcol-1)*TN, 0, TN);
        }
    }

    //printf("22222 ?\n");
}

void set_tile_info(char tn, char tm, char tc, char tr){
    global_tile_info.tn = tn;
    global_tile_info.tm = tm;
    global_tile_info.tr = tr;
    global_tile_info.tc = tc;
}


void oofs_func(short *src, char* des, int n, int odir, int oofs){
    for(int i = 0; i < n; i++){
        // printf("%d; %d; %d\n", odir, src[i], oofs);
        if(odir > 0){
            des[i] = (char)(src[i] << oofs);
        } else{
            des[i] = (char)(src[i] >> oofs);
        }
        // printf("%d;\n", des[i]);
    }
}

void activ_func(char *src, int n){
    for(int i = 0; i < n; i++){
        src[i] = (src[i] >= 0) ? src[i] : src[i]/8;
    }
}

void pool_func(char *op0, char *op1, char *op2, char *op3, char *des, int n){
    for(int i = 0; i < n; i++){
        char o0 = (op0[i] > op1[i]) ? op0[i] : op1[i];
        char o1 = (op2[i] > op3[i]) ? op2[i] : op3[i];
        des[i] = (o0 > o1) ? o0 : o1;
    }
}

struct debug{
    int tile_index;
    int layer_index;
};


static void* calculate(void *arg){
    
    int tr = global_tile_info.tr;
    int tc = global_tile_info.tc;
    int tm = global_tile_info.tm;
    int tn = global_tile_info.tn;

    int tcol = tc + 2;

    int iofs = global_tile_ofs.input_ofs;
    int idir = global_tile_ofs.input_dir;
    int bofs = global_tile_ofs.bias_ofs;
    int bdir = global_tile_ofs.bias_dir;
    int oofs = global_tile_ofs.output_ofs;
    int odir = global_tile_ofs.output_dir;

    bool first = global_tile_detail.first;
    bool pool = global_tile_detail.pool;
    bool activ = global_tile_detail.activ;
    bool last = global_tile_detail.last;
    bool pingpong_i = global_tile_detail.pingpong_i;
    bool pingpong_k = global_tile_detail.pingpong_k;
    bool pingpong_b = global_tile_detail.pingpong_b;

    int ibuf_base = pingpong_i ? TROW*TCOL : 0;
    int kbuf_base = pingpong_k ? KERNEL*KERNEL : 0;
    int bbuf_base = pingpong_b ? 1: 0;
    //printf("tn : %d \n", tn);

#ifdef CALC_DEBUG 
    if(debug_info.tile_index == 7 && debug_info.layer_index == 1){
            printf("=====tile index: %d ======== \n", debug_info.tile_index);
            // printf("up: %d, down: %d, left: %d, right: %d \n",is_up_pad,  is_down_pad,  is_left_pad,  is_right_pad);
            // if(test == 1){
            //     printf("=========== 2 ==================\n");
            // }
            // else{
            //     printf("=========== 1 ==================\n");
            // }
            for(int channel = 0; channel < 16; channel++){
                for(int test_inp = ibuf_base; test_inp < ibuf_base + 22*18; test_inp++){
                    printf("%d\t", ibuf[test_inp][channel]);
                    if((test_inp+1) % 22 == 0){
                        printf("\n");
                    }
                }
                printf("\n");
            }
            printf("final :%d \n", ibuf[ibuf_base + 88][0]);
            printf("inp_buf_start: %d \n", ibuf_base);
        }
#endif

    for(int r = 0; r < tr; r++){
        for (int c = 0; c < tc; c++){
            for (int m = 0; m < tm; m++){

                short bias_extend = bdir ? ((short)bbuf[bbuf_base][m]) << bofs : ((short)bbuf[bbuf_base][m]) >>bofs;
                short local_psum = first ? bias_extend : psbuf[c+tc*r][m];
                //printf("bias : %d \n", (short)bbuf[bbuf_base][m]);
                //printf("bias_extend : %d \n", local_psum);

                for (int n = 0; n < tn; n++){
                    for (int i = 0; i < KERNEL; i++){
                        for (int j = 0; j < KERNEL; j++){
                            #ifdef CALC_DEBUG 
                            if(debug_info.tile_index == 7 && debug_info.layer_index == 1 && m == 0 && r == 2 && c == 0){
                                printf("addr: %d, ", ibuf_base+c+j+tcol*(r+i));
                                printf("ibuf: %d, ", ibuf[ibuf_base+c+j+tcol*(r+i)][n]);
                                printf("kbuf: %d, ", kbuf[kbuf_base+j+i*KERNEL][n+m*TN]);
                                printf("bbuf: %d, ", bbuf[bbuf_base][m]);
                                printf("bofs: %d, ", bofs);
                                printf("iofs: %d, ", iofs);
                                printf("oofs: %d, ", oofs);
                                }
                            #endif

                            if(idir == 0){
                                local_psum += ((short)ibuf[ibuf_base+c+j+tcol*(r+i)][n]*(short)kbuf[kbuf_base+j+i*KERNEL][n+m*TN]) >> iofs;
                                #ifdef CALC_DEBUG 
                                    //printf("psum part : %d \n", (ibuf[ibuf_base+c+j+tcol*(r+i)][n]*kbuf[kbuf_base+j+i*KERNEL][n+m*TN]) >> iofs);
                                #endif
                            }
                            else{
                                local_psum += ((short)ibuf[ibuf_base+c+j+tcol*(r+i)][n]*(short)kbuf[kbuf_base+j+i*KERNEL][n+m*TN]) << iofs;
                                #ifdef CALC_DEBUG 
                                    //printf("psum part : %d \n", (ibuf[ibuf_base+c+j+tcol*(r+i)][n]*kbuf[kbuf_base+j+i*KERNEL][n+m*TN]) << iofs);
                                #endif
                            }
                            #ifdef CALC_DEBUG
                            if(debug_info.tile_index == 7 && debug_info.layer_index == 1 && m == 0 && r == 2 && c == 0){
                                printf("local_psum: %d \n\n", local_psum);
                                }
                            #endif
                        }
                    }
                }
                //printf("one finished, local_psum : %d \n\n", local_psum);
                #ifdef CALC_DEBUG
                    //printf("one finished \n\n", local_psum);
                #endif

                psbuf[c+tc*r][m] = local_psum;
                
                //printf("r:%d, c:%d, m:%d, psbuf:%d\n", r, c, m, psbuf[c+tc*r][m]);
            }
            // pool / activ / obuf;
            if(last){
                char output[tm];
                bool pool_needed = ((r % 2) == 1) && ((c % 2) == 1);
                // printf("r:%d; c:%d; ", r, c);
                // printf("psbuf: %d; ", psbuf[c+tc*r][0]);
                // printf("activ: %d; pool : %d; pool_needed: %d\n", activ, pool, pool_needed);
                if(activ && pool){
                    if(pool_needed){
                        char op0[tm];
                        char op1[tm];
                        char op2[tm];
                        char op3[tm];
                        // for(int debug_i = 0; debug_i < tm; debug_i++){
                        //     printf("%d; \n", psbuf[c-1+tc*(r-1)][debug_i]);}
                        // printf("\n");
                        oofs_func(psbuf[c-1+tc*(r-1)], op0, tm, odir, oofs);
                        // for(int debug_i = 0; debug_i < tm; debug_i++)
                        //     printf("%d; \n", op0[debug_i]);
                        // printf("\n");

                        oofs_func(psbuf[c+tc*(r-1)], op1, tm, odir, oofs);
                        oofs_func(psbuf[c-1+tc*r], op2, tm, odir, oofs);
                        //printf("psbuf_ofs: %d; o_ofs: %d; ", op2[0], oofs);
                        oofs_func(psbuf[c+tc*r], op3, tm, odir, oofs);
                        activ_func(op0, tm);
                        // for(int debug_i = 0; debug_i < tm; debug_i++)
                        //     printf("%d; \n", op0[debug_i]);
                        // printf("\n");

                        activ_func(op1, tm);
                        activ_func(op2, tm);
                        activ_func(op3, tm);

                        #ifdef CALC_DEBUG
                        if(debug_info.tile_index == 7 && debug_info.layer_index == 1){
                            printf("activ: %d, %d\n", op0[0], op1[0]);
                            printf("activ: %d, %d\n\n", op2[0], op3[0]);
                        }
                        #endif

                        pool_func(op0, op1, op2, op3, output, tm);

                        //printf("output: %d\n", output[0]);
                        memcpy(obuf[c/2+tc/2*((r-1)/2)], output, tm);
                    }
                } else if(activ && !pool) {

                    oofs_func(psbuf[c+tc*r], output, tm, odir, oofs);
                    activ_func(output, tm);
                    // for(int m = 0; m < tm; m ++)
                    //     printf("r:%d, c:%d, m:%d, output:%d\n", r, c, m, output[m]);
                    memcpy(obuf[c+tc*r], output, tm);
                } else if(!activ) {
                    
                    oofs_func(psbuf[c+tc*r], output, tm, odir, oofs);
                    // for(int m = 0; m < tm; m ++)
                    //     printf("r:%d, c:%d, m:%d, output:%d\n", r, c, m, output[m]);
                    memcpy(obuf[c+tc*r], output, tm);
                }
            }

        }
    }

    return 0;
}



pthread_t cal_pthread;

void start_calculate(int tile_index, int layer_index){
    // thread calculate
    debug_info.tile_index = tile_index;
    debug_info.layer_index = layer_index;
    pthread_create(&cal_pthread, NULL, calculate, NULL);
}


void wait_cal_done(){
    //pthread_join();
    pthread_join(cal_pthread, NULL);
}


// int main(){
//     char y = -9;

//     // printf("%d\t", y >> 3);
//     // printf("%d\t", y/8);

//     char tile_test[16][1];

//     int i;

//     for (i = 0; i < 16; i++){
//         tile_test[i][0] = i;
//     }

//     char kernel_test[9][512];

//     memset(kernel_test, 0, 9*512);

//     kernel_test[0][0] = 1;
//     kernel_test[0][16] = 1;
//     kernel_test[0][32] = 0;
//     kernel_test[1][0] = 0;
//     kernel_test[1][16] = 1;
//     kernel_test[1][32] = 0;
//     kernel_test[2][0] = 1;
//     kernel_test[2][16] = 1;
//     kernel_test[2][32] = 1;
//     kernel_test[3][0] = 0;
//     kernel_test[3][16] = 1;
//     kernel_test[3][32] = 0;
//     kernel_test[4][0] = 1;
//     kernel_test[4][16] = 1;
//     kernel_test[4][32] = 0;
//     kernel_test[5][0] = 0;
//     kernel_test[5][16] = 1;
//     kernel_test[5][32] = 1;
//     kernel_test[6][0] = 0;
//     kernel_test[6][16] = 1;
//     kernel_test[6][32] = 0;
//     kernel_test[7][0] = 1;
//     kernel_test[7][16] = 1;
//     kernel_test[7][32] = 0;
//     kernel_test[8][0] = 1;
//     kernel_test[8][16] = 1;
//     kernel_test[8][32] = 1;

//     char bias_test[1][3];

//     bias_test[0][0] = 0;
//     bias_test[0][1] = 1;
//     bias_test[0][2] = 1;

//     for (i = 0; i < 16; i++){
//         write_input_buffer(tile_test[i], i, 0);
//     }

//     for (i = 0; i < 9; i++){
//         write_kernel_buffer(kernel_test[i], i, 511);
//     }

//     write_bias_buffer(bias_test[0], 0, 2);

//     set_offset(0, 0, 0, 0, 0, 0);
//     set_tile_detail(NO_PADDING, 0, 0, 0, 0, 1, 1, 0);
//     set_tile_info(1, 3, 2, 2);

//     start_calculate();

//     wait_cal_done();

//     for(i = 0; i < 4; i ++){
//         for(int j = 0; j < 3; j ++){
//             printf("%d\t", obuf[i][j]);
//         }
//         printf("\n");
//     }


// }



/*
int main(int argc, char const *argv[])
{

    char test[TN];

    char test_out[TN];

    for(int i = 0; i < TN; i++){
        test[i] = i;
    }

    printf("test_in\t:\n");
    for(int i = 0; i < TN; i++){
        printf("%d\t", test[i]);
    }

    printf("\n");

    write_psram(test, 1, TN-1);


    printf("psram\t:\n");
    for(int i = 0; i < TN; i++){
        printf("%d\t", psram[1+i]);
    }

    printf("\n");

    read_psram(test_out, 1, TN-1);

    printf("test_out\t:\n");
    for(int i = 0; i < TN; i++){
        printf("%d\t", test_out[i]);
    }


    printf("\n");
    
    return 0;
}
*/