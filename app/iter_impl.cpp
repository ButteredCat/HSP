// Copyright (C) 2023 xiaoyc
// C++ standard
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

// Boost
#include <boost/iterator/iterator_facade.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

class RawData {
  friend class Frame;

 public:
  using Frame = boost::shared_ptr<char[]>;
  explicit RawData(const std::string& filename)
      : filename_{filename}, in_(filename, std::ios::binary) {}
  size_t frame_size() const { return frame_size_; }

  class FrameIterator
      : public boost::iterator_facade<FrameIterator, Frame const,
                                      boost::single_pass_traversal_tag> {
    friend boost::iterator_core_access;

   public:
    explicit FrameIterator(RawData& d, int cur_frame = 0)  // NOLINT
        : data_(d), cur_frame_{cur_frame} {
      buffer = boost::shared_ptr<char[]>(new char[data_.frame_size_]);
      data_.in_.read(buffer.get(), data_.frame_size_);
    }

   private:
    RawData& data_;
    int cur_frame_;
    Frame buffer;

   private:
    void increment() {
      data_.in_.seekg(cur_frame_ * data_.frame_size_, data_.in_.beg);
      data_.in_.read(buffer.get(), data_.frame_size_);
      ++cur_frame_;
    }

    reference dereference() const { return buffer; }
    bool equal(FrameIterator const& other) const {
      return (this->cur_frame_ == other.cur_frame_);
    }
  };

 private:
  std::string filename_;
  std::ifstream in_;
  size_t frame_size_ = 1024 + 426 * 341 * 2;
  int n_frame_ = 521;
};

int main(void) {
  std::string filename =
      "/home/xiaoyc/dataset/HGY/nir/Goldeye-20230103_142007-00000.dat";
  RawData data(filename);
  RawData::FrameIterator beg(data, 0), end(data, 521);
  RawData::Frame frame;
  std::string out_file = "/home/xiaoyc/dataset/HGY/nir/out.dat";
  std::ofstream out(out_file, std::ios::binary);
  for (auto it = beg; it != end; ++it) {
    (*it)[4];
    out.write(it->get(), data.frame_size());
  }
  return 0;
}
