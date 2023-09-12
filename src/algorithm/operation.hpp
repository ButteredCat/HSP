// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_ALGORITHM_OPERATION_HPP_
#define SRC_ALGORITHM_OPERATION_HPP_

// C++ Standard
#include <memory>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>

namespace hsp {
class UnaryOperation {
 public:
  virtual cv::Mat operator()(cv::Mat) = 0;
};

using unary_op = std::shared_ptr<UnaryOperation>;

template <typename T>
const auto make_op = std::make_shared<T>;

class UnaryOpCombo : public UnaryOperation {
 public:
  UnaryOpCombo& add(unary_op op) {
    ops_.emplace_back(op);
    return *this;
  }
  cv::Mat operator()(cv::Mat m) override {
    cv::Mat res = m;
    for (auto&& each : ops_) {
      res = each->operator()(res);
    }
    return res;
  }

 private:
  std::vector<unary_op> ops_;
};

}  // namespace hsp

#endif  // SRC_ALGORITHM_OPERATION_HPP_
