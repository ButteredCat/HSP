// Copyright (c) 2023 Xiao Yunchen
#ifndef SRC_ITERATOR_BANDITERATOR_HPP_
#define SRC_ITERATOR_BANDITERATOR_HPP_

// GDAL
#include <gdal.h>

// Boost
#include <boost/iterator/iterator_facade.hpp>

// OpenCV
#include <opencv2/core.hpp>

// project
#include "../types.hpp"
#include "../gdal_traits.hpp"

namespace hsp {
namespace raster {

int gdal2cv(GDALDataType type) {
  int res = 0;
  switch (type) {
    case GDT_Byte:
      res = CV_MAKETYPE(CV_8U, 1);
      break;
    case GDT_UInt16:
      res = CV_MAKETYPE(CV_16U, 1);
      break;
    case GDT_Int16:
      res = CV_MAKETYPE(CV_16S, 1);
      break;
    case GDT_Float32:
      res = CV_MAKETYPE(CV_32F, 1);
      break;
  }
  return res;
}

template <typename T>
class BandInputIterator
    : public boost::iterator_facade<BandInputIterator<T>, cv::Mat const,
                                    boost::single_pass_traversal_tag> {
  friend boost::iterator_core_access;

 public:
  using reference = cv::Mat const&;
  // end-of-file iterator
  explicit BandInputIterator(GDALDataset* dataset) : dataset_{dataset} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      cur_ = n_bands_;
    }
  }
  BandInputIterator(GDALDataset* dataset, int cur)
      : dataset_{dataset}, cur_{cur} {
    if (dataset) {
      n_samples_ = dataset->GetRasterXSize();
      n_lines_ = dataset->GetRasterYSize();
      n_bands_ = dataset->GetRasterCount();
      type_ = dataset->GetRasterBand(1)->GetRasterDataType();
      img_ = cv::Mat_<T>(n_lines_, n_samples_);
    }
  }

 private:
  GDALDataset* dataset_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  int cur_;
  cv::Mat img_;
  GDALDataType type_;

 private:
  bool equal(BandInputIterator const& other) {
    return this->cur_ == other.cur_;
  }
  void increment() {
    CPLErr err = dataset_->GetRasterBand(cur_)->RasterIO(
        GF_Read, 0, 0, n_samples_, n_lines_, img_.data, n_samples_, n_lines_,
        gdal::DataType<T>::type(), 0, 0);
    ++cur_;
  }
  reference dereference() const { return img_; }
};
}  // namespace raster
}  // namespace hsp

#endif  // SRC_ITERATOR_BANDITERATOR_HPP_
