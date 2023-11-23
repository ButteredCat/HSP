/**
 * @file HGYData.hpp
 * @author xiaoyc
 * @brief 解析核工业岩心扫描仪原始数据。
 * @version 0.1
 * @date 2023-11-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_DECODER_HGYDATA_HPP_
#define HSP_DECODER_HGYDATA_HPP_

// C++ Standard
#include <fstream>
#include <string>

// hsp
#include "./IRawData.hpp"

namespace hsp {
/**
 * @brief 用于解析核工业岩心扫描仪短波相机落盘原始数据。
 *
 */
class HGYData : public IRawData<cv::Mat> {
 public:
  explicit HGYData(const std::string& datafile) : IRawData(datafile) {}
  void Traverse() override;
  cv::Mat GetFrame(int i) const override;
};

void HGYData::Traverse() {}

}  // namespace hsp

#endif  // HSP_DECODER_HGYDATA_HPP_
