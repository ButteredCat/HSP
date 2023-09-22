/**
 * @file cuda.hpp
 * @author xiaoyc
 * @brief 使用GPU加速的算法，需要NVIDIA CUDA和cv::cuda支持
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SRC_ALGORITHM_CUDA_HPP_
#define SRC_ALGORITHM_CUDA_HPP_

// C++ Standard
#include <string>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>

// project
#include "../gdalex.hpp"
#include "./operation.hpp"
#include "./radiometric.hpp"

namespace hsp {

namespace cuda {

/**
 * @brief 基于CUDA的暗电平扣除算法
 *
 * @tparam T 载入系数的像元数据类型
 */
template <typename T>
class DarkBackgroundCorrection : public hsp::UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::cuda::GpuMat img, res_gpu;
    cv::Mat res;
    img.upload(m);
    cv::cuda::subtract(img, m_, res_gpu);
    res_gpu.download(res);
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
  cv::cuda::GpuMat m_;
};

/**
 * @brief 基于CUDA的非均匀校正算法
 *
 * @tparam T_out 算法输出的像元数据类型
 * @tparam T_coeff 载入系数的像元数据类型
 */
template <typename T_out, typename T_coeff = float>
class NonUniformityCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::cuda::GpuMat img, mid_gpu, res_gpu;
    cv::Mat mm, res;
    m.convertTo(mm, cv::DataType<T_coeff>::type);
    img.upload(mm);
    cv::cuda::multiply(img, a_, mid_gpu);
    cv::cuda::add(mid_gpu, b_, res_gpu);
    res_gpu.download(mm);
    mm.convertTo(res, cv::DataType<T_out>::type);
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
 * @brief 基于CUDA的高斯滤波算法
 *
 * @tparam T 算法输出的像元数据类型
 *
 * @note 不支持双精度数据类型
 */
template <typename T>
class GaussianFilter : public hsp::UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::cuda::GpuMat src, dst;
    src.upload(m);
    auto ftr = cv::cuda::createGaussianFilter(
        cv::DataType<T>::type, cv::DataType<T>::type, cv::Size(3, 3), 1.0, 1.0);
    ftr->apply(src, dst);
    dst.download(m);
    return m;
  }
};

template <typename T_out, typename T_coeff = float>
class UnifiedOps : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::cuda::GpuMat img, mid_gpu, res_gpu, blured;
    cv::Mat res;
    img.upload(m);
    cv::cuda::multiply(img, a_, mid_gpu);
    cv::cuda::add(mid_gpu, b_, res_gpu);
    auto fltr = cv::cuda::createGaussianFilter(cv::DataType<T_out>::type,
                                               cv::DataType<T_out>::type,
                                               cv::Size(3, 3), 1.0, 1.0);
    fltr->apply(res_gpu, blured);
    blured.download(res);
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

}  // namespace cuda

}  // namespace hsp

#endif  // SRC_ALGORITHM_CUDA_HPP_
