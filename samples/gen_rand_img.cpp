
// GDAL
#include <gdal.h>
#include <gdal_priv.h>

// C++
#include <exception>
#include <string>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "../src/gdal_traits.hpp"

int main(void) {
  using this_type = double;
  const std::string filename = "/home/xiaoyc/dataset/testdata/rand_a.tif";
  const int n_samples = 1999;
  const int n_lines = 76;
  const int n_bands = 1;
  GDALAllRegister();
  cv::RNG rng;
  cv::Mat img(cv::Size(n_samples, n_lines), cv::DataType<this_type>::type);
  rng.fill(img, cv::RNG::UNIFORM, 0.f, 1.f);
  cv::imwrite(filename, img);

  //   auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
  //   if (!poDriver) {
  //     throw std::runtime_error("nullptr poDriver");
  //   }
  //   auto dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
  //       poDriver->Create(filename.c_str(), n_samples, n_lines, n_bands,
  //                        hsp::gdal::DataType<this_type>::type(), nullptr)));
  //   if (!dataset) {
  //     throw std::runtime_error("nullptr dataset");
  //   }
  //   auto err = dataset->GetRasterBand(1)->RasterIO(
  //       GF_Write, 0, 0, n_samples, n_lines, img.data, n_samples, n_lines,
  //       hsp::gdal::DataType<this_type>::type(), n_bands, 0, 0);
  return 0;
}
