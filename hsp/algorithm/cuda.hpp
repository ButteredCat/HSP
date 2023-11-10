/**
 * @file cuda.hpp
 * @author xiaoyc
 * @brief 使用GPU加速的算法，需要用到NVIDIA CUDA和OpenCV中的CUDA模块。
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_ALGORITHM_CUDA_HPP_
#define HSP_ALGORITHM_CUDA_HPP_

// C++ Standard
#include <memory>
#include <string>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>

// project
#include "../gdalex.hpp"
#include "../utils.hpp"
#include "./operation.hpp"

namespace hsp {

namespace cuda {
using cv::Mat;
using cv::cuda::GpuMat;

using unary_op = std::shared_ptr<UnaryOperation<GpuMat>>;

/**
 * @brief 用于容纳一元操作组合器。
 *
 */
class UnaryOpCombo : public UnaryOperation<GpuMat> {
 public:
  /**
   * @brief 按照添加顺序，运行组合器中添加的算法。
   *
   * @param m
   * @return cv::cuda::GpuMat
   */
  GpuMat operator()(GpuMat m) const override {
    for (auto&& each : ops_) {
      m = each->operator()(m);
    }
    return m;
  }

  /**
   * @brief 在组合器中添加操作。
   *
   * @param op 一元图像操作指针
   * @return UnaryOpCombo&
   */
  UnaryOpCombo& add(unary_op op) {
    ops_.emplace_back(op);
    return *this;
  }

  UnaryOpCombo& remove_back() {
    ops_.pop_back();
    return *this;
  }
  std::size_t size() const { return ops_.size(); }
  bool empty() const { return ops_.empty(); }

 private:
  std::vector<unary_op> ops_;
};

static auto GpuUploader = [](Mat m) {
  GpuMat res;
  res.upload(m);
  return res;
};

static auto GpuDownloader = [](GpuMat m) {
  Mat res;
  m.download(res);
  return res;
};

/**
 * @brief 基于CUDA的暗电平扣除算法。
 *
 * @tparam T 载入系数的像元数据类型
 */
template <typename T>
class DarkBackgroundCorrection : public hsp::UnaryOperation<GpuMat> {
 public:
  GpuMat operator()(GpuMat m) const override {
    GpuMat res;
    cv::cuda::subtract(m, m_, res);
    return res;
  }
  void load(const std::string& filename) {
    cv::Mat coeff;
    if (hsp::gdal::IsRasterDataset(filename.c_str())) {
      coeff = hsp::load_raster<T>(filename.c_str());
    } else {
      coeff = hsp::load_text<T>(filename.c_str());
    }
    m_.upload(coeff);
  }

 private:
  GpuMat m_;
};

/**
 * @brief 基于CUDA的非均匀校正算法。
 *
 * @tparam T_out 算法输出的像元数据类型
 * @tparam T_coeff 载入系数的像元数据类型
 */
template <typename T_out, typename T_coeff = float>
class NonUniformityCorrection : public UnaryOperation<GpuMat> {
 public:
  GpuMat operator()(GpuMat m) const override {
    GpuMat img, mid, mid2, res;
    m.convertTo(img, cv::DataType<T_coeff>::type);
    cv::cuda::multiply(img, a_, mid);
    cv::cuda::add(mid, b_, mid2);
    mid2.convertTo(res, cv::DataType<T_out>::type);
    return res;
  }
  void load(const std::string& coeff_a, const std::string& coeff_b) {
    cv::Mat a, b;
    if (hsp::gdal::IsRasterDataset(coeff_a.c_str())) {
      a = load_raster<T_coeff>(coeff_a.c_str());
    } else {
      a = load_raster<T_coeff>(coeff_a.c_str());
    }
    if (hsp::gdal::IsRasterDataset(coeff_b.c_str())) {
      b = load_raster<T_coeff>(coeff_b.c_str());
    } else {
      b = load_raster<T_coeff>(coeff_b.c_str());
    }
    a_.upload(a);
    b_.upload(b);
  }

 private:
  cv::cuda::GpuMat a_;
  cv::cuda::GpuMat b_;
};

/**
 * @brief 基于CUDA的高斯滤波算法。
 *
 * @tparam T 算法输出的像元数据类型
 *
 * @note 不支持双精度数据类型。
 */
template <typename T>
class GaussianFilter : public hsp::UnaryOperation<GpuMat> {
 public:
  GpuMat operator()(GpuMat m) const override {
    GpuMat res;
    auto ftr = cv::cuda::createGaussianFilter(
        cv::DataType<T>::type, cv::DataType<T>::type, cv::Size(3, 3), 1.0, 1.0);
    ftr->apply(m, res);
    return res;
  }
};

// template <typename T_out, typename T_coeff = float>
// class UnifiedOps : public UnaryOperation {
//  public:
//   cv::Mat operator()(cv::Mat m) const override {
//     cv::cuda::GpuMat img, mid_gpu, res_gpu, blured;
//     cv::Mat res;
//     img.upload(m);
//     cv::cuda::multiply(img, a_, mid_gpu);
//     cv::cuda::add(mid_gpu, b_, res_gpu);
//     auto fltr = cv::cuda::createGaussianFilter(cv::DataType<T_out>::type,
//                                                cv::DataType<T_out>::type,
//                                                cv::Size(3, 3), 1.0, 1.0);
//     fltr->apply(res_gpu, blured);
//     blured.download(res);
//     return res;
//   }
//   void load(const std::string& coeff_a, const std::string& coeff_b) {
//     cv::Mat a, b;
//     if (hsp::gdal::IsRasterDataset(coeff_a.c_str())) {
//       a = load_raster<T_coeff>(coeff_a.c_str());
//     } else {
//       a = load_raster<T_coeff>(coeff_a.c_str());
//     }
//     if (hsp::gdal::IsRasterDataset(coeff_b.c_str())) {
//       b = load_raster<T_coeff>(coeff_b.c_str());
//     } else {
//       b = load_raster<T_coeff>(coeff_b.c_str());
//     }
//     a_.upload(a);
//     b_.upload(b);
//   }

//  private:
//   cv::cuda::GpuMat a_;
//   cv::cuda::GpuMat b_;
// };

}  // namespace cuda

}  // namespace hsp

#endif  // HSP_ALGORITHM_CUDA_HPP_
