#include "decode.h"

struct BOX decode_txtytwth(float tx, float ty, float tw, float th, char cx, char cy, \
                           int inp_h, int inp_w ,char anchor_index){
    struct BOX bbox = {0, 0, 0, 0, 0, 0, 0}; 
    float x_center = (sigmoid(tx) + cx) * stride;
    float y_center = (sigmoid(ty) + cy) * stride;
    float w = anchor_size[anchor_index][0] * exp((double) tw) * stride;
    float h = anchor_size[anchor_index][1] * exp((double) th) * stride;
    #ifdef TXTYTWTH_DEBUG
        printf("x_center:%f\n", x_center);
        printf("y_center:%f\n", y_center);
        printf("w_c:%f\n", w);
        printf("h_c:%f\n", h);
    #endif
    bbox.x_min = clamp((int)(x_center - w / 2), 0, inp_w);
    bbox.x_max = clamp((int)(x_center + w / 2), 0, inp_w);
    bbox.y_min = clamp((int)(y_center - h / 2), 0, inp_h);
    bbox.y_max = clamp((int)(y_center + h / 2), 0, inp_h);
    return bbox;
}

int get_boxes(int inp_h, int inp_w, int out_h, int out_w, char anchor_num, \
              float conf_thresh, struct BOX bbox_lst[], \
              char *conf_pointer, char *cls_pointer, char *txtytwth_pointer){
    float conf, tx, ty, tw, th;

    float cls_lst[2];
    char cls;

    int anchor_index = 0;
    int selected_anchor_num = 0;

    for(int h = 0; h < out_h; h++){
        for(int w = 0; w < out_w; w ++){ 
            for(char anchor = 0; anchor < anchor_num; anchor++){
                conf = dequantize(*conf_pointer, scale_a[10]);
                #ifdef POST_DEBUG
                    printf("h:%d ,w:%d, anchor:%d\n", h, w, anchor);
                #endif

                #ifdef CONF_DEBUG
                    printf("*conf: %f\n", conf);
                #endif
                conf = sigmoid(conf);
                #ifdef CONF_DEBUG
                    //printf("*cls_pointer1: %d\n", *cls_pointer);
                    printf("*sigmoid conf: %f\n", conf);
                #endif
                conf_pointer ++;

                cls_lst[0] = dequantize(*cls_pointer, scale_a[10]);
                #ifdef CONF_DEBUG
                    //printf("*cls_pointer1: %d\n", *cls_pointer);
                    printf("*cls_lst[0]: %f\n", cls_lst[0]);
                #endif
                cls_pointer ++;
                cls_lst[1] = dequantize(*cls_pointer, scale_a[10]);
                #ifdef CONF_DEBUG
                    //printf("*cls_pointer1: %d\n", *cls_pointer);
                    printf("*cls_lst[1]: %f\n", cls_lst[1]);
                #endif
                cls_pointer ++;
                softmax(cls_lst);
                cls = cls_sort(cls_lst);
                conf = conf * cls_lst[(int) cls];
                #ifdef CONF_DEBUG
                    printf("softmax conf[0]: %f\n", cls_lst[0]);
                    printf("softmax conf[1]: %f\n", cls_lst[1]);
                    //printf("*cls_pointer1: %d\n", *cls_pointer);
                    printf("*conf_score: %f\n\n", conf);
                #endif

                #ifdef TXTYTWTH_DEBUG_ALL
                    conf = 1.0;
                #endif
                if(conf > conf_thresh){
                    tx = dequantize((*txtytwth_pointer), scale_a[10]);
                    #ifdef TXTYTWTH_DEBUG
                        printf("tx: %f\n", tx);
                    #endif
                    txtytwth_pointer ++;
                    ty = dequantize((*txtytwth_pointer), scale_a[10]);
                    #ifdef TXTYTWTH_DEBUG
                        printf("ty: %f\n", ty);
                    #endif
                    txtytwth_pointer ++;
                    tw = dequantize((*txtytwth_pointer), scale_a[10]);
                    #ifdef TXTYTWTH_DEBUG
                        printf("tw: %f\n", tw);
                    #endif
                    txtytwth_pointer ++;
                    th = dequantize((*txtytwth_pointer), scale_a[10]);
                    #ifdef TXTYTWTH_DEBUG
                        printf("th: %f\n", th);
                    #endif
                    txtytwth_pointer ++;

                    struct BOX bbox = decode_txtytwth(tx, ty, tw, th, w, h, inp_h, inp_w, anchor);
                    
                    #ifdef TXTYTWTH_DEBUG
                        printf("xmin: %d\n", bbox.x_min);
                        printf("ymin: %d\n", bbox.y_min);
                        printf("xmax: %d\n", bbox.x_max);
                        printf("ymax: %d\n\n", bbox.y_max);
                    #endif

                    bbox.conf = conf;
                    bbox.cls = cls;

                    bbox_lst[selected_anchor_num] = bbox;

                    selected_anchor_num ++;
                }
                else{txtytwth_pointer += 4;}
                anchor_index ++;

                
                // cls先做反量化
                // cls再做softmax（2个做）
                // 比较cls，返回最大的index以及数值(2个求max)
                // sigmoid(conf) * cls_max_val
                // 与conf_thresh比较，若大于，则将此数值以及对应的index(anchor + w * anchor_num + h * out_w * anchor_num)
                // 以及分类的index记录下
            }
            conf_pointer += 30;
            cls_pointer += 25;
            txtytwth_pointer += 15;
        }
    }
    return selected_anchor_num;
}