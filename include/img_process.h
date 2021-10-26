#ifndef __IMG_PROCESS_H
#define __IMG_PROCESS_H

#include <opencv2/imgcodecs/imgcodecs_c.h>
#include "box.h"

void image_load(int h, int w, IplImage *src, short int *camera_bram);

void image_show(int h, int w, IplImage *src, short int *camera_bram);

void draw_rectangle(struct BOX bbox_lst[], int bbox_lst_length, \
                    short int *camera_bram_pointer, int inp_w);

#endif