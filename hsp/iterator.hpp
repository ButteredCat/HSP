/**
 * @file iterator.hpp
 * @author xiaoyc
 * @brief 实现了对GDALDataset按照样本、行、波段进行迭代的输入输出迭代器。
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_ITERATOR_HPP_
#define HSP_ITERATOR_HPP_

// GDAL
#include <gdal.h>
#include <gdal_priv.h>

// C++ Standard
#include <exception>

// Boost
#include <boost/iterator/iterator_facade.hpp>

// OpenCV
#include <opencv2/core.hpp>

// project
#include "./gdal_traits.hpp"

namespace hsp {

/**
 * @brief 输入迭代器，用于特化为其他迭代器。
 *
 * @tparam T 读取影像像元的数据类型
 * @tparam N 特化迭代器类型，1为样本迭代器，2为波段迭代器，3为波段迭代器
 *
 *
 * @details
 * 输入迭代器InputIterator_基于boost::iterator_facade，封装了对GDALDataset所拥有的高光谱数据立方资源通过RasterIO进行的读取操作。
 *
 * 迭代器的值类型为cv::Mat const。
 *
 * 在实例化时，如果指定了当前的样本/行/波段号（均从0开始计数），那么当前数据会在构造函数中预读取。如果不指定，那么该实例为末端迭代器，仅用于表示数据集的末尾。
 *
 * 本迭代器类通过指定类型，特化为SampleInputIterator、LineInputIterator和BandInputIterator。
 */
template <typename T, unsigned N>
class InputIterator_
    : public boost::iterator_facade<InputIterator_<T, N>, cv::Mat const,
                                    boost::single_pass_traversal_tag> {
  friend boost::iterator_core_access;

 public:
  using reference = cv::Mat const&;
  /**
   * @brief 仅用于构造末端迭代器，用于表示数据集的末尾
   *
   * @param dataset 数据集指针
   */
  explicit InputIterator_(GDALDataset* dataset) : dataset_{dataset} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      switch (N) {
        case 1:
          cur_ = n_samples_;
          break;
        case 2:
          cur_ = n_lines_;
          break;
        default:
          cur_ = n_bands_;
      }
      max_idx_ = cur_;
    }
  }

  /**
   * @brief 用于构造输入迭代器。
   *
   * @note 构造时，会预读取cur指向的数据。
   *
   * @param dataset 数据集指针
   * @param cur 迭代开始位置，从0开始计数
   */
  InputIterator_(GDALDataset* dataset, int cur) : dataset_{dataset}, cur_{cur} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      CPLErr err;
      switch (N) {
        case 1:
          img_ = cv::Mat::zeros(cv::Size(n_lines_, n_bands_),
                                cv::DataType<T>::type);
          err = dataset_->RasterIO(GF_Read, cur_, 0, 1, n_lines_, img_.data, 1,
                                   n_lines_, gdal::DataType<T>::type(),
                                   n_bands_, nullptr, 0, 0, 0);
          max_idx_ = n_samples_;
          break;
        case 2:
          img_ = cv::Mat::zeros(cv::Size(n_samples_, n_bands_),
                                cv::DataType<T>::type);
          err = dataset_->RasterIO(GF_Read, 0, cur_, n_samples_, 1, img_.data,
                                   n_samples_, 1, gdal::DataType<T>::type(),
                                   n_bands_, nullptr, 0, 0, 0);
          max_idx_ = n_lines_;
          break;
        default:
          img_ = cv::Mat::zeros(cv::Size(n_samples_, n_lines_),
                                cv::DataType<T>::type);
          err = dataset_->GetRasterBand(cur_ + 1)->RasterIO(
              GF_Read, 0, 0, n_samples_, n_lines_, img_.data, n_samples_,
              n_lines_, gdal::DataType<T>::type(), 0, 0);
          max_idx_ = n_bands_;
      }
    }
  }

 private:
  GDALDataset* dataset_;
  int n_samples_{0};
  int n_lines_{0};
  int n_bands_{0};
  int cur_{0};
  int max_idx_{0};
  cv::Mat img_;

 private:
  bool equal(InputIterator_ const& other) const { return cur_ == other.cur_; }
  void increment() {
    if (cur_ + 1 < max_idx_) {
      read_data_(cur_ + 1);  // read in advance
    }
    ++cur_;
  }
  reference dereference() const { return img_; }

  void read_data_(int idx) {
    if (idx < max_idx_ && idx > 0) {
      CPLErr err;
      switch (N) {
        case 1:
          err = dataset_->RasterIO(GF_Read, idx, 0, 1, n_lines_, img_.data, 1,
                                   n_lines_, gdal::DataType<T>::type(),
                                   n_bands_, nullptr, 0, 0, 0);
          break;
        case 2:
          err = dataset_->RasterIO(GF_Read, 0, idx, n_samples_, 1, img_.data,
                                   n_samples_, 1, gdal::DataType<T>::type(),
                                   n_bands_, nullptr, 0, 0, 0);
          break;
        default:
          err = dataset_->GetRasterBand(idx + 1)->RasterIO(
              GF_Read, 0, 0, n_samples_, n_lines_, img_.data, n_samples_,
              n_lines_, gdal::DataType<T>::type(), 0, 0);
      }
    } else {
      throw std::out_of_range("Input iterator out of range.");
    }
  }
};

/**
 * @brief 样本输入迭代器，逐个样本读取GDALDataset中的影像数据。
 *
 * @tparam T 读取影像的数据类型
 * @details 见LineInputIterator。
 *
 */
template <typename T>
using SampleInputIterator = InputIterator_<T, 1>;

/**
 * @brief 行输入迭代器，逐行读取GDALDataset中的影像数据。
 *
 * @tparam T 读取影像像元的数据类型
 * @details
 * 支持的操作包括
 * \code{.cpp}
 * hsp::LineInputIterator<float> it(src_dataset, 0), end(src_dataset);
 * cv::Mat val = *it;
 * ++it;
 * it++;
 * it != end;
 * \endcode
 */
template <typename T>
using LineInputIterator = InputIterator_<T, 2>;

/**
 * @brief 波段输入迭代器，逐波段读取GDALDataset中的影像数据。
 *
 * @tparam T 读取影像的数据类型
 * @details 见LineInputIterator。
 */
template <typename T>
using BandInputIterator = InputIterator_<T, 3>;

/**
 * @brief 输出迭代器，用于特化为其他迭代器。
 *
 * @tparam T 输出影像类型
 * @tparam N 特化迭代器类型，1为样本迭代器，2为波段迭代器，3为波段迭代器
 *
 *
 * @details
 * 输出迭代器OutputIterator_基于std::iterator，封装了对GDALDataset所拥有的高光谱数据立方资源通过RasterIO进行的写入操作。
 *
 * 迭代器的值类型为cv::Mat。
 *
 * 在实例化时，如果指定了当前的样本/行/波段号（均从0开始计数），那么实例化为普通的迭代器。如果不指定，那么该实例为末端迭代器，仅用于表示数据集的末尾。
 *
 * 本迭代器类通过指定类型，特化为SampleOutputIterator、LineOutputIterator和BandOutputIterator。
 */
template <typename T, unsigned N>
class OutputIterator_
    : public std::iterator<std::output_iterator_tag, void, void, void, void> {
 public:
  /**
   * @brief 初始化为末端迭代器。
   *
   * @param dataset 数据集指针
   */
  explicit OutputIterator_(GDALDataset* dataset) : dataset_{dataset} {
    init_();
    const int cur_table[] = {n_samples_, n_lines_, n_lines_};
    cur_ = cur_table[N - 1];
  }
  /**
   * @brief 初始化为普通输出迭代器。
   *
   * @param dataset 数据集指针
   * @param cur 当前位置，从0开始计数
   */
  OutputIterator_(GDALDataset* dataset, int cur)
      : dataset_{dataset}, cur_{cur} {
    init_();
  }

  /**
   * @brief 重载赋值符号。
   *
   * @param value
   * @return OutputIterator_& 返回*this
   *
   * @note 数据写入操作在此处进行。
   */
  OutputIterator_& operator=(const cv::Mat& value) {
    CPLErr err;
    switch (N) {
      case 1:
        err = dataset_->RasterIO(GF_Write, cur_, 0, 1, n_lines_, value.data, 1,
                                 n_lines_, gdal::DataType<T>::type(), n_bands_,
                                 nullptr, 0, 0, 0);
        break;
      case 2:
        err = dataset_->RasterIO(GF_Write, 0, cur_, n_samples_, 1, value.data,
                                 n_samples_, 1, gdal::DataType<T>::type(),
                                 n_bands_, nullptr, 0, 0, 0);
        break;
      default:
        err = dataset_->GetRasterBand(cur_ + 1)->RasterIO(
            GF_Write, 0, 0, n_samples_, n_lines_, value.data, n_samples_,
            n_lines_, gdal::DataType<T>::type(), 0, 0);
    }
    return *this;
  }

  /**
   * @brief 前缀自增。
   *
   * @return OutputIterator_&
   */
  OutputIterator_& operator++() {
    ++cur_;
    return *this;
  }
  /**
   * @brief 后缀自增。
   *
   * @return OutputIterator_
   */
  OutputIterator_ operator++(int) {
    OutputIterator_<T, N> old(*this);
    ++(*this);
    return old;
  }
  /**
   * @brief 迭代器比较。
   *
   * @param other
   * @return true
   * @return false
   */
  bool operator==(const OutputIterator_& other) const {
    return cur_ == other.cur_;
  }
  /**
   * @brief 迭代器比较。
   *
   * @param other
   * @return true
   * @return false
   */
  bool operator!=(const OutputIterator_& other) const {
    return !(*this == other);
  }
  /**
   * @brief 迭代器解引用。
   *
   * @return OutputIterator_& 返回*this
   */
  OutputIterator_& operator*() { return *this; }

 private:
  GDALDataset* dataset_;
  int n_samples_{0};
  int n_lines_{0};
  int n_bands_{0};
  int cur_{0};

 private:
  void init_() {
    if (dataset_) {
      n_samples_ = dataset_->GetRasterXSize();
      n_lines_ = dataset_->GetRasterYSize();
      n_bands_ = dataset_->GetRasterCount();
    } else {
      throw std::runtime_error("Initialize OutputIterator with nullptr!");
    }
  }
  constexpr cv::Size get_img_size_() {
    switch (N) {
      case 1:
        return cv::Size(n_lines_, n_bands_);
      case 2:
        return cv::Size(n_samples_, n_bands_);
      default:
        return cv::Size(n_samples_, n_lines_);
    }
  }
};

/**
 * @brief 样本输出迭代器，用于逐样本向GDALDataset输出（写入）影像。
 *
 * @tparam T 写入影像的像元数据类型
 */
template <typename T>
using SampleOutputIterator = OutputIterator_<T, 1>;

/**
 * @brief 行输出迭代器，用于逐行向GDALDataset输出（写入）影像。
 *
 * @tparam T 写入影像的像元数据类型
 */
template <typename T>
using LineOutputIterator = OutputIterator_<T, 2>;

/**
 * @brief 波段输出迭代器，用于逐波段向GDALDataset输出（写入）影像。
 *
 * @tparam T 写入影像的像元数据类型
 */
template <typename T>
using BandOutputIterator = OutputIterator_<T, 3>;

}  // namespace hsp

#endif  // HSP_ITERATOR_HPP_
