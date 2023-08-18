#include <boost/iterator/function_output_iterator.hpp>
#include <vector>

class to_hex {
 private:
  std::vector<unsigned char> &v;  // 存储十六进制数的容器
  int count;                      // 字符计数

  char trans(const char c) const  // 从ASCII码转换到十六进制数
  {
    if (c >= 'a') {
      return c - 'a' + 10;
    } else if (c >= 'A') {
      return c - 'A' + 10;
    } else {
      return c - '0';
    }
  }

 public:
  to_hex(std::vector<unsigned char> &_v) : v(_v), count(0) {}
  void operator()(const char c) {
    static char tmp;
    if ((count++) % 2 == 0) {
      tmp = trans(c) * 0x10;
    } else {
      tmp += trans(c);
      v.push_back(tmp);
    }
  }
};

int main() {
  char s[] = "1234abcd";
  std::vector<unsigned char> v;
  std::copy(s, s + 8, boost::make_function_output_iterator(to_hex(v)));
  return 0;
}