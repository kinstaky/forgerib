# ForgeRIB

## 简介

本项目用于预处理在 RIBLL1 终端完成的丰质子核直接核反应实验的数据，包含解码数据、对齐获取系统、重建事件。

项目地址：https://github.com/kinstaky/forgerib.git

> [!WARNING]
>
> 本项目摒弃了将所有探测器合并到一个 ROOT 文件中的做法，而是将每一个探测器独立存储于一个 ROOT 文件中，以期在处理单个探测器时更加高效。预处理的结果都是保存到 ROOT 文件中叫 tree 的 TTree 中，所有探测器的 entry 数相同的，可以通过 `TTree::AddFriend` 并联。

## 文档结构

本文档分为两部分

+ 第一部分为基础，讲的是下载、编译、运行该程序，并得到重组后的 ROOT 文件；
+ 第二部分为进阶，给出了本项目的详细流程和原理。

## 改进

欢迎提交 [issue](https://github.com/kinstaky/forgerib/issues) 对本说明文档或者项目提出修改意见，或者发送邮件到 kinstaky@163.com。
> [!NOTE]
>
> 本项目以运行速度为第一优先级，而非通用性，因此高度绑定本次实验的探测器和获取系统，对于其它实验需要重写部分代码。

