/**
 * @file radiometric.hpp
 * @author xiaoyc
 * @brief 辐射校正相关算法
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SRC_ALGORITHM_RADIOMETRIC_HPP_
#define SRC_ALGORITHM_RADIOMETRIC_HPP_

// C++ Standard
#include <fstream>
#include <string>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// project
#include "../gdal_traits.hpp"
#include "../gdalex.hpp"
#include "./operation.hpp"

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

/**
 * @brief 暗电平扣除算法
 *
 * @tparam T 载入系数的像元数据类型
 * 
 * @note 配合行迭代器使用
 */
template <typename T>
class DarkBackgroundCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override { return m - m_; }

  /**
   * @brief 载入暗电平系数文件
   *
   * @param filename 系数文件路径，支持栅格数据和文本文件
   */
  void load(const std::string& filename) {
    if (IsRasterDataset(filename.c_str())) {
      m_ = load_raster<T>(filename.c_str());
    } else {
      m_ = load_text<T>(filename.c_str());
    }
  }

 private:
  cv::Mat m_;
};

/**
 * @brief 非均匀校正算法
 * 
 * @tparam T_out 算法输出的像元数据类型
 * @tparam T_coeff 载入系数的像元数据类型
 * 
 * @note 配合行迭代器使用
 */
template <typename T_out, typename T_coeff = float>
class NonUniformityCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::Mat res;
    m.convertTo(m, cv::DataType<T_coeff>::type);
    m = m.mul(a_) + b_;
    m.convertTo(res, cv::DataType<T_out>::type);
    return res;
  }
  /**
   * @brief 载入非均匀系数
   * 
   * @param coeff_a 系数a路径
   * @param coeff_b 系数b路径
   */
  void load(const std::string& coeff_a, const std::string& coeff_b) {
    if (IsRasterDataset(coeff_a.c_str())) {
      a_ = load_raster<T_coeff>(coeff_a.c_str());
    } else {
      a_ = load_raster<T_coeff>(coeff_a.c_str());
    }
    if (IsRasterDataset(coeff_b.c_str())) {
      b_ = load_raster<T_coeff>(coeff_b.c_str());
    } else {
      b_ = load_raster<T_coeff>(coeff_b.c_str());
    }
  }

 private:
  cv::Mat a_;
  cv::Mat b_;
};

/**
 * @brief 绝对辐射校正算法
 * 
 * @tparam T_out 
 * @tparam T_coeff 
 * 
 * @note 配合行迭代器使用
 */
template <typename T_out = float, typename T_coeff = float>
class AbsoluteRadiometricCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::Mat res;
    m.convertTo(m, cv::DataType<T_coeff>::type);
    // m = m.mul(a_) + b_;
    m.convertTo(res, cv::DataType<T_out>::type);
    return res;
  }
  void load(const std::string& filename) {}

 private:
  cv::Mat a_;
  cv::Mat b_;
};

/**
 * @brief 高斯滤波算法
 * 
 */
class GaussianFilter : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::Mat res;
    cv::GaussianBlur(m, res, cv::Size(3, 3), 0, 0);
    return res;
  }
};

}  // namespace hsp

#endif  // SRC_ALGORITHM_RADIOMETRIC_HPP_
