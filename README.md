# RISC-V C-Embedding Yolo
基于Yolo v2的蜂鸟e203 RISC-V部署代码，其中的加速器由队伍中负责硬件的人使用Verilog编写（暂不提供），并在硬件提供的C API上搭建了yolo的部署代码。其中，加速器硬件模块暂由c编写的神经网络加速器模拟器来代替。

网络实现了人脸口罩检测功能，人脸显示为红色框，口罩显示为绿色框。

# Installation Dependencies:
- gcc

# Visualization
![Image](./example_result/demo.png)

# How to run
```Shell
make
./yolo
```
