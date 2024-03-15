/**
 * @file utils.hpp
 * @author xiaoyc
 * @brief 通用工具。
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
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>

// hsp
#include "./gdal_traits.hpp"

namespace hsp {

constexpr double NaNd = std::numeric_limits<double>::quiet_NaN();

constexpr float NaNf = std::numeric_limits<float>::quiet_NaN();

constexpr float Invalid = -1.0;

/**
 * @brief 载入栅格系数。
 *
 * @note 对于多波段的数据，仅载入第1波段。
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
 * @brief 载入文本格式的系数。
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
    // std::vector<T> data_line;
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

/**
 * @brief 判断浮点数是否为HSP定义的无效值。
 *
 * @param val
 * @return true val是无效值。
 * @return false val不是无效值。
 */
inline bool isInvalid(float val) { return val < 0; }

/**
 * @brief 判断矩阵元素是否为HSP定义的有效值。
 *
 * @param m
 * @return cv::MatExpr 返回的矩阵中，无效值对应位置的元素为非0值，
 * 非无效值对应位置的元素为0。
 */
inline cv::MatExpr isInvalid(const cv::Mat& m) { return m < 0; }

/**
 * @brief 计算输入矩阵各列中位数。
 *
 * @details
 * 先对各列排序。如果列中元素个数为奇数，中值为排序后的中间元素；
 * 如果元素个数为偶数，中值为排序后中间2个元素的均值。
 * 无效值不参与计算。
 *
 */
inline cv::Mat1f median(const cv::Mat& m) {
  cv::Mat1f m_f;
  if (m.type() != CV_32F) {
    m.convertTo(m_f, CV_32F);
  } else {
    m_f = m;
  }
  cv::Mat1f res = cv::Mat::zeros(1, m.cols, CV_32F);
  auto col_median = res.begin();

  for (int i = 0; i < m.cols; ++i) {
    std::vector<float> column(m.rows, Invalid);
    int size{0};
    for (int j = 0; j < m.rows; ++j) {
      const float& val = m_f.at<float>(j, i);
      if (!isInvalid(val)) {
        column[size++] = val;
      }
    }
    int mid = size / 2;
    if (size % 2 == 1) {
      std::nth_element(column.begin(), column.begin() + mid,
                       column.begin() + size);
      *col_median++ = column.at(mid);
    } else if (mid != 0) {
      std::nth_element(column.begin(), column.begin() + mid - 1,
                       column.begin() + size);
      std::nth_element(column.begin(), column.begin() + mid,
                       column.begin() + size);
      *col_median++ = (column.at(mid - 1) + column.at(mid)) / 2.0;
    } else {
      ++col_median;
    }
  }
  return res;
}

/**
 * @brief 计算各列均值（无效值不参与计算）。
 *
 */
inline cv::Mat mean(const cv::Mat& m) {
  cv::Mat m_T;
  cv::transpose(m, m_T);
  cv::Mat res(1, m.cols, CV_32F, Invalid);
  for (int i = 0; i < m.cols; ++i) {
    auto each_row = m_T.row(i);
    res.at<float>(0, i) = cv::mean(each_row, ~isInvalid(each_row))[0];
  }
  return res;
}

/**
 * @brief 查找数据中的离群值。
 *
 * @details
 * 查找输入矩阵中每一列的离群值。离群值是指与中位数相差超过三倍经过换算的中位数绝对偏差
 * (scaled MAD) 的值。无效值不认为是离群值。
 * 实现了与MATLAB中同名函数类似的功能。
 *
 */
inline cv::Mat isoutlier(const cv::Mat& m) {
  constexpr double erfcinv_1_5 = -0.476936276204470;
  constexpr double sqrt2 = 1.41421;
  constexpr float c = -1.0 / (sqrt2 * erfcinv_1_5);
  auto m_median = cv::repeat(median(m), m.rows, 1);
  auto scaled_MAD = c * median(cv::abs(m - m_median));
  return m - m_median > 3 * cv::repeat(scaled_MAD, m.rows, 1);
}

/**
 * @brief 分别计算矩阵各列的均值和标准差（无效值不参与计算）。
 */
inline cv::Mat meanStdDev(const cv::Mat& m) {
  cv::Mat m_T;
  cv::transpose(m, m_T);
  auto mask = ~isInvalid(m_T);
  cv::Mat res(2, m_T.rows, CV_32F, Invalid);
  for (int i = 0; i < m_T.rows; ++i) {
    cv::Mat mean_, stddev;
    cv::Mat1f row = m_T.row(i);
    if (!std::all_of(row.begin(), row.end(),
                     [](float val) { return isInvalid(val); })) {
      cv::meanStdDev(m_T.row(i), mean_, stddev, mask.row(i));
      res.at<float>(0, i) = static_cast<float>(mean_.at<double>(0, 0));
      res.at<float>(1, i) = static_cast<float>(stddev.at<double>(0, 0));
    }
  }
  return res;
}

}  // namespace hsp

#endif  // HSP_UTILS_HPP_
