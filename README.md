# HSP
统一的高光谱数据处理框架。

## 环境需求
### 基础编译
- C++ 14
- CMake 3.16
- Boost 1.71
- GDAL 2.3
- OpenCV 4

### 代码静态检查工具
[cpplint](https://github.com/cpplint/cpplint)

### 集成测试框架 
[GoogleTest](https://github.com/google/googletest)

## 测试
静态代码检查:
```
cpplint --recursive --quiet src test app
```

集成测试：
```
cmake -S . -B build
cmake --build build --target hsp-test
cd build && ctest
```
