# 下载编译

## 依赖

+ [cern ROOT](https://root.cern/)
+ [CMake](https://cmake.org/) 用于编译

具体的版本没有测试，请自行尝试。

## 下载

使用 git 从 github 下载

```bash
git clone https://github.com/kinstaky/forgerib.git
```

## 编译

进入代码目录

```bash
cd forgerib
```

使用 CMake 编译，通过以下命令创建 `build` 文件夹并预处理

```cmake
cmake -S. -Bbuild
```

编译

```bash
cmake --build build -j
```

编译后的程序在 `build/bin` 目录中。