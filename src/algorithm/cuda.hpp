// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_ALGORITHM_CUDA_HPP_
#define SRC_ALGORITHM_CUDA_HPP_

// C++ Standard
#include <string>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>

// project
#include "../gdalex.hpp"
#include "./operation.hpp"
#include "./radiometric.hpp"

namespace hsp {

namespace cuda {

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
    if (IsRasterDataset(filename.c_str())) {
      coeff = hsp::load_raster<T>(filename.c_str());
    } else {
      coeff = hsp::load_text<T>(filename.c_str());
    }
    m_.upload(coeff);
  }

 private:
  cv::cuda::GpuMat m_;
};
}  // namespace cuda

}  // namespace hsp

#endif  // SRC_ALGORITHM_CUDA_HPP_
