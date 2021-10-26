#include "box.h"

//传入两个候选框变量某一维度坐标值，返回重叠部分的宽和高
int overlap(int a_min, int a_max, int b_max, int b_min) {    
    int sum_sides = a_max - a_min + b_max - b_min;    //同一方向两个边长之和
    int new_sides = MAX(a_max, b_max) - MIN(a_min, b_min);    //同一方向长度并集
    return  sum_sides - new_sides;
}
    
//传入两个候选框变量a、b，返回相交面积值
int box_intersection(int a_x_max, int a_x_min, int a_y_max, int a_y_min,\
                     int b_x_max, int b_x_min, int b_y_max, int b_y_min)    
{
    int w = overlap(a_x_min, a_x_max, b_x_max, b_x_min);
    int h = overlap(a_y_min, a_y_max, b_y_max, b_y_min);
    if (w <= 0 || h <= 0) return 0;
    int area = w * h;
    return w * h;
}

//传入两个候选框变量a、b，返回并集面积值
                //右下角x 左上角x 右下角y 左上角y
int box_union(int a_x_max, int a_x_min, int a_y_max, int a_y_min,\
                int b_x_max, int b_x_min, int b_y_max, int b_y_min) {    
    int insersection = box_intersection(a_x_max, a_x_min, a_y_max, a_y_min,\
                                          b_x_max, b_x_min, b_y_max, b_y_min);
    int areaA = (a_x_max - a_x_min) * (a_y_max - a_y_min);    
    //printf("areaA %f \n", areaA);
    int areaB = (b_x_max - b_x_min) * (b_y_max - b_y_min);
    //printf("areaB %f \n", areaB);
    int area = areaA + areaB - insersection;
    return area;
}

float box_iou(struct BOX a,\
              struct BOX b)
{
    return (float) box_intersection(a.x_max, a.x_min, a.y_max, a.y_min, b.x_max, b.x_min, b.y_max, b.y_min) / \
           (float) box_union(a.x_max, a.x_min, a.y_max, a.y_min, b.x_max, b.x_min, b.y_max, b.y_min);    
}

int NMS(struct BOX src[], int anchor_num, float nms_thresh){
    int max_index = 0, current_index = 0;
    int j;
    float iou;
    struct BOX tmp;
    while (current_index < anchor_num) {    //探究一轮循环的方法，与所以输出框比较，递归？？
        //printf("current_index: %d\n", current_index);
        if (!(src[current_index].supression)) {
            tmp = src[current_index];
            for (j = current_index + 1; j < anchor_num; j++) {
                iou = box_iou(tmp, src[j]);
                if (iou >= nms_thresh)
                    src[j].supression = 1;
            }
            max_index++;
        }
        current_index++;
    }
    return (max_index + 1);
}