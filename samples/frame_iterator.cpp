#include <boost/iterator/function_output_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

struct deleter {
  void operator()(char* p) const {
    delete[] p;
    std::cout << "deleted!\n";
  }
};

class RawData {
 public:
  using Frame = std::shared_ptr<char>;
  explicit RawData(const std::string& filename)
      : raw_in_(filename), filename_{filename} {
    std::cout << "RawData is constructed!\n";
  }
  class FrameIterator
      : public boost::iterator_facade<FrameIterator, Frame const,
                                      boost::single_pass_traversal_tag> {
    // using super_type = boost::iterator_facade<FrameIterator, Frame const,
    // boost::single_pass_traversal_tag > ;
    using this_type = FrameIterator;
    //using reference = Frame;
    friend class boost::iterator_core_access;

   public:
    FrameIterator(RawData& d, int cur_frame = 0)
        : data_{d}, cur_frame_{cur_frame} {}
    FrameIterator(const FrameIterator& other)
        : data_{other.data_}, cur_frame_{other.cur_frame_} {
      std::cout << "copy constructor called!\n";
    }
    // FrameIterator(FrameIterator&& other) : FrameIterator(other) {}

   private:
    RawData& data_;
    int cur_frame_;

    void increment() { ++cur_frame_; }
    void decrement() { --cur_frame_; }
    reference dereference()  {
      Frame buffer = Frame(new char[data_.frame_size], [](auto* p) {
        delete[] p;
        std::cout << "delele!\n";
      });
      data_.raw_in_.seekg(cur_frame_ * data_.frame_size, data_.raw_in_.beg);
      data_.raw_in_.read(buffer.get(), data_.frame_size);
      return buffer;
    }
    bool equal(this_type const& other) const  // 比较操作
    {
      return (this->data_.filename_ == other.data_.filename_) &&
             (this->cur_frame_ == other.cur_frame_);
    }
  };
  FrameIterator begin() { return FrameIterator(*this, 0); }
  FrameIterator end() { return FrameIterator(*this, frame); }

 private:
  std::ifstream raw_in_;
  std::string filename_;

  int aux_size = 1024;
  int width = 421;
  int height = 326;
  int frame = 521;
  int frame_size = aux_size + 421 * 326 * 2;
};

RawData::Frame ret_frame(std::ifstream& in) {
  auto buffer = new char[100];
  in.read(buffer, 100);
  return RawData::Frame(buffer, [](char* p) {
    std::cout << "ret_frame deleted!\n";
    delete[] p;
  });
}

int main() {
  std::string filename =
      "/home/xiaoyc/dataset/HGY/nir/Goldeye-20230103_142007-00000.dat";
  std::string filename2 = filename + "_";
  RawData data(filename);
  RawData data2(filename2);
  std::ifstream in(filename), in2(filename2);
  RawData::FrameIterator beg(data, 0), end(data, 100), a(data2, 0);
  auto res = (beg != a);
  int count = 0;
  for (auto it = data.begin(); it != data.end(); ++it) {
    auto frame = *beg;
  }

  return 0;
}
