#ifndef __WEIGHT_H
#define __WEIGHT_H

extern const char scale_w[];
extern const char scale_b[];
extern const char scale_a[];
extern const char retune[];

extern const char b_conv0[];
extern const char b_conv1[];
extern const char b_conv2[];
extern const char b_conv3[];
extern const char b_conv4[];
extern const char b_conv5[];
extern const char b_conv6[];
extern const char b_conv7[];
extern const char b_conv8[];
extern const char b_conv9[];
extern const char w_conv0[][3][512];
extern const char w_conv1[][3][512];
extern const char w_conv2[][3][2048];
extern const char w_conv3[][3][4096];
extern const char w_conv4[][3][8192];
extern const char w_conv5[][3][16384];
extern const char w_conv6[][3][32768];
extern const char w_conv7[][3][65536];
extern const char w_conv8[][3][65536];
extern const char w_conv9[][3][8960];

#endif