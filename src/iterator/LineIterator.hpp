// Copyright (C) 2023 Xiao Yunchen
#pragma once
// GDAL
#include <gdal.h>
#include <gdal_priv.h>

// C++ Standard

// Boost
#include <boost/iterator/iterator_facade.hpp>

// OpenCV
#include <opencv2/core.hpp>

// project
#include "../gdal_traits.hpp"
#include "../types.hpp"

namespace hsp {
namespace raster {

template <typename T>
class LineInputIterator
    : public boost::iterator_facade<LineInputIterator<T>, Image<T> const,
                                    boost::single_pass_traversal_tag> {
  friend boost::iterator_core_access;

 public:
  using reference = Image<T> const&;
  explicit LineInputIterator(GDALDataset* dataset) : dataset_{dataset} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      cur_ = n_lines_;
    }
  }
  LineInputIterator(GDALDataset* dataset, int cur)
      : dataset_{dataset}, cur_{cur} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      img_.data = boost::shared_ptr<T[]>(new T[n_samples_ * n_bands_]);
      img_.width = n_samples_;
      img_.height = n_bands_;
    }
  }

  size_t size() const { return n_lines_; }

 private:
  GDALDataset* dataset_;
  int cur_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  Image<T> img_;

 private:
  bool equal(LineInputIterator const& other) const {
    return this->cur_ == other.cur_;
  }
  void increment() {
    CPLErr err = dataset_->RasterIO(
        GF_Read, 0, cur_, n_samples_, 1, img_.data.get(), n_samples_, 1,
        gdal::DataType<T>::type(), n_bands_, nullptr, 0, 0, 0);
    ++cur_;
  }
  reference dereference() const { return img_; }
};

template <typename T>
class LineOutputIterator
    : public boost::iterator_facade<LineOutputIterator<T>, Image<T>,
                                    boost::single_pass_traversal_tag> {
  friend boost::iterator_core_access;

 public:
  using reference = Image<T>&;
  explicit LineOutputIterator(GDALDataset* dataset) : dataset_{dataset} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      cur_ = n_lines_;
    }
  }
  LineOutputIterator(GDALDataset* dataset, int cur)
      : dataset_{dataset}, cur_{cur} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      img_.data = boost::shared_ptr<T[]>(new T[n_samples_ * n_bands_]);
      img_.width = n_samples_;
      img_.height = n_bands_;
    }
  }

 private:
  GDALDataset* dataset_;
  int cur_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  Image<T> img_;

 private:
  bool equal(LineOutputIterator const& other) const {
    return this->cur_ == other.cur_;
  }
  void increment() { ++cur_; }
  reference dereference() const {
    CPLErr err = dataset_->RasterIO(
        GF_Write, 0, cur_, n_samples_, 1, img_.data.get(), n_samples_, 1,
        gdal::DataType<T>::type(), n_bands_, nullptr, 0, 0, 0);
    return const_cast<reference>(img_);
  }
};

}  // namespace raster
}  // namespace hsp
