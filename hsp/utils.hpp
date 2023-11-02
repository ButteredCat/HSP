/**
 * @file utils.hpp
 * @author xiaoyc
 * @brief 通用工具
 * @version 0.1
 * @date 2023-11-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_UTILS_HPP_
#define HSP_UTILS_HPP_

// GDAL
#include <gdal.h>
#include <gdal_priv.h>

// C++ Standard
#include <string>
#include <vector>

// Opencv
#include <opencv2/core.hpp>

// hsp
#include "./gdal_traits.hpp"

namespace hsp {
/**
 * @brief 载入栅格系数
 *
 * @note 对于多波段的数据，仅载入第1波段
 * @tparam T 像元数据类型
 * @param filename 系数完整路径
 * @return cv::Mat 读取到的系数
 */
template <typename T>
cv::Mat load_raster(const std::string& filename) {
  auto dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_ReadOnly)));
  if (!dataset) {
    throw std::runtime_error("unable to open dataset");
  }
  const int n_samples = dataset->GetRasterXSize();
  const int n_lines = dataset->GetRasterYSize();
  cv::Mat res =
      cv::Mat::zeros(cv::Size(n_samples, n_lines), cv::DataType<T>::type);
  CPLErr err = dataset->GetRasterBand(1)->RasterIO(
      GF_Read, 0, 0, n_samples, n_lines, res.data, n_samples, n_lines,
      gdal::DataType<T>::type(), 0, 0);
  return res;
}

/**
 * @brief 载入文本格式的系数
 *
 * @tparam T 像元数据类型
 * @param filename 系数完整路径
 * @return cv::Mat 读取到的系数
 */
template <typename T>
cv::Mat load_text(const std::string& filename) {
  std::ifstream in(filename);
  std::vector<T> data_vec;
  int n_lines{0}, count{0};
  for (std::string line; std::getline(in, line);) {
    std::vector<T> data_line;
    std::istringstream input(line);
    for (T data; input >> data;) {
      data_vec.push_back(data);
      ++count;
    }
    ++n_lines;
  }
  return cv::Mat(cv::Size(count / n_lines, n_lines), cv::DataType<T>::type,
                 data_vec.data())
      .clone();
}
}  // namespace hsp

#endif  // HSP_UTILS_HPP_
