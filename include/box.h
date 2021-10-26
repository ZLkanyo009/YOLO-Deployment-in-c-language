#ifndef __BOX_H
#define __BOX_H 

#include <stdbool.h>

#define MAX(a,b)(a>b?a:b)
#define MIN(a,b)(a<=b?a:b)

struct BOX{
    int x_max;
    int x_min;
    int y_max;
    int y_min;
    float conf;
    char cls;
    bool supression; //是否被nms抑制
};

//传入两个候选框变量某一维度坐标值，返回重叠部分的宽和高
int overlap(int a_min, int a_max, int b_max, int b_min);
    
//传入两个候选框变量a、b，返回相交面积值
int box_intersection(int a_x_max, int a_x_min, int a_y_max, int a_y_min,\
                     int b_x_max, int b_x_min, int b_y_max, int b_y_min);

//传入两个候选框变量a、b，返回并集面积值
//右下角x 左上角x 右下角y 左上角y
int box_union(int a_x_max, int a_x_min, int a_y_max, int a_y_min,\
              int b_x_max, int b_x_min, int b_y_max, int b_y_min);

float box_iou(struct BOX a, struct BOX b);

#endif