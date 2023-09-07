// Copyright (C) 2023 Xiao Yunchen
#pragma once

// C++ Standard
#include <memory>
#include <string>

// Boost
#include <boost/iterator/iterator_facade.hpp>

namespace hsp {

enum class Endian { Little = 1, Big = 2 };

class RawData {
  explicit RawData(const std::string& filename) : filename_{filename} {}
  virtual void Traverse() = 0;

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
  RawData& set_compressed(bool value) {
    is_compressed_ = value;
    return *this;
  }
  RawData& set_word_length(int length) {
    word_length_ = length;
    return *this;
  }
  RawData& set_max_aux_size(int value) {
    if (value < 0) {
      // throw std::exception("must not be negative");
    }
    max_aux_size_ = value;
    return *this;
  }
  RawData& set_leading_bytes(const std::string& value) {
    leading_bytes_ = value;
    return *this;
  }

 protected:
  void set_frame_aux_size(int value) { frame_aux_size_ = value; }
  void set_sensor_width(int w) { sensor_width_ = w; }
  void set_sensor_height(int h) { sensor_height_ = h; }
  void set_n_frames(int n) { n_frames_ = n; }
  void set_traversed(bool value) { is_traversed_ = value; }

 private:
  const std::string filename_;
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
