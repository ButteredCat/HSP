/**
 * @file operation.hpp
 * @author xiaoyc
 * @brief 定义基于函数对象的基本图像处理操作。
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_ALGORITHM_OPERATION_HPP_
#define HSP_ALGORITHM_OPERATION_HPP_

// C++ Standard
#include <memory>
#include <vector>

// OpenCV
#include <opencv2/core.hpp>

namespace hsp {

/**
 * @brief 一元图像操作。
 *
 * @param[in] cv::Mat 输入图像矩阵
 * @return cv::Mat 处理后的图像矩阵
 *
 */
template <typename T>
class UnaryOperation {
 public:
  virtual T operator()(T) const = 0;
};

using unary_op = std::shared_ptr<UnaryOperation<cv::Mat>>;

/**
 * @brief 构造一元图像操作指针。
 *
 * @tparam T 图像像元的数据类型
 */
template <typename T>
const auto make_op = std::make_shared<T>;

/**
 * @brief 一元操作组合器。
 *
 */
class UnaryOpCombo : public UnaryOperation<cv::Mat> {
 public:
  /**
   * @brief 按照添加顺序，运行组合器中添加的算法。
   *
   * @param m
   * @return cv::Mat
   */
  cv::Mat operator()(cv::Mat m) const override {
    cv::Mat res = m;
    for (auto&& each : ops_) {
      res = each->operator()(res);
    }
    return res;
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

}  // namespace hsp

#endif  // HSP_ALGORITHM_OPERATION_HPP_
