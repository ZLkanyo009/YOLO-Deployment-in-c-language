#ifndef __DECODE_H
#define __DECODE_H

#include "box.h"
#include "post_utils.h"
#include "weight.h"
#include "config.h"

struct BOX decode_txtytwth(float tx, float ty, float tw, float th, char cx, char cy, \
                           int inp_h, int inp_w ,char anchor_index);

int get_boxes(int inp_h, int inp_w, int out_h, int out_w, char anchor_num, \
              float conf_thresh, struct BOX bbox_lst[], \
              char *conf_pointer, char *cls_pointer, char *txtytwth_pointer);

#endif