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
`std::transform`的前2个参数是输入的迭代范围，第3个参数是输出结果的起始迭代位置，第4个参数是一个匿名函数，即所谓的谓语，它不关心迭代过程，只是简单地对每次迭代的值进行处理。

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

高光谱图像处理中，常见的场景是，将图像处理算法逐行或者逐波段应用到数据立方上。HSP实现了针对行和波段的输入输出迭代器，也实现了较少使用的样本迭代器，迭代器的值类型是OpenCV中的`cv::Mat`，算法只需要关注单个`cv::Mat`的处理，并且可以利用OpenCV中的矩阵运算和图像处理算法，提升开发效率。

下面的代码完整展示了使用HSP进行暗电平扣除的步骤：
```cpp
using DataType = uint16_t;
// 使用GDAL提供的GDALDatasetUniquePtr管理资源，无须手动调用GDALClose释放
auto dataset = GDALDatasetUniquePtr(
    GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_Update)));
if (!dataset) {
    throw std::exception();
}
// 创建行输入迭代器
hsp::LineInputIterator<DataType> beg(dataset.get(), 0), end(dataset.get());

auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
if (!poDriver) {
    throw std::exception();
}
auto out_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
    poDriver->CreateCopy(out_raster.c_str(), dataset.get(), FALSE, nullptr,
                        nullptr, nullptr)));
if (!out_dataset) {
    throw std::exception();
}
// 创建行输出迭代器
hsp::LineOutputIterator<DataType> obeg(out_dataset.get(), 0);

// 创建暗电平扣除算法，并载入暗电平系数。dbc是函数对象，可以直接作为谓语
auto dbc = hsp::DarkBackgroundCorrection<DataType>();
dbc.load(dark_coeff);

std::transform(beg, end, obeg, ops);
```

如上所述，简单的算法原本是可以用匿名函数实现的，HSP中使用函数对象，是为了方便算法串联。暗电平扣除和非均匀校正都是按行迭代的，如果串联起来，可以节省一次文件读写，对于尺寸普遍较大的高光谱影像而言，意义重大。
```cpp
// 创建暗电平扣除算法。由于需要添加进UnaryOpCombo，这里的dbc由hsp::make_op创建，实际是共享指针
auto dbc = hsp::make_op<hsp::DarkBackgroundCorrection<DataType> >();
dbc->load(dark_coeff);
// 创建非均匀校正算法，并载入a、b系数
auto nuc = hsp::make_op<hsp::NonUniformityCorrection<DataType, float> >();
nuc->load(rel_a_coeff, rel_b_coeff);
// 将算法加入Combo类对象，实现串联
hsp::UnaryOpCombo ops;
ops.add(dbc).add(nuc);

std::transform(beg, end, obeg, ops);
```

由于实现了标准的C++迭代器，HSP可以充分利用STL提供的各类算法设施，如用`std::reduce`配合行输入迭代器计算暗电平系数，或者实现二元操作同时迭代2个高光谱影像进行空间维盲元修复等，需要根据实际情况由算法开发者自行探索。

### HSP中的图像处理算法实现
为了实现算法串联，所有一元操作算法，包括`UnaryOpCombo`，都是`UnaryOperation`的子类。Combo类中维护了一个数组，保存了指向算法基类（一元操作的话，就是`UnaryOperation`）的共享指针，当Combo类的函数对象被调用时，会按照添加顺序，依次执行数组中的算法。

### 图像处理算法加速
算法可以利用OpenCV中的CUDA算法，在hsp::cuda命名空间下，实现了暗电平扣除、非均匀校正以及其他算法的CUDA版本。
