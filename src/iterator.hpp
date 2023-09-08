// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_ITERATOR_HPP_
#define SRC_ITERATOR_HPP_

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

template <typename T, unsigned N>
class InputIterator_
    : public boost::iterator_facade<InputIterator_<T, N>, cv::Mat const,
                                    boost::single_pass_traversal_tag> {
  friend boost::iterator_core_access;

 public:
  using reference = cv::Mat const&;
  // end-of-file iterator
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
      ++cur_;
    }
  }

 private:
  GDALDataset* dataset_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  int cur_;
  int max_idx_;
  cv::Mat img_;

 private:
  bool equal(InputIterator_ const& other) const { return cur_ == other.cur_; }
  void increment() {
    read_data_(cur_);
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
    }
  }
};

template <typename T>
using SampleInputIterator = InputIterator_<T, 1>;

template <typename T>
using LineInputIterator = InputIterator_<T, 2>;

template <typename T>
using BandInputIterator = InputIterator_<T, 3>;

template <typename T, unsigned N>
class OutputIterator_
    : public boost::iterator_facade<OutputIterator_<T, N>, cv::Mat,
                                    std::output_iterator_tag> {
  friend boost::iterator_core_access;

 public:
  using reference = cv::Mat&;
  explicit OutputIterator_(GDALDataset* dataset) : dataset_{dataset} {
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
    }
  }
  OutputIterator_(GDALDataset* dataset, int cur)
      : dataset_{dataset}, cur_{cur} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      switch (N) {
        case 1:
          img_ = cv::Mat::zeros(cv::Size(n_lines_, n_bands_),
                                cv::DataType<T>::type);
          break;
        case 2:
          img_ = cv::Mat::zeros(cv::Size(n_samples_, n_bands_),
                                cv::DataType<T>::type);
          break;
        default:
          img_ = cv::Mat::zeros(cv::Size(n_samples_, n_lines_),
                                cv::DataType<T>::type);
      }
    }
  }

 private:
  GDALDataset* dataset_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  int cur_;
  cv::Mat img_;

 private:
  bool equal(OutputIterator_ const& other) const { return cur_ == other.cur_; }
  void increment() { ++cur_; }
  reference dereference() const {
    CPLErr err;
    switch (N) {
      case 1:
        err = dataset_->RasterIO(GF_Write, cur_, 0, 1, n_lines_, img_.data, 1,
                                 n_lines_, gdal::DataType<T>::type(), n_bands_,
                                 nullptr, 0, 0, 0);
        break;
      case 2:
        err = dataset_->RasterIO(GF_Write, 0, cur_, n_samples_, 1, img_.data,
                                 n_samples_, 1, gdal::DataType<T>::type(),
                                 n_bands_, nullptr, 0, 0, 0);
        break;
      default:
        err = dataset_->GetRasterBand(cur_ + 1)->RasterIO(
            GF_Write, 0, 0, n_samples_, n_lines_, img_.data, n_samples_,
            n_lines_, gdal::DataType<T>::type(), 0, 0);
    }
    return const_cast<reference>(img_);
  }
};

template <typename T>
using SampleOutputIterator = OutputIterator_<T, 1>;

template <typename T>
using LineOutputIterator = OutputIterator_<T, 2>;

// template <typename T>
// using BandOutputIterator = OutputIterator_<T, 3>;

template <typename T>
class BandOutputIterator
    : public std::iterator<std::output_iterator_tag, void, void, void, void> {
 public:
  explicit BandOutputIterator(GDALDataset* dataset) : dataset_{dataset} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      cur_ = n_bands_;
    }
  }
  BandOutputIterator(GDALDataset* dataset, int cur)
      : dataset_{dataset}, cur_{cur} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      img_ =
          cv::Mat::zeros(cv::Size(n_samples_, n_lines_), cv::DataType<T>::type);
    }
  }

  BandOutputIterator& operator=(const cv::Mat& value) {
    dataset_->GetRasterBand(cur_ + 1)->RasterIO(
        GF_Write, 0, 0, n_samples_, n_lines_, value.data, n_samples_, n_lines_,
        gdal::DataType<T>::type(), 0, 0);
    return *this;
  }
  BandOutputIterator& operator++() {
    ++cur_;
    return *this;
  }
  BandOutputIterator operator++(int) {
    BandOutputIterator<T> old(*this);
    ++(*this);
    return old;
  }
  bool operator==(const BandOutputIterator& other) const {
    return cur_ == other.cur_;
  }
  bool operator!=(const BandOutputIterator& other) const {
    return !(*this == other);
  }
  cv::Mat& operator*() { return img_; }

 private:
  GDALDataset* dataset_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  int cur_;
  cv::Mat img_;
};

}  // namespace hsp

#endif  // SRC_ITERATOR_HPP_
