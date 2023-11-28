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
 * @brief 盲元修复算法名称。
 *
 */
enum class Inpaint { TELEA, MEAN_BLUR };

/**
 * @brief 带掩膜（mask）的均值滤波。只修改mask对应点值为1的点。
 *
 * @param input 输入矩阵。
 * @param ksize 均值滤波核尺寸。
 * @param mask 掩膜。
 * @return cv::Mat
 */
static cv::Mat meanBlur(cv::Mat input, int ksize, cv::Mat mask) {
  cv::Mat blured, mask_cvt;
  cv::medianBlur(input, blured, ksize);
  mask.convertTo(mask_cvt, input.type());
  return input.mul(1 - mask_cvt) + blured.mul(mask_cvt);
}

/**
 * @brief 空间维盲元修复算法。
 *
 * @details
 * 内部调用OpenCV中的`inpaint`函数实现，使用`cv::INPAINT_TELEA`算法。
 * 使用示例如下：
 * \code{.cpp}
 *  hsp::DefectivePixelCorrectionSpatial dpc;
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
class DefectivePixelCorrectionSpatial {
 public:
  /**
   * @brief `cv::inpaint`算法中的邻域半径。
   *
   */
  double radius{3.0};

  /**
   * @brief 均值平滑算法中的核尺寸。
   *
   */
  int ksize{3};

 public:
  cv::Mat operator()(cv::Mat img, int band) const {
    cv::Mat mask = dpm_.row(band);
    mask.step[0] = 0;
    mask.rows = img.rows;
    cv::Mat res;
    switch (inpaint_) {
      case Inpaint::MEAN_BLUR:
        res = meanBlur(img, ksize, mask);
        break;
      default:
        cv::inpaint(img, mask, res, radius, cv::INPAINT_TELEA);
    }
    return res;
  }

  /**
   * @brief 载入盲元列表。盲元列表为 n_samples * n_bands
   * 尺寸的 Tiff 文件。1代表该位置为盲元，0代表不是盲元。
   *
   * @param filename 盲元列表路径。
   */
  void load(const std::string& filename) {
    dpm_ = hsp::load_raster<uint8_t>(filename);
  }

  /**
   * @brief 设置盲元修复方法。
   *
   * @param value
   */
  void set_inpaint(Inpaint value) { inpaint_ = value; }

 private:
  cv::Mat dpm_;
  Inpaint inpaint_{Inpaint::TELEA};
};

/**
 * @brief 光谱维盲元修复算法。
 *
 * @note 配合行迭代器使用。
 */
class DefectivePixelCorrectionSpectral : public UnaryOperation<cv::Mat> {
 public:
  /**
   * @brief `cv::inpaint`算法中的邻域半径。
   *
   */
  double radius{3.0};

  /**
   * @brief 均值平滑算法中的核尺寸。
   *
   */
  int ksize{3};

 public:
  cv::Mat operator()(cv::Mat img) const override {
    cv::Mat res;
    switch (inpaint_) {
      case Inpaint::MEAN_BLUR:
        res = meanBlur(img, ksize, dpm_);
        break;
      default:
        cv::inpaint(img, dpm_, res, radius, cv::INPAINT_TELEA);
    }
    return res;
  }

  /**
   * @brief 载入盲元列表。盲元列表为 n_samples * n_bands
   * 尺寸的 Tiff 文件。1代表该位置为盲元，0代表不是盲元。
   *
   * @param filename 盲元列表路径。
   */
  void load(const std::string& filename) {
    dpm_ = hsp::load_raster<uint8_t>(filename);
  }

  /**
   * @brief 设置盲元修复方法。
   *
   * @param value
   */
  void set_inpaint(Inpaint value) { inpaint_ = value; }

 private:
  cv::Mat dpm_;
  Inpaint inpaint_{Inpaint::TELEA};
};

/**
 * @brief 对连续盲元进行特殊处理的盲元修复算法，在光谱维进行盲元修复。
 *
 */
class DefectivePixelCorrection : public UnaryOperation<cv::Mat> {
 public:
  cv::Mat operator()(cv::Mat img) const override { return img; }

  void load(const std::string& filename) {
    dpm_ = hsp::load_raster<uint8_t>(filename);
    find_consecutive();
  }

  /**
   * @brief
   * 返回行连续盲元标签矩阵。0代表不是盲元，1代表该盲元左右无紧邻的盲元，n>1代表该盲元左右（含自身）共有n个紧邻盲元。
   *
   * @return cv::Mat
   */
  cv::Mat get_row_label() const { return row_label_; }

  /**
   * @brief
   * 返回列连续盲元标签矩阵。0代表不是盲元，1代表该盲元上下无紧邻的盲元，n>1代表该盲元上下（含自身）共有n个紧邻盲元。
   *
   * @return cv::Mat
   */
  cv::Mat get_col_label() const { return col_label_; }

 private:
  cv::Mat dpm_;
  cv::Mat row_label_;
  cv::Mat col_label_;

 private:
  void find_consecutive() {
    using LabelType = uint16_t;
    row_label_ = cv::Mat::zeros(dpm_.size(), cv::DataType<LabelType>::type);
    col_label_ = cv::Mat::zeros(dpm_.size(), cv::DataType<LabelType>::type);
    // row label
    for (int i = 0; i < dpm_.rows; ++i) {
      row_label_.at<LabelType>(i, 0) = dpm_.at<uint8_t>(i, 0);
      for (int j = 1; j < dpm_.cols; ++j) {
        if (dpm_.at<uint8_t>(i, j) == 1) {
          row_label_.at<LabelType>(i, j) =
              row_label_.at<LabelType>(i, j - 1) + 1;
        }
      }
    }
    for (int i = dpm_.rows - 1; i >= 0; --i) {
      for (int j = dpm_.cols - 2; j >= 0; --j) {
        if (row_label_.at<LabelType>(i, j) != 0 &&
            row_label_.at<LabelType>(i, j + 1) != 0) {
          row_label_.at<LabelType>(i, j) = row_label_.at<LabelType>(i, j + 1);
        }
      }
    }
    // column label
    for (int j = 0; j < dpm_.cols; ++j) {
      col_label_.at<LabelType>(0, j) = dpm_.at<uint8_t>(0, j);
      for (int i = 1; i < dpm_.rows; ++i) {
        if (dpm_.at<uint8_t>(i, j) == 1) {
          col_label_.at<LabelType>(i, j) =
              col_label_.at<LabelType>(i - 1, j) + 1;
        }
      }
    }
    for (int j = dpm_.cols - 1; j >= 0; --j) {
      for (int i = dpm_.rows - 2; i >= 0; --i) {
        if (col_label_.at<LabelType>(i, j) != 0 &&
            col_label_.at<LabelType>(i + 1, j) != 0) {
          col_label_.at<LabelType>(i, j) = col_label_.at<LabelType>(i + 1, j);
        }
      }
    }
  }
};

}  // namespace hsp

#endif  // HSP_ALGORITHM_RADIOMETRIC_HPP_
