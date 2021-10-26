#include "post_utils.h"

float sigmoid(float x){
    float result_data = (float) (1 / (exp((double) (-x)) + 1));
    return result_data;
}

float dequantize(char output, const char scale_a_single){
    double scale_a_single_pow = pow(2, (double) scale_a_single);
    float output_fp = output / scale_a_single_pow;
    return output_fp;
}

void softmax(float cls_lst[]){
    int i = 0;
    float sum = 0;
    for(i = 0; i < 2; i++)
    {
        cls_lst[i] = exp((double) cls_lst[i]);
        sum += cls_lst[i];
    }
    for(i = 0; i < 2; i++){
        cls_lst[i] = cls_lst[i] / sum;
    }
}

int clamp(int input, int min, int max){
    if(input < min){input = min;}
    if(input > max){input = max;}
    return input;
}

char cls_sort(int cls_lst[])
{
    if(cls_lst[0] >= cls_lst[1]){
        return 0;
    }
    else if(cls_lst[0] < cls_lst[1]){
        return 1;
    }
}

void conf_sort(struct BOX src[], int size)
{
    int i, j;
    for (i = 0; i < size - 1; i++){
        for (j = i + 1; j < size; j++){
            if (src[j].conf > src[i].conf){
                struct BOX tmp = src[j];
                src[j] = src[i];
                src[i] = tmp;
            }             
        }
    }
}