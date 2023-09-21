# HSP
通用高性能高光谱数据处理框架。

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
```shell
cpplint --recursive --quiet src test app
```

集成测试：
```shell
cmake -S . -B build
cmake --build build --target hsp-test
cd build && ctest
```

## 入门教程
### C++ STL 中的迭代器、算法和函数对象
Container（容器）、allocator（分配器）、algorithm（算法）、iterator（迭代器）、adapter（适配器）和functor（函数对象）是C++ STL的6大组成部分。通过迭代器，C++隐藏了迭代对象的内部实现，基于模板的算法将算法和具体数据结构解耦，函数对象可以灵活地配置函数状态，将多参数的函数适配为适合算法要求的谓语（predicate）。

考虑有数组a和b，
```cpp
std::vector<int> a{1, 2, 3};
std::vector<int> b(a.size());
```
现在需要将数组a中的所有元素加1，并将结果写入数组b中。一种基础的做法是这样：
```cpp
for(int i = 0; i != a.size(); ++i) {
    b[i] = a[i] + 1;
}
```
如果使用`std::transform`算法，可以有另一种写法：
```cpp
std::transform(a.begin(), a.end(), b.begin(), [](int val){ return val + 1;})
```
`std::transform`的前2个参数是输入的迭代范围，第3个参数是输出结果的起始处，第4个参数是一个匿名函数，即所谓的谓语，它不关心迭代过程，只是简单地对每次迭代的值进行处理。

在`std::transform`中，谓语接受的参数个数和类型是确定的，它必须是一个一元操作，它只能接受迭代器值类型的数据。如果我们要对所有元素加2，加3，乃至加100，从哪里进行配置呢？当然可以让匿名函数捕获外部值，还有一种方法是，使用函数对象。

下面的代码中，定义了一个`Plus`类，由于重载了`()`操作符，这个类的对象可以像函数一样被调用，又由于它是一个类，有自己的数据成员，所以可以通过设置数据成员，来配置函数对象的行为，相比于上个版本的匿名函数，`Plus`的可复用性提高了。
```cpp
struct Plus {
    int operator()(int val) {
        return val + n_;
    }
    void set_n(int n) {
        n_ = n;
    }
    private:
        int n_ = 1;
};

Plus pls;
pls.set_n(2);
std::transform(a.begin(), a.end(), b.begin(), pls);
```

### HSP 中的迭代器和图像处理操作
HSP的设计遵循了和STL相似的逻辑，通过各种输入输出迭代器，将数据立方的迭代细节，与图像处理操作分离开来，通过函数对象的设计，实现了算法的串联。
