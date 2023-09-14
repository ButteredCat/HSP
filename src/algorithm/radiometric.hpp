// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_ALGORITHM_RADIOMETRIC_HPP_
#define SRC_ALGORITHM_RADIOMETRIC_HPP_

// C++ Standard
#include <fstream>

// project
#include "../gdal_traits.hpp"
#include "../gdalex.hpp"
#include "./operation.hpp"

namespace hsp {

template <typename T>
bool load(const char* filepath, size_t count, T* data) {
  if (IsRasterDataset(filepath)) {
    GDALDataset* dataset =
        reinterpret_cast<GDALDataset*>(GDALOpen(filepath, GA_ReadOnly));
    if (static_cast<size_t>(dataset->GetRasterXSize()) *
            dataset->GetRasterYSize() <
        count) {
      GDALClose(dataset);
      return false;
    }
    int cols = dataset->GetRasterXSize();
    int rows = count / cols;
    if (dataset->RasterIO(GF_Read, 0, 0, cols, rows, data, cols, rows,
                          gdal::DataType<T>::type(), 1, nullptr, 0, 0, 0)) {
    }
    GDALClose(dataset);
    return true;
  }
  FILE* fp = fopen(filepath, "r");
  if (fp == nullptr) {
    return false;
  }
  size_t i;
  for (i = 0; i < count; ++i) {
    double t;
    if (fscanf(fp, "%lf%*c", &t) != 1) break;
    data[i] = (T)t;
  }

  fclose(fp);
  return i == count;
}

template <typename T>
cv::Mat load_raster(const std::string& filename) {
  auto dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_ReadOnly)));
  if (!dataset) {
    throw std::runtime_error("unable to open dataset");
  }
  const int n_samples = dataset->GetRasterXSize();
  const int n_lines = dataset->GetRasterYSize();
  cv::Mat res =
      cv::Mat::zeros(cv::Size(n_samples, n_lines), cv::DataType<T>::type);
  CPLErr err = dataset->GetRasterBand(1)->RasterIO(
      GF_Read, 0, 0, n_samples, n_lines, res.data, n_samples, n_lines,
      gdal::DataType<T>::type(), 0, 0);
  return res;
}

template <typename T>
cv::Mat load_text(const std::string& filename) {
  std::ifstream in(filename);
  std::vector<T> data_vec;
  int n_lines{0}, count{0};
  for (std::string line; std::getline(in, line);) {
    std::vector<T> data_line;
    std::istringstream input(line);
    for (T data; input >> data;) {
      data_vec.push_back(data);
      ++count;
    }
    ++n_lines;
  }
  return cv::Mat(cv::Size(count / n_lines, n_lines), cv::DataType<T>::type,
                 data_vec.data())
      .clone();
}

template <typename T>
class DarkBackgroundCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override { return m - m_; }

  void load(const std::string& filename) {
    if (IsRasterDataset(filename.c_str())) {
      m_ = load_raster<T>(filename.c_str());
    } else {
      m_ = load_text<T>(filename.c_str());
    }
  }

 private:
  cv::Mat m_;
};

template <typename T_out, typename T_coeff = float>
class NonUniformityCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::Mat res;
    m.convertTo(m, cv::DataType<T_coeff>::type);
    m = m.mul(a_) + b_;
    m.convertTo(res, cv::DataType<T_out>::type);
    return res;
  }
  void load(const std::string& coeff_a, const std::string& coeff_b) {
    if (IsRasterDataset(coeff_a.c_str())) {
      a_ = load_raster<T_coeff>(coeff_a.c_str());
    } else {
      a_ = load_raster<T_coeff>(coeff_a.c_str());
    }
    if (IsRasterDataset(coeff_b.c_str())) {
      b_ = load_raster<T_coeff>(coeff_b.c_str());
    } else {
      b_ = load_raster<T_coeff>(coeff_b.c_str());
    }
  }

 private:
  cv::Mat a_;
  cv::Mat b_;
};

template <typename T_out = float, typename T_coeff = float>
class AbsoluteRadiometricCorrection : public UnaryOperation {
 public:
  cv::Mat operator()(cv::Mat m) override {
    cv::Mat res;
    m.convertTo(m, cv::DataType<T_coeff>::type);
    // m = m.mul(a_) + b_;
    m.convertTo(res, cv::DataType<T_out>::type);
    return res;
  }
  void load(const std::string& filename) {}

 private:
  cv::Mat a_;
  cv::Mat b_;
};

}  // namespace hsp

#endif  // SRC_ALGORITHM_RADIOMETRIC_HPP_
