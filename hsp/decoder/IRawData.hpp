/**
 * @file IRawData.hpp
 * @author xiaoyc
 * @brief 数据解析基类。
 * @version 0.1
 * @date 2023-11-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_DECODER_IRAWDATA_HPP_
#define HSP_DECODER_IRAWDATA_HPP_

// C++ Standard
#include <fstream>
#include <iterator>
#include <string>

// OpenCV
#include <opencv2/core.hpp>

namespace hsp {

/**
 * @brief
 * 原始数据解析基类，实现了帧的输出迭代器和一些基本查询接口。
 * 子类开发者需要实现自己的Traverse()和GetFrame()方法。
 *
 * @tparam Tf
 * 迭代器返回的帧类型。默认是cv::Mat，如果有特殊需求（如附带序列号等信息），可自定义帧类型。
 */
template <typename Tf = cv::Mat>
class IRawData {
 public:
  const std::string filename;

 public:
  /**
   * @brief 构造函数。
   *
   * @param datafile 原始数据路径。
   */
  explicit IRawData(const std::string& datafile)
      : filename{datafile}, in_stream_(datafile, std::ios::binary) {}

  IRawData(const IRawData&) = delete;

  /**
   * @brief
   * 纯虚函数。实现者需要遍历原始数据以获取图像尺寸、波段数和传感器类型等信息。其中n_samples_、n_lines_和n_bands_为必须获取的信息。
   *
   */
  virtual void Traverse() = 0;

  /**
   * @brief 纯虚函数。返回第i帧的帧对象。
   *
   * @param i 原始数据的帧序号，从0开始计数。
   * @return Tf 帧类型，默认是cv::Mat，可自定义以携带其他帧信息。
   */
  virtual Tf GetFrame(int i) = 0;

  int n_samples() const { return n_samples_; }

  int n_lines() const { return n_lines_; }

  int n_bands() const { return n_bands_; }

  /**
   * @brief 帧迭代器。对迭代器解引用后，得到 Tf 类型的一帧影像。
   *
   */
  class FrameIterator : public std::iterator<std::input_iterator_tag, Tf> {
   public:
    explicit FrameIterator(IRawData* raw_data, int cur = 0)
        : raw_{raw_data}, cur_{cur} {
      raw_data->Traverse();
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

    Tf operator*() const { return raw_->GetFrame(cur_); }

    // Frame* operator->() const {
    //   return raw_->GetFrame(cur_);
    // };

   private:
    IRawData* raw_;
    int cur_;
  };

  /**
   * @brief 返回指向起始位置的帧迭代器。
   *
   * @note 遵循C++ STL的一般规则，begin() 和 end() 构成前闭后开区间，即[begin(),
   * end())。
   *
   * @return FrameIterator 指向起始位置的帧迭代器。
   */
  FrameIterator begin() { return FrameIterator(this, 0); }

  /**
   * @brief 返回指向末尾的迭代器。
   *
   * @note 遵循C++ STL的一般规则，begin() 和 end() 构成前闭后开区间，即[begin(),
   * end())。
   *
   * @return FrameIterator 指向末尾的迭代器。
   */
  FrameIterator end() { return FrameIterator(this, n_lines_); }

 protected:
  bool is_traversed_ = false;
  std::ifstream in_stream_;
  int n_samples_ = 0;
  int n_bands_ = 0;
  int n_lines_ = 0;
  cv::Mat img_;
};

}  // namespace hsp

#endif  // HSP_DECODER_IRAWDATA_HPP_
