#include "config.h"

int *vga_bram = 0x42000000;

const float anchor_size[5][2] = {{0.27894, 0.49337}, {0.8669, 1.37835}, \
                                 {1.82727, 2.8404}, {3.4131, 5.05744}, \
                                 {5.8903, 7.6757}};
const char stride = 16;