/**
 * @file AHSI_specific.hpp
 * @author xiaoyc
 * @brief 专用于AHSI的算法。
 * @version 0.1
 * @date 2023-11-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_ALGORITHM_AHSI_SPECIFIC_HPP_
#define HSP_ALGORITHM_AHSI_SPECIFIC_HPP_

// C++ Standard
#include <string>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
// HAVE_CUDA is defined in cvconfig.h
#include <opencv2/cvconfig.h>  // NOLINT
#ifdef HAVE_CUDA
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#endif  // HAVE_CUDA

// hsp
#include "../decoder/AHSIData.hpp"
#include "../utils.hpp"

namespace hsp {

/**
 * @brief 高五01A专用暗电平扣除算法。
 *
 * @details
 * 按照以下公式扣除暗电平：DN' = DN - (a * idx + b)。
 * 其中，DN和DN'分别为原始和校正后的DN值，a和b为暗电平系数，idx为帧序列号。
 *
 */
class GF501A_DBC {
 public:
  using CoeffDataType = float;
  cv::Mat operator()(const AHSIFrame& frame) const {
    cv::Mat dark;
    cv::Mat tmp = a_ * static_cast<CoeffDataType>(frame.index) + b_;
    tmp.convertTo(dark, cv::DataType<uint16_t>::type);
    return frame.data - dark;
  }
  void load(const std::string& a, const std::string& b) {
    a_ = hsp::load_raster<CoeffDataType>(a);
    b_ = hsp::load_raster<CoeffDataType>(b);
  }

 private:
  cv::Mat a_;
  cv::Mat b_;
};

#ifdef HAVE_CUDA
namespace cuda {

using cv::cuda::add;
using cv::cuda::GpuMat;
using cv::cuda::multiply;
using cv::cuda::subtract;

/**
 * @brief CUDA加速版本的高五01A专用暗电平扣除算法。
 *
 */
class GF501A_DBC {
 public:
  GpuMat operator()(const hsp::AHSIFrame& frame) {
    GpuMat img_orig, img, idx_res, dark_res, sub_res, res;
    img_orig.upload(frame.data);
    img_orig.convertTo(img, cv::DataType<double>::type);
    multiply(a_, frame.index, idx_res);
    add(idx_res, b_, dark_res);
    subtract(img, dark_res, sub_res);
    sub_res.convertTo(res, cv::DataType<uint16_t>::type);
    return res;
  }
  void load(const std::string& dark_a, const std::string& dark_b) {
    cv::Mat a0 = load_raster<double>(dark_a);
    cv::Mat b0 = load_raster<double>(dark_b);
    a_.upload(a0);
    b_.upload(b0);
  }

 private:
  GpuMat a_;
  GpuMat b_;
};

/**
 * @brief CUDA加速版本的高五01A VNIR相机辐射校正算法。
 *
 * @details 先扣除暗电平，再分别进行Etalon效应校正和非均匀校正。
 *
 */
class GF501A_VN_proc {
 public:
  cv::Mat operator()(const AHSIFrame& frame) {
    GpuMat img_orig, img, img_res, idx_res, sub_res, res_gpu, res_uint;
    img_orig.upload(frame.data);
    img_orig.convertTo(img, cv::DataType<double>::type);
    multiply(img, img_gain_, img_res);
    multiply(idx_gain_, frame.index, idx_res);
    subtract(img_res, idx_res, sub_res);
    add(sub_res, offset_, res_gpu);
    res_gpu.convertTo(res_uint, cv::DataType<uint16_t>::type);
    cv::Mat res;
    res_uint.download(res);
    return res;
  }

  void load(const std::string& dark_a, const std::string& dark_b,
            const std::string& etalon_a, const std::string& etalon_b,
            const std::string& rel_a, const std::string& rel_b) {
    auto a0 = load_raster<double>(dark_a);
    auto b0 = load_raster<double>(dark_b);
    auto a1 = load_raster<double>(etalon_a);
    auto b1 = load_raster<double>(etalon_b);
    auto a2 = load_raster<double>(rel_a);
    auto b2 = load_raster<double>(rel_b);
    cv::Mat img_gain = a1.mul(a2);
    cv::Mat idx_gain = a0.mul(a1).mul(a2);
    cv::Mat offset = b1.mul(a2) + b2 - a1.mul(a2).mul(b0);
    img_gain_.upload(img_gain);
    idx_gain_.upload(idx_gain);
    offset_.upload(offset);
  }

 private:
  GpuMat img_gain_;
  GpuMat idx_gain_;
  GpuMat offset_;
};

}  // namespace cuda

#endif  // HAVE_CUDA

}  // namespace hsp

#endif  // HSP_ALGORITHM_AHSI_SPECIFIC_HPP_
