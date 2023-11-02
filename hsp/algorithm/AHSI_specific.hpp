/**
 * @file AHSI_specific.hpp
 * @author xiaoyc
 * @brief 专用于AHSI的算法
 * @version 0.1
 * @date 2023-11-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_ALGORITHM_AHSI_SPECIFIC_HPP_
#define HSP_ALGORITHM_AHSI_SPECIFIC_HPP_

// C++ Standard
#include <string>

#include "../decoder/AHSIData.hpp"
#include "../utils.hpp"

namespace hsp {
/**
 * @brief 高五01A专用暗电平扣除算法
 * @details
 *
 */
class GF501A_DBC {
 public:
  using CoeffDataType = float;
  cv::Mat operator()(const AHSIData::Frame& frame) const {
    cv::Mat dark;
    cv::Mat tmp = a_ * static_cast<CoeffDataType>(frame.index) + b_;
    tmp.convertTo(dark, cv::DataType<uint16_t>::type);
    return frame.data - dark;
  }
  void load(const std::string& a, const std::string& b) {
    a_ = hsp::load_raster<CoeffDataType>(a);
    b_ = hsp::load_raster<CoeffDataType>(b);
  }

 private:
  cv::Mat a_;
  cv::Mat b_;
};
}  // namespace hsp

#endif  // HSP_ALGORITHM_AHSI_SPECIFIC_HPP_
