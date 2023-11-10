/**
 * @file radiometric.hpp
 * @author xiaoyc
 * @brief 辐射校正相关算法。
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_ALGORITHM_RADIOMETRIC_HPP_
#define HSP_ALGORITHM_RADIOMETRIC_HPP_

// C++ Standard
#include <fstream>
#include <string>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

// project
#include "../gdal_traits.hpp"
#include "../gdalex.hpp"
#include "../utils.hpp"
#include "./operation.hpp"

namespace hsp {

/**
 * @brief 暗电平扣除算法。
 *
 * @tparam T 载入系数的像元数据类型
 *
 * @note 配合行迭代器使用。
 */
template <typename T>
class DarkBackgroundCorrection : public UnaryOperation<cv::Mat> {
 public:
  cv::Mat operator()(cv::Mat m) const override { return m - m_; }

  /**
   * @brief 载入暗电平系数文件。
   *
   * @param filename 系数文件路径，支持栅格数据和文本文件
   */
  void load(const std::string& filename) {
    if (gdal::IsRasterDataset(filename.c_str())) {
      m_ = load_raster<T>(filename.c_str());
    } else {
      m_ = load_text<T>(filename.c_str());
    }
  }

 private:
  cv::Mat m_;
};

/**
 * @brief 非均匀校正算法。
 *
 * @tparam T_out 算法输出的像元数据类型
 * @tparam T_coeff 载入系数的像元数据类型
 *
 * @note 配合行迭代器使用
 */
template <typename T_out, typename T_coeff = float>
class NonUniformityCorrection : public UnaryOperation<cv::Mat> {
 public:
  cv::Mat operator()(cv::Mat m) const override {
    cv::Mat res;
    m.convertTo(m, cv::DataType<T_coeff>::type);
    m = m.mul(a_) + b_;
    m.convertTo(res, cv::DataType<T_out>::type);
    return res;
  }
  /**
   * @brief 载入非均匀系数。
   *
   * @param coeff_a 系数a路径
   * @param coeff_b 系数b路径
   */
  void load(const std::string& coeff_a, const std::string& coeff_b) {
    if (gdal::IsRasterDataset(coeff_a.c_str())) {
      a_ = load_raster<T_coeff>(coeff_a.c_str());
    } else {
      a_ = load_raster<T_coeff>(coeff_a.c_str());
    }
    if (gdal::IsRasterDataset(coeff_b.c_str())) {
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
 * @brief 绝对辐射校正算法。
 *
 * @tparam T_out
 * @tparam T_coeff
 *
 * @note 配合行迭代器使用
 */
template <typename T_out = float, typename T_coeff = float>
class AbsoluteRadiometricCorrection : public UnaryOperation<cv::Mat> {
 public:
  cv::Mat operator()(cv::Mat m) const override {
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
 * @brief 高斯滤波算法。
 *
 */
class GaussianFilter : public UnaryOperation<cv::Mat> {
 public:
  cv::Mat operator()(cv::Mat m) const override {
    cv::Mat res;
    cv::GaussianBlur(m, res, cv::Size(3, 3), 0, 0);
    return res;
  }
};

/**
 * @brief 空间维盲元修复算法。
 *
 * @details
 * 内部调用OpenCV中的`inpaint`函数实现，使用`cv::INPAINT_TELEA`算法。
 * 使用示例如下：
 * \code{.cpp}
 *  hsp::SpatialDefectivePixelCorrection dpc;
 *  dpc.load(badpixel);
 * 
 *  hsp::BandInputIterator<uint16_t> band_it(src_dataset.get(), 0),
 *     band_it_end(src_dataset.get());
 *  hsp::BandOutputIterator<uint16_t> band_out_it(dst_dataset.get(), 0);
 * 
 *  // 用 boost::counting_iterator 生成连续的波段号 
 *  std::transform(band_it, band_it_end, boost::counting_iterator<int>(0),
 *                 band_out_it, dpc);
 * \endcode
 *
 * @note
 * 配合波段迭代器使用，需要同时给出波段号。由于是二元操作，所以不能放入hsp::UnaryOpCombo。
 */
class SpatialDefectivePixelCorrection {
 public:
  /**
   * @brief `cv::inpaint`算法中的邻域半径。
   *
   */
  double radius{3.0};

 public:
  cv::Mat operator()(cv::Mat img, int band) const {
    cv::Mat mask = dpm_.row(band);
    mask.step[0] = 0;
    mask.rows = img.rows;
    cv::Mat res;
    cv::inpaint(img, mask, res, radius, cv::INPAINT_TELEA);
    return res;
  }
  void load(const std::string& filename) {
    dpm_ = hsp::load_raster<uint8_t>(filename);
  }

 private:
  cv::Mat dpm_;
};

/**
 * @brief 光谱维盲元修复算法。
 *
 * @note 配合行迭代器使用。
 */
class SpectralDefectivePixelCorrection : public UnaryOperation<cv::Mat> {
 public:
  /**
   * @brief `cv::inpaint`算法中的邻域半径。
   *
   */
  double radius{3.0};

 public:
  cv::Mat operator()(cv::Mat img) const override {
    cv::Mat res;
    cv::inpaint(img, dpm_, res, radius, cv::INPAINT_TELEA);
    return res;
  }
  void load(const std::string& filename) {
    dpm_ = hsp::load_raster<uint8_t>(filename);
  }

 private:
  cv::Mat dpm_;
};

}  // namespace hsp

#endif  // HSP_ALGORITHM_RADIOMETRIC_HPP_
