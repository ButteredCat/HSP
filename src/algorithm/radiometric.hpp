// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_ALGORITHM_RADIOMETRIC_HPP_
#define SRC_ALGORITHM_RADIOMETRIC_HPP_

// project
#include "./operation.hpp"

namespace hsp {
class DarkBackgroundCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override { return m; }
};

class NonUniformityCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override { return m; }
};
}  // namespace hsp

#endif  // SRC_ALGORITHM_RADIOMETRIC_HPP_
