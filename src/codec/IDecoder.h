#pragma once
#include <string>
#include <vector>

namespace hsp {

class IDecoder {
  using DataType = std::vector<char>;

 public:
  IDecoder() = default;
  void set_data(const DataType& data) {
    std::copy(data.begin(), data.end(), std::back_inserter(aux_data_));
  };
  const DataType& get_data() const { return aux_data_; }
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual int frame_aux_size() const = 0;
  virtual bool is_leading_bytes_matched(const std::string& bytes) const = 0;

 private:
  DataType aux_data_;
};

}  // namespace hsp
