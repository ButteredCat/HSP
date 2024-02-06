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

/**
 * @brief 计算输入矩阵各列中位数。
 *
 * @details
 * 先对各列排序。如果列中元素个数为奇数，中值为排序后的中间元素；
 * 如果元素个数为偶数，中值为排序后中间2个元素的均值。
 * NaN不参与计算。
 *
 */
inline cv::Mat1d median(const cv::Mat& m) {
  cv::Mat1d m_d;
  if (m.type() != CV_64FC1) {
    m.convertTo(m_d, CV_64FC1);
  } else {
    m_d = m;
  }
  cv::Mat1d res = cv::Mat::zeros(1, m.cols, CV_64FC1);
  auto col_median = res.begin();

  for (int i = 0; i < m.cols; ++i) {
    std::vector<double> column(m.rows);
    int size{0};
    for (int j = 0; j < m.rows; ++j) {
      auto val = m_d.at<double>(j, i);
      if (!isnan(val)) {
        column[size++] = val;
      }
    }
    // TODO(xiaoyc): 尝试用std::nth_element解决，注意NaN的处理方式
    std::sort(column.begin(), column.begin() + size);
    int mid = size / 2;
    if (size % 2) {
      *col_median++ = column.at(mid);
    } else if (mid != 0) {
      *col_median++ = (column.at(mid - 1) + column.at(mid)) / 2.0;
    } else {
      *col_median++ = std::numeric_limits<double>::quiet_NaN();
    }
  }
  return res;
}

/**
 * @brief 查找数据中的离群值。
 *
 * @details
 * 查找输入矩阵中每一列的离群值。离群值是指与中位数相差超过三倍经过换算的中位数绝对偏差
 * (scaled MAD) 的值。元素NaN不认为是离群值。
 * 实现了与MATLAB中同名函数类似的功能。
 *
 */
inline cv::Mat isoutlier(const cv::Mat& m) {
  constexpr double erfcinv_1_5 = -0.476936276204470;
  constexpr double sqrt2 = 1.41421;
  constexpr double c = -1.0 / (sqrt2 * erfcinv_1_5);
  auto m_median = cv::repeat(median(m), m.rows, 1);
  auto scaled_MAD = c * median(cv::abs(m - m_median));
  return m - m_median > 3 * cv::repeat(scaled_MAD, m.rows, 1);
}

/**
 * @brief 分别计算矩阵各列的均值和标准差。忽略NaN。
 */
inline cv::Mat1d meanStdDev(const cv::Mat& m) {
  auto mask = (m == m);
  cv::Mat1d res = cv::Mat::zeros(2, m.cols, CV_64FC1);
  for (int i = 0; i < m.cols; ++i) {
    cv::Mat1d mean, stddev;
    cv::meanStdDev(m.col(i), mean, stddev, mask.col(i));
    res.at<double>(0, i) = mean.at<double>(0, 0);
    res.at<double>(1, i) = stddev.at<double>(0, 0);
  }
  return res;
}

}  // namespace hsp

#endif  // HSP_UTILS_HPP_
