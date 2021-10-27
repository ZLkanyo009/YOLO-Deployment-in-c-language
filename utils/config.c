#include "config.h"

int *vga_bram = 0x42000000;

const float anchor_size[5][2] = {{0.41841, 0.627615}, {0.941422, 2.067525}, \
                                 {3.1013, 4.2606}, {5.11965, 7.58616}, \
                                 {8.83545, 11.51355}};
// const float anchor_size[5][2] = {{0.27894, 0.49337}, {0.8669, 1.37835}, \
//                                  {1.82727, 2.8404}, {3.4131, 5.05744}, \
//                                  {5.8903, 7.6757}};

const int pic_width = 320;
const int pic_height = 240;
const int out_width = 20;
const int out_height = 15;

const char stride = 16;

const float conf_thresh = 0.121, nms_thresh = 0.3;
const char anchor_num = 5;

