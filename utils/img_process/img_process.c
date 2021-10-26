#include "img_process.h"

void image_load(int h, int w, IplImage *src, short int *camera_bram){
    int tmpb, tmpg, tmpr;

    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            tmpb = (int)(cvGet2D(src, i, j).val[0] * 15. / 255.) << 8;            
            tmpg = (int)(cvGet2D(src, i, j).val[1] * 15. / 255.) << 4;
            tmpr = (int)(cvGet2D(src, i, j).val[2] * 15. / 255.);

            #ifdef INP_DEBUG
                if(j == 0 && i <= 16){
                    printf("i: %d , j: %d \n", i, j);
                    printf("tmpRED: %d \n", tmpr);
                    printf("red val: %f \n\n", cvGet2D(src, i, j).val[2]);
                }
            #endif

            *camera_bram = tmpb | tmpg | tmpr;
            //printf("%d \n", *camera_bram);
            camera_bram ++;
        }
    }
}

void image_show(int h, int w, IplImage *src, short int *camera_bram){
    CvScalar tmp_pixel;
    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            tmp_pixel.val[0] = ((double)(((*camera_bram) & 0x0f00) >> 8)) / 15 * 255;
            tmp_pixel.val[1] = ((double)(((*camera_bram) & 0x00f0) >> 4)) / 15 * 255;
            tmp_pixel.val[2] = ((double)((*camera_bram) & 0x000f)) / 15 * 255;

            cvSet2D(src, i, j, tmp_pixel);
            camera_bram ++;
        }
    }
    cvShowImage("example", src);
    cvWaitKey(0);
    cvReleaseImage(&src);
    cvDestroyWindow("example");
}

void draw_rectangle(struct BOX bbox_lst[], int bbox_lst_length, \
                    short int *camera_bram_pointer,
                    int inp_w){
    short int red = 0x000f;
    short int green = 0x00f0;
    short int pixel = 0x0000;
    int current_index = 0;
    short int *camera_bram_start = camera_bram_pointer;
    int bbox_left_up_index, bbox_right_up_index, bbox_left_down_index, bbox_right_down_index;
    while (current_index < bbox_lst_length){
        //printf("current_index: %d\n", current_index);
        if (!(bbox_lst[current_index].supression)){
            
            bbox_left_up_index = bbox_lst[current_index].x_min + bbox_lst[current_index].y_min * inp_w;
            bbox_right_up_index = bbox_lst[current_index].x_max + bbox_lst[current_index].y_min * inp_w;
            bbox_left_down_index = bbox_lst[current_index].x_min + bbox_lst[current_index].y_max * inp_w;
            bbox_right_down_index = bbox_lst[current_index].x_max + bbox_lst[current_index].y_max * inp_w;
            #ifdef DRAW_DEBUG
                printf("bbox_lst[current_index].x_min: %d \n", bbox_lst[current_index].x_min);
                printf("bbox_lst[current_index].y_min: %d \n", bbox_lst[current_index].y_min);
                printf("bbox_lst[current_index].x_max: %d \n", bbox_lst[current_index].x_max);
                printf("bbox_lst[current_index].y_max: %d \n", bbox_lst[current_index].y_max);
            #endif
            if(bbox_lst[current_index].cls == 0){pixel = red;}
            else{pixel = green;}

            #ifdef DRAW_DEBUG
                for(int index = 0; index <= bbox_lst[current_index].x_max - bbox_lst[current_index].x_min; index++){
                    *(camera_bram_pointer + index) = pixel;
                }
                for(int index = 0; index <= bbox_lst[current_index].y_max - bbox_lst[current_index].y_min; index++){
                    *(camera_bram_pointer + index) = pixel;
                }
            #endif

            for(int index = 0; index <= bbox_lst[current_index].x_max - bbox_lst[current_index].x_min; index++){
                    *(camera_bram_pointer + bbox_left_up_index + index) = pixel;
                    *(camera_bram_pointer + bbox_left_down_index + index) = pixel;
            }
            for(int index = 0; index <= bbox_lst[current_index].y_max - bbox_lst[current_index].y_min; index++){
                *(camera_bram_pointer + bbox_left_up_index + index * inp_w) = pixel;
                *(camera_bram_pointer + bbox_right_up_index + index * inp_w) = pixel;
            }
        }
        current_index++;
    }
}
