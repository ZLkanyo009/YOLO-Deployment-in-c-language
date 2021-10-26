#include <stdint.h> // uint32
#include <stdlib.h> // malloc
#include <stdbool.h>
#include <math.h>
#include "weight.h"
#include "cores.h"
#include "img_process.h"
#include "config.h"
#include "decode.h"
#include "post_utils.h"
#include "box.h"

void yolo_forward(const int inp_w, int inp_h, const int out_w, const int out_h, 
                  const float conf_thresh, const float nms_thresh, \
                  const char anchor_num, \
                  short int *camera_bram_pointer, int *vga_bram_pointer){
    const char TRow = 18;
    const char TCol = 22;
    const char Tr = 16;
    const char Tc = 20;
    const char Tm = 32;
    const char Tn = 16;

    int anchor_total = anchor_num * out_w * out_h; //anchor_num * out_w * out_h;

    bool is_up_pad, is_down_pad;
    bool is_left_pad, is_right_pad;

    struct INFO pre_layer_info;


    int *tile_output_save = (int *) malloc(Tm); //取一个像素点channel=Tm
    int *tile_output_save_pointer = tile_output_save;

    bool activ, pool;
    bool pingpong[3] = {0, 0, 0}; //0:pingpong_i, 1:pingpong_w, 2:pingpong_b
    
    #ifdef DEBUG
        printf("first conv! \n");
    #endif
    pre_layer_info = first_conv(320, 240, Tr, Tc, 16, 3, TRow, TCol, 0, \
                                camera_bram_pointer, tile_output_save_pointer, \
                                (char *)b_conv0, (char *)w_conv0, \
                                PSRAM_ADDR, \
                                1, 1, pingpong, 3);

    // //PSRAM_ADDR2和psram每过一层对换一下，做pingpong操作
    pre_layer_info = second_conv(160, 120, Tr, Tc, Tm, Tn, TRow, TCol, 1, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv1, (char *)w_conv1, \
                                 PSRAM_ADDR, PSRAM_ADDR2, pre_layer_info, \
                                 1, 1, pingpong, 3);

    pre_layer_info = conv_normal(80, 60, 32, 64, Tr, Tc, Tm, Tn, TRow, TCol, 2, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv2, (char *)w_conv2, \
                                 PSRAM_ADDR2, PSRAM_ADDR, pre_layer_info, \
                                 1, 0, pingpong, 3);

    pre_layer_info = conv_normal(80, 60, 64, 64, Tr, Tc, Tm, Tn, TRow, TCol, 3, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv3, (char *)w_conv3, \
                                 PSRAM_ADDR, PSRAM_ADDR2, pre_layer_info, \
                                 1, 1, pingpong, 3);
    
    pre_layer_info = conv_normal(40, 30, 64, 128, Tr, Tc, Tm, Tn, TRow, TCol, 4, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv4, (char *)w_conv4, \
                                 PSRAM_ADDR2, PSRAM_ADDR, pre_layer_info, \
                                 1, 0, pingpong, 3);
    
    pre_layer_info = conv_normal(40, 30, 128, 128, Tr, Tc, Tm, Tn, TRow, TCol, 5, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv5, (char *)w_conv5, \
                                 PSRAM_ADDR, PSRAM_ADDR2, pre_layer_info, \
                                 1, 1, pingpong, 3);
                                 
    // 测试一下下一层的输出是否也有同样问题：有+-2误差，目前认为是scale右移造成的小数位损失误差
    // 前几层误差小是因为右移的位数小，并且channel repeat少
    pre_layer_info = conv_normal(20, 15, 128, 256, 15, Tc, Tm, Tn, 17, TCol, 6, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv6, (char *)w_conv6, \
                                 PSRAM_ADDR2, PSRAM_ADDR, pre_layer_info, \
                                 1, 0, pingpong, 3);

    pre_layer_info = conv_normal(20, 15, 256, 256, 15, Tc, Tm, Tn, 17, TCol, 7, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv7, (char *)w_conv7, \
                                 PSRAM_ADDR, PSRAM_ADDR2, pre_layer_info, \
                                 1, 0, pingpong, 3);
    
    pre_layer_info = conv_normal(20, 15, 256, 256, 15, Tc, Tm, Tn, 17, TCol, 8, \
                                 tile_output_save_pointer, \
                                 (char *)b_conv8, (char *)w_conv8, \
                                 PSRAM_ADDR2, PSRAM_ADDR, pre_layer_info, \
                                 1, 0, pingpong, 3);
    
    char *net_output = (char *) malloc(35 * 20 * 15);
    char *net_output_pointer = net_output; // h, w, c
    pre_layer_info = conv_last(20, 15, 256, 35, 15, Tc, Tm, Tn, 17, TCol, 9, \
                               tile_output_save_pointer, \
                               (char *)b_conv9, (char *)w_conv9, \
                               PSRAM_ADDR, net_output_pointer, pre_layer_info, \
                               0, 0, pingpong, 3); 
    free(tile_output_save_pointer);
    #ifdef OUT_DEBUG5
        for(int h = 0; h < 15; h++){
            for(int w = 0; w < 20; w++){
                for(int c = 0; c < 35; c++){
                    printf("%d\n", net_output_pointer[c + w * 35 + h * 20 * 35]);
                }
            }
        }
    #endif
    
    struct BOX bbox_lst[anchor_total];  //后续可以考虑使用链表

    int bbox_lst_length = get_boxes(inp_h, inp_w, out_h, out_w, anchor_num, \
                                    conf_thresh, bbox_lst, \
                                    net_output_pointer, (net_output_pointer + 5), \
                                    (net_output_pointer + 15));    
    free(net_output);
    #ifdef DRAW_DEBUG
        printf("bbox_lst_length: %d\n", bbox_lst_length);
    #endif

    conf_sort(bbox_lst, bbox_lst_length);
    
    int NMS_length = NMS(bbox_lst, bbox_lst_length, nms_thresh);

    draw_rectangle(bbox_lst, bbox_lst_length, camera_bram_pointer, inp_w);
    //memcpy(vga_bram_pointer, camera_bram_pointer, 76800*2);    
}

int main()
{
    int *dest_bram = vga_bram;
    short int *camera_bram = (short int *) malloc(153600); // 320*240*16/8
    char *ori_pic = (char *) malloc(307200);

    IplImage *src = cvLoadImage("pic.png", -1); 
    const int inp_h = src->height;
	const int inp_w = src->width;
    const int out_h = 15;
    const int out_w = 20;

    #ifdef DEBUG
        printf("width = %d \n", src->width);
        printf("height = %d \n", src->height);
    #endif
    image_load(inp_h, inp_w, src, camera_bram);
    //image_show(inp_h, inp_w, src, camera_bram);

    const float conf_thresh = 0.04, nms_thresh = 0.5;
    const char anchor_num = 5;

    yolo_forward(inp_w, inp_h, out_w, out_h, \
                 conf_thresh, nms_thresh, anchor_num, \
                 (short int *)camera_bram, dest_bram);
    image_show(inp_h, inp_w, src, camera_bram);
    return 0;
}