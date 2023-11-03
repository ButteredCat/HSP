/**
 * @file AHSIData.hpp
 * @author xiaoyc
 * @brief 高分五号01A AHSI的0级数据解析接口。
 * @version 0.1
 * @date 2023-10-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_DECODER_AHSIDATA_HPP_
#define HSP_DECODER_AHSIDATA_HPP_

// C++ Standard Library
#include <fstream>
#include <memory>
#include <string>

// Boost
#include <boost/endian/conversion.hpp>

// OpenCV
#include <opencv2/core.hpp>

namespace hsp {
/**
 * @brief AHSI 0级数据处理类型。
 *
 * @details
 *
 */
class AHSIData {
 public:
  /**
   * @brief 传感器类型。
   *
   */
  enum class SensorType { SWIR = 1, VNIR = 2 };
  /**
   * @brief 压缩模式。
   *
   */
  enum class Compress { Lossless = 0, Lossy8 = 1, Lossy4 = 2, Direct = 3 };
  /**
   * @brief
   * AHSI帧类型。
   *
   */
  struct Frame {
    Frame() = delete;
    Frame(const cv::Mat& d, uint32_t i) : data(d), index{i} {}
    Frame(const Frame& other) : data{other.data}, index{other.index} {}
    /**
     * @brief 帧图像数据。DN值以16位无符号整型存储。
     * 
     */
    const cv::Mat& data;
    /**
     * @brief 帧序列号。
     * 
     */
    uint32_t index;
  };
  /**
   * @brief 帧引导头。
   *
   */
  const char leading_bytes[4] = {0x09, 0x15, static_cast<char>(0xC0), 0x00};
  /**
   * @brief 待处理的0级数据的路径。
   *
   */
  const std::string filename;

 public:
  explicit AHSIData(const std::string& datafile)
      : filename{datafile}, in_stream_(datafile, std::ios::binary) {}
  AHSIData(const AHSIData&) = delete;
  /**
   * @brief
   * 遍历整个0级数据文件，更新传感器类型（VNIR、SWIR），图像尺寸（samples、lines、bands）等信息。
   *
   * @note 需要手工调用本函数一次，才能获取正确的传感器类型、图像尺寸，以及使用迭代器。
   *
   */
  void Traverse();
  Frame GetFrame(int i);
  int n_samples() const { return n_samples_; }
  int n_lines() const { return n_lines_; }
  int n_bands() const { return n_bands_; }
  SensorType sensor_type() const { return type_; }
  Compress compress_mode() const { return compress_; }

  friend class FrameIterator;
  /**
   * @brief 帧迭代器。对迭代器解引用后，得到`AHSIData::Frame`格式的一帧影像。
   *
   */
  class FrameIterator : public std::iterator<std::input_iterator_tag, Frame> {
   public:
    explicit FrameIterator(AHSIData& raw_data, int cur = 0)  // NOLINT
        : raw_{raw_data}, cur_{cur} {
      raw_data.Traverse();
    }
    FrameIterator operator++() {
      ++cur_;
      return *this;
    }
    FrameIterator operator++(int) {
      FrameIterator old(*this);
      ++(*this);
      return old;
    }
    bool operator==(const FrameIterator& other) const {
      return cur_ == other.cur_;
    }
    bool operator!=(const FrameIterator& other) const {
      return !(*this == other);
    }
    Frame operator*() const { return raw_.GetFrame(cur_); }
    // Frame* operator->() const {
    //   return &raw_.GetFrame(cur_);
    // };

   private:
    AHSIData& raw_;
    int cur_;
  };
  /**
   * @brief 返回指向起始位置的帧迭代器
   *
   * @note 遵循C++ STL的同行规则，begin() 和 end() 构成前闭后开区间，即[begin(),
   * end())。
   *
   * @return FrameIterator
   */
  FrameIterator begin() { return FrameIterator(*this, 0); }
  /**
   * @brief 返回指向末尾的迭代器
   *
   * @note 遵循C++ STL的同行规则，begin() 和 end() 构成前闭后开区间，即[begin(),
   * end())。
   *
   * @return FrameIterator
   */
  FrameIterator end() { return FrameIterator(*this, n_lines_); }

 private:
  bool is_traversed_ = false;
  std::ifstream in_stream_;
  int n_samples_ = 0;
  int n_bands_ = 0;
  int n_lines_ = 0;
  SensorType type_ = SensorType::SWIR;
  Compress compress_ = Compress::Lossless;
  cv::Mat img_;
};

void AHSIData::Traverse() {
  if (is_traversed_) {
    return;
  }
  //  parse the first frame
  const int buffer_size = 5 * 1024;
  auto buffer = std::make_unique<char[]>(buffer_size);
  if (!in_stream_) {
    throw std::runtime_error("unable to open raw data");
  }
  in_stream_.read(buffer.get(), buffer_size);

  auto head = std::search(buffer.get(), buffer.get() + buffer_size,
                          leading_bytes, leading_bytes + sizeof(leading_bytes));
  if (head == buffer.get() + buffer_size) {
    throw std::runtime_error("unable to find leading bytes!");
  }
  n_samples_ =
      boost::endian::load_big_u16(reinterpret_cast<unsigned char*>(head + 4));
  if ((head[6] & 0x0F) != 0x07) {
    throw std::runtime_error("this frame is not a data frame");
  }
  type_ = static_cast<SensorType>(head[6] >> 4);
  compress_ = static_cast<Compress>(head[7] & 0x03);

  if (compress_ == Compress::Direct) {
    n_bands_ = type_ == SensorType::SWIR ? 180 : 150;
  } else {
    // set n_bands_ to default regardless of compress mode
    n_bands_ = type_ == SensorType::SWIR ? 180 : 150;
  }

  // traverse the whole data
  const size_t band_size = 6 + 6 + n_samples_ * 2;
  const size_t frame_size = 8 + band_size * n_bands_;
  const size_t read_size = 100;
  in_stream_.seekg(0, in_stream_.beg);
  while (in_stream_.read(buffer.get(), read_size)) {
    head = std::search(buffer.get(), buffer.get() + buffer_size, leading_bytes,
                       leading_bytes + sizeof(leading_bytes));
    if (head != buffer.get() + 8) {
      break;
    }
    ++n_lines_;
    in_stream_.seekg(-read_size + frame_size, in_stream_.cur);
  }
  // reserve space
  img_ = cv::Mat::zeros(cv::Size(n_samples_, n_bands_),
                        cv::DataType<uint16_t>::type);
  in_stream_.clear();
  is_traversed_ = true;
}

AHSIData::Frame AHSIData::GetFrame(int i) {
  Traverse();
  if (i >= n_lines_) {
    throw std::out_of_range("");
  }
  if (!in_stream_) {
    in_stream_.clear();
  }

  // size in bytes
  const size_t header_size = 12;
  const size_t band_size = n_samples_ * 2 + header_size;
  const size_t frame_size = band_size * n_bands_;

  auto buffer = std::make_unique<char[]>(frame_size);
  in_stream_.seekg(8 + i * (frame_size + 8), in_stream_.beg);
  in_stream_.read(buffer.get(), frame_size);
  uint32_t index =
      boost::endian::load_big_u24(reinterpret_cast<uint8_t*>(buffer.get() + 9));
  // Method 1
  // std::vector<uint16_t> res_vec(n_samples_ * n_bands_);
  // auto res_it = res_vec.begin();
  // for (size_t k = 0; k < frame_size; k += 2) {
  //   if ((k % band_size) >= header_size) {
  //     *res_it =
  //     boost::endian::load_little_u16(reinterpret_cast<uint8_t*>(buffer.get()
  //     + k));
  //     ++res_it;
  //   }
  // }
  // memcpy(img_.data, res_vec.data(), res_vec.size() * sizeof(uint16_t));

  // Method 2, 10x faster than method 1
  size_t buffer_offset = header_size;
  size_t img_offset = 0;
  for (auto b = 0; b < n_bands_; ++b) {
    memcpy(img_.data + img_offset, buffer.get() + buffer_offset,
           n_samples_ * 2);
    buffer_offset += band_size;
    img_offset += n_samples_ * 2;
  }
  return Frame(img_, index);
}

}  // namespace hsp

#endif  // HSP_DECODER_AHSIDATA_HPP_
