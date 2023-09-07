# HSP
统一的高光谱数据处理框架。

## 编译需求
C++ 14, Boost 1.71, GDAL 2.3, OpenCV 4

静态代码检查工具 cpplint

集成测试框架 GoogleTest

## 测试
运行cpplint进行静态代码检查:
```
cpplint --recursive src test app
```

使用GoogleTest框架进行集成测试：
```
cmake -S . -B build
cmake --build build --target hsp-test
cd build && ctest
```
