// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_ALGORITHM_CUDA_HPP_
#define SRC_ALGORITHM_CUDA_HPP_

// C++ Standard
#include <memory>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>

// project
#include "./operation.hpp"

namespace hsp {

namespace cuda {

template <typename T>
class DarkBackgroundCorrection : public hsp::UnaryOperation {
 public:
  explicit DarkBackgroundCorrection(const std::string& filename) {}
  cv::Mat operator()(cv::Mat m) override { return m; }

 private:
  cv::Mat m_;
};
}  // namespace cuda

}  // namespace hsp

#endif  // SRC_ALGORITHM_CUDA_HPP_
