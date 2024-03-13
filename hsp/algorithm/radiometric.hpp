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

// #define __DEBUG__

// C++ Standard
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

// spdlog
#ifdef __DEBUG__
#include <spdlog/spdlog.h>
#endif

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
template <typename T_coeff = float>
class DarkBackgroundCorrection : public UnaryOperation<cv::Mat> {
 public:
  cv::Mat operator()(cv::Mat m) const override { return m - m_; }

  /**
   * @brief 载入暗电平系数文件。
   *
   * @param filename 系数文件路径，支持栅格数据和文本文件
   */
  void load(const std::string& filename) {
    // if (gdal::IsRasterDataset(filename.c_str())) {
    m_ = load_raster<T_coeff>(filename.c_str());
    // } else {
    //   m_ = load_text<T>(filename.c_str());
    // }
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
enum class Inpaint {
  TELEA,                 /**< 调用 OpenCV 中的 cv::INPAINT_TELEA 算法  */
  NEIGHBORHOOD_AVERAGING /**< 8邻域插值算法  */
};

/**
 * @brief 8领域插值。只修改mask对应点值为1的点。
 *
 * @param input 输入矩阵。
 * @param output 输出矩阵。
 * @param mask 掩膜。
 */
static cv::Mat neighborhood_averaging(cv::Mat input, cv::Mat mask) {
  cv::Mat kernel = (cv::Mat_<float>(3, 3) << 1, 1, 1, 1, 0, 1, 1, 1, 1) / 8;
  cv::Mat filtered, mask_cvt;
  cv::filter2D(input, filtered, -1, kernel);
  mask.convertTo(mask_cvt, input.type());
  return input.mul(1 - mask_cvt) + filtered.mul(mask_cvt);
}

/**
 * @brief 空间维盲元修复算法。
 *
 * @details
 * 可选择使用`cv::INPAINT_TELEA`或8邻域均值算法修复盲元。
 *
 * @par Sample
 * @code{.cpp}
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
 * @endcode
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

 public:
  cv::Mat operator()(cv::Mat img, int band) const {
    cv::Mat mask = dpm_.row(band);
    mask.step[0] = 0;
    mask.rows = img.rows;
    cv::Mat res;
    switch (inpaint_) {
      case Inpaint::NEIGHBORHOOD_AVERAGING:
        res = neighborhood_averaging(img, mask);
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
 * @brief
 * 光谱维盲元修复算法。可选择使用`cv::INPAINT_TELEA`或8邻域均值算法修复盲元。
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

 public:
  cv::Mat operator()(cv::Mat img) const override {
    cv::Mat res;
    switch (inpaint_) {
      case Inpaint::NEIGHBORHOOD_AVERAGING:
        res = neighborhood_averaging(img, dpm_);
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
 * @brief 基于反距离权重法（Inverse Distance
 * Weighting）的盲元修复算法，对连续盲元进行特殊处理。
 *
 * @note 配合行迭代器使用。
 *
 */
class DefectivePixelCorrectionIDW : public UnaryOperation<cv::Mat> {
 public:
  using ComputingType = float;
  const ComputingType NaN = std::numeric_limits<ComputingType>::quiet_NaN();
  const ComputingType inf = std::numeric_limits<ComputingType>::infinity();

 public:
  cv::Mat operator()(cv::Mat img) const override {
    cv::Mat img_1d, padded;
    if (img.type() != CV_32F) {
      img.convertTo(img_1d, CV_32F);
    } else {
      img_1d = img.clone();
    }
    img_1d.setTo(cv::Scalar::all(Invalid), dpm_ != 0);
    cv::copyMakeBorder(img_1d, padded, max_win_spectral_, max_win_spectral_,
                       max_win_spatial_, max_win_spatial_, cv::BORDER_CONSTANT,
                       cv::Scalar_<ComputingType>::all(Invalid));
#ifndef __DEBUG__
#pragma omp parallel for
#endif  // __DEBUG__
    for (int i = 0; i < dp_list_.size(); ++i) {
      std::array<uint16_t, 4> log_info{};
      const cv::Point& defective_pixel = dp_list_[i];
      auto win_spatial = row_label_.at<LabelType>(defective_pixel);
      auto win_spectral = col_label_.at<LabelType>(defective_pixel);
      cv::Mat idw_t(inverse_weights_table_,
                    cv::Range(max_win_spectral_ - win_spectral,
                              max_win_spectral_ + win_spectral + 1),
                    cv::Range(max_win_spatial_ - win_spatial,
                              max_win_spatial_ + win_spatial + 1));
      cv::Mat window_t(
          padded,
          cv::Range(max_win_spectral_ + defective_pixel.y - win_spectral,
                    max_win_spectral_ + defective_pixel.y + win_spectral + 1),
          cv::Range(max_win_spatial_ + defective_pixel.x - win_spatial,
                    max_win_spatial_ + defective_pixel.x + win_spatial + 1));
      cv::Mat window, idw;
      cv::transpose(window_t, window);
      cv::transpose(idw_t, idw);
      const cv::Point window_center(window.rows / 2, window.cols / 2);
      double mean_window = cv::mean(window, window == window)[0];
      cv::Mat mean_stddev_window = meanStdDev(window);
      cv::Mat stddev_window = mean_stddev_window.row(1);
      if (std::any_of(stddev_window.begin<ComputingType>(),
                      stddev_window.end<ComputingType>(),
                      [mean_window](ComputingType stddev) {
                        return stddev > 0.1 * mean_window;
                      })) {
        double max_DN, min_DN;
        cv::Point max_Loc, min_Loc;
        cv::minMaxLoc(window, &min_DN, &max_DN, &min_Loc, &max_Loc);
        cv::Mat1f window0 = window.clone();
        auto n_max = cv::sum(window0 == max_DN);
        auto n_min = cv::sum(window0 == min_DN);
        window0.setTo(cv::Scalar::all(Invalid), window0 == min_DN);
        window0.setTo(cv::Scalar::all(Invalid), window0 == max_DN);
        auto mst1 = median(window0);
        cv::Mat ratio = get_ratio_mat(window0);
        if (cv::countNonZero(isInvalid(ratio)) != 0) {
          auto alt_max = mst1.at<ComputingType>(0, max_Loc.x);
          auto alt_min = mst1.at<ComputingType>(0, min_Loc.x);
          if (!isInvalid(alt_max)) {
            window.setTo(cv::Scalar::all(alt_max), window == max_DN);
            log_info[2] = 1;
          }
          if (!isInvalid(alt_min)) {
            window.setTo(cv::Scalar::all(alt_min), window == min_DN);
            log_info[2] = 1;
          }
        }
      }
      uint16_t patch = get_patch(window, idw);
      log_info[0] = patch;
      window.at<ComputingType>(window_center.x, window_center.y) = patch;
      cv::Mat spb = get_ratio_mat(window);
      // cv::Mat inf_mask = (spb ==
      // std::numeric_limits<ComputingType>::infinity());
      cv::Mat Tpb = spb.row(window_center.x).clone();
      spb.row(window_center.x) = Invalid;
      // setCorrespondingToNaN(window, spb);
      window.setTo(cv::Scalar::all(Invalid), isInvalid(spb));
      auto TA1 = isoutlier(spb);
      auto TA2 = isoutlier(window);
      cv::Mat spb_vec = spb.reshape(0, spb.rows * spb.cols);
      auto TA3 = isoutlier(spb_vec).reshape(0, spb.rows);
      cv::MatExpr outlier_mask = (TA1 + TA2 + TA3 != 0);
      spb.setTo(cv::Scalar::all(Invalid), outlier_mask);
      spb.setTo(cv::Scalar::all(Invalid), spb == 0);
      window.setTo(cv::Scalar::all(Invalid), outlier_mask);
      auto mean_stddev_spb = meanStdDev(spb);
      auto mean_spb = mean_stddev_spb.row(0);
      auto stddev_spb = mean_stddev_spb.row(1);
      cv::Mat1i sum = TA1.row(window_center.x) + TA2.row(window_center.x) +
                      isInvalid(window.row(window_center.x)) +
                      isInvalid(mean_spb);
      cv::Mat window2;
      if (std::any_of(sum.begin(), sum.end(), [](int i) { return i == 0; })) {
        window2 = window.row(window_center.x);
      } else {
        window2 = mean(window);
      }

      if (win_spatial < 0.8 * img.rows && win_spectral < 0.8 * img.cols &&
          (cv::sum(Tpb <= mean_spb - stddev_spb)[0] != 0 ||
           cv::sum(Tpb >= mean_spb + stddev_spb)[0] != 0 ||
           cv::sum(~isInvalid(Tpb + mean_spb))[0] == 0)) {
        cv::Mat idw_mid_row = idw.row(window_center.x).clone();
        idw_mid_row.setTo(cv::Scalar::all(0.0), isInvalid(window2));
        idw_mid_row.setTo(cv::Scalar::all(0.0), isInvalid(mean_spb));
        // setCorrespondingToNaN(idw_mid_row, window2);
        // setCorrespondingToNaN(idw_mid_row, mean_spb);
        // cv::patchNaNs(idw_mid_row);

        // cv::Mat mean_spbf = mean_spb.clone();
        //  cv::patchNaNs(mean_spbf);
        // mean_spb.setTo(cv::Scalar::all(0.0), isInvalid(mean_spb));
        // window2.setTo(cv::Scalar::all(0.0), isInvalid(window2));
        uint16_t patch_alt = get_patch(window2.mul(mean_spb), idw_mid_row);
        log_info[1] = patch_alt;
        if (patch_alt != 0) {
          log_info[3] = 1;
          patch = patch_alt;
        }
      }
      img.at<uint16_t>(defective_pixel) = patch;
#ifdef __DEBUG__
      spdlog::debug("({}, {}), patch={}, patch_alt={}, final={}, repaired={}",
                    defective_pixel.y, defective_pixel.x, log_info[0],
                    log_info[1], patch, log_info[2]);
#endif
    }
    return img;
  }

  void load(const std::string& filename) {
    dpm_ = hsp::load_raster<uint8_t>(filename);
    construct_dp_list();
    find_consecutive();
    double max;
    cv::minMaxLoc(row_label_, nullptr, &max);
    max_win_spatial_ = static_cast<int>(max);
    cv::minMaxLoc(col_label_, nullptr, &max);
    max_win_spectral_ = static_cast<int>(max);
    inverse_weights_table_ = get_inverse_weights_table(
        2 * max_win_spectral_ + 1, 2 * max_win_spatial_ + 1);
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
  using LabelType = uint16_t;
  cv::Mat dpm_;
  cv::Mat row_label_;
  cv::Mat col_label_;
  cv::Mat inverse_weights_table_;
  std::vector<cv::Point> dp_list_;
  int max_win_spatial_ = 1;
  int max_win_spectral_ = 1;

 private:
  /**
   * @brief
   *
   */
  cv::Mat get_ratio_mat(const cv::Mat& m) const {
    cv::Mat res;
    cv::Mat m1 = m.clone();
    m1.setTo(cv::Scalar::all(0.0), isInvalid(m1));
    cv::divide(cv::repeat(m.col(m.cols / 2), 1, m.cols), m1, res);
    cv::patchNaNs(res, Invalid);
    res.setTo(cv::Scalar::all(Invalid), res == inf);
    return res;
  }

  /**
   * @brief 计算补丁值
   */
  uint16_t get_patch(const cv::Mat& window, const cv::Mat& idw) const {
    cv::Mat idw_patched = idw.clone();
    idw_patched.setTo(cv::Scalar::all(0.0), isInvalid(window));
    double idw_sum = cv::sum(idw_patched)[0];
    if (idw_sum == 0) {
      return 0;
    }
    cv::MatExpr prod = window.mul(idw_patched / cv::sum(idw_patched)[0]);
    return static_cast<uint16_t>(std::round(cv::sum(prod)[0]));
  }

  /**
   * @brief 将盲元矩阵转为盲元列表
   *
   */
  void construct_dp_list() {
    for (int i = 0; i < dpm_.rows; ++i) {
      for (int j = 0; j < dpm_.cols; ++j) {
        if (dpm_.at<uint8_t>(i, j) == 1) {
          dp_list_.emplace_back(j, i);
        }
      }
    }
  }

  /**
   * @brief 按照给定尺寸，给出反距离权重矩阵。
   *
   * @param rows 矩阵行数
   * @param cols 矩阵列数
   * @return cv::Mat
   */
  cv::Mat1f get_inverse_weights_table(int rows, int cols) const {
    cv::Point center(rows / 2, cols / 2);
    cv::Mat1f col_idx =
        cv::Mat::zeros(1, cols, cv::DataType<ComputingType>::type);
    std::iota(col_idx.begin(), col_idx.end(), 0);
    cv::Mat col_idx_mat = cv::repeat(col_idx, rows, 1);
    cv::Mat1f row_idx =
        cv::Mat::zeros(rows, 1, cv::DataType<ComputingType>::type);
    std::iota(row_idx.begin(), row_idx.end(), 0);
    cv::Mat row_idx_mat = cv::repeat(row_idx, 1, cols);

    cv::Mat1f distance, inv_d;
    cv::magnitude(center.x - row_idx_mat, center.y - col_idx_mat, distance);
    cv::divide(1.0, distance, inv_d);
    inv_d.at<ComputingType>(center.x, center.y) = 0;
    return inv_d;
  }

  void find_consecutive() {
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
