#pragma once
#include <gdal.h>
#include <gdal_priv.h>

#include <boost/iterator/iterator_facade.hpp>
#include <exception>
#include <opencv2/core.hpp>

namespace hsp {
namespace raster {
class LineIterator
    : public boost::iterator_facade<LineIterator, cv::Mat,
                                    boost::single_pass_traversal_tag> {
  friend boost::iterator_core_access;

 public:
  LineIterator(GDALDataset* dataset, int cur_line = 0)
      : dataset_{dataset}, cur_line_{cur_line} {
    if (!dataset) {
      throw std::exception();
    }
    n_samples_ = dataset->GetRasterXSize();
    n_lines_ = dataset->GetRasterYSize();
    n_bands_ = dataset->GetRasterCount();
  }
  size_t size() const {
    return n_lines_;
  }

 private:
  GDALDataset* dataset_;
  int n_samples_;
  int n_lines_;
  int n_bands_;
  int cur_line_;
  cv::Mat img_;

 private:
  bool equal(LineIterator const& other) {}
  void increment() {}
  reference dereference() const {

  }
  reference dereference() {

  }
};
}  // namespace raster
}  // namespace hsp
