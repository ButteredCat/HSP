// Copyright (C) 2023 Xiao Yunchen
#pragma once

// C++ Standard
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// project
#include "IDecoder.h"

namespace hsp {

enum class Interleave {
  BIL = 1,
  BIP = 2,
  BSQ = 3,
};

enum class Endian { Little = 1, Big = 2 };

using DecoderPtr = std::shared_ptr<IDecoder>;

class IRawData {
  using frame_iterator = std::istream_iterator<char>;
  using RawFrame = std::vector<char>;
  friend std::istream& operator>>(std::istream& stream, RawFrame& frame) {
    const stream.read(RawFrame.data(), 1);
  }

 public:
  IRawData(const std::string& raw_data, const DecoderPtr& aux_decoder)
      : raw_data_{raw_data}, aux_dec{aux_decoder} {}
  virtual void Traverse() = 0;
  virtual void ToRaster(const std::string& dst_file, int begin, int end) = 0;
  bool is_traversed() const { return is_traversed_; }
  int word_length() const { return word_length_; }
  int sensor_width() const { return sensor_width_; }
  int sensor_height() const { return sensor_height_; }
  int n_frames() const { return n_frames_; }
  int max_aux_size() const { return max_aux_size_; }
  int frame_aux_size() const { return frame_aux_size_; }
  int band_aux_size() const { return band_aux_size_; }
  float bytes_per_pixel() const {
    float res = 2.0;
    if (word_length_ <= 8) {
      res = 1.0;
    } else if (is_compressed_) {
      res = 1.5;
    }
    return res;
  }
  std::string leading_bytes() const { return leading_bytes_; }
  Endian endian() const { return endian_; }
  IRawData& set_compressed(bool value) {
    is_compressed_ = value;
    return *this;
  }
  IRawData& set_word_length(int length) {
    word_length_ = length;
    return *this;
  }
  IRawData& set_max_aux_size(int value) {
    if (value < 0) {
      // throw std::exception("must not be negative");
    }
    max_aux_size_ = value;
    return *this;
  }
  IRawData& set_leading_bytes(const std::string& value) {
    leading_bytes_ = value;
    return *this;
  }

 protected:
  const std::string raw_data_;
  const DecoderPtr aux_dec;

 protected:
  void set_frame_aux_size(int value) { frame_aux_size_ = value; }
  void set_sensor_width(int w) { sensor_width_ = w; }
  void set_sensor_height(int h) { sensor_height_ = h; }
  void set_n_frames(int n) { n_frames_ = n; }
  void set_traversed(bool value) { is_traversed_ = value; }

 private:
  int word_length_ = 16;
  bool is_traversed_ = false;
  bool is_compressed_ = false;
  Endian endian_ = Endian::Little;
  int sensor_width_ = 0;
  int sensor_height_ = 0;
  int n_frames_ = 0;
  int max_aux_size_ = 1024;
  int frame_aux_size_ = 1024;
  int band_aux_size_ = 0;
  std::string leading_bytes_;
};

}  // namespace hsp
