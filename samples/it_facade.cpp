#include <boost/iterator/iterator_facade.hpp>
#include <iostream>
#include <vector>

template <typename T>
class vs_iterator : public boost::iterator_facade<  // 基类链技术继承
                        vs_iterator<T>, T,          // 子类名和值类型
                        boost::single_pass_traversal_tag>  // 单遍迭代器类型
{
 private:
  std::vector<T>& v;   // 容器的引用
  size_t current_pos;  // 迭代器的当前位置
 public:
  typedef boost::iterator_facade<vs_iterator<T>, T,
                                 boost::single_pass_traversal_tag>
      super_type;
  typedef vs_iterator this_type;                     // 定义自身的别名
  typedef typename super_type::reference reference;  // 使用基类的引用类型

  vs_iterator(std::vector<T>& _v, size_t pos = 0) : v(_v), current_pos(pos) {}
  vs_iterator(this_type const& other)
      : v(other.v), current_pos(other.current_pos) {}
  void operator=(this_type const& other) {
    this->v = other.v;
    this->current_pos = other.current_pos;
  }

 private:
  friend class boost::iterator_core_access;  // 必需的友元声明

  reference dereference() const  // 解引用操作
  {
    return v[current_pos];
  }

  void increment()  // 递增操作
  {
    ++current_pos;
  }

  bool equal(this_type const& other) const  // 比较操作
  {
    return this->current_pos == other.current_pos;
  }
};

int main() {
  std::vector<int> v{1, 2, 3, 4, 5};
  vs_iterator<int> vsi(v), vsi_end(v, v.size());

  *vsi = 9;
  std::copy(vsi, vsi_end, std::ostream_iterator<int>(std::cout, ","));
  return 0;
}