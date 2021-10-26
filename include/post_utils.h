#ifndef __POST_UTILS_H
#define __POST_UTILS_H

#include "box.h"

float sigmoid(float x);

float dequantize(char output, const char scale_a_single);

void softmax(float cls_lst[]);

int clamp(int input, int min, int max);

char cls_sort(int cls_lst[]);

void conf_sort(struct BOX src[], int size);

#endif