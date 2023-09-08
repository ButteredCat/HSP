#include <gdal.h>
#include <gdal_priv.h>

#include <boost/foreach.hpp>
#include <boost/iterator/function_input_iterator.hpp>
#include <boost/range/istream_range.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// #include "../src/iterator/BandIterator.hpp"
// #include "../src/iterator/LineIterator.hpp"
#include "../src/iterator.hpp"

int main(void) {
  GDALAllRegister();
  std::string filename =
      "/home/xiaoyc/dataset/HGY/"
      "HGY_SWIR-20230429_110205-00000_outdark_mod_ref.dat";
  std::string raw_file =
      "/home/xiaoyc/dataset/HGY/nir/Goldeye-20230103_142007-00000.dat";
  std::string out_file = "/home/xiaoyc/dataset/HGY/nir/out.dat";
  std::string out_raster = "/home/xiaoyc/dataset/HGY/out_raster.dat";
  auto dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_Update)));
  // hsp::BandInputIterator<float> beg(dataset.get(), 0), end(dataset.get());

  auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
  if (!poDriver) {
    throw std::exception();
  }
  auto out_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
      poDriver->CreateCopy(out_raster.c_str(), dataset.get(), false, 0, 0, 0)));
  hsp::BandOutputIterator<float> obeg(out_dataset.get(), 0);

  const int n_samples = dataset->GetRasterXSize();
  const int n_lines = dataset->GetRasterYSize();
  const auto type = dataset->GetRasterBand(1)->GetRasterDataType();
  const int buffer_size = n_samples * n_lines * GDALGetDataTypeSize(type);
  cv::Mat img =
      cv::Mat::zeros(cv::Size(n_samples, n_lines), cv::DataType<float>::type);

  for (auto&& band : dataset->GetBands()) {
    auto err = band->RasterIO(GF_Read, 0, 0, n_samples, n_lines, img.data,
                              n_samples, n_lines, type, 0, 0);
    *obeg = img;
    ++obeg;
  }
  // std::copy(beg, end, obeg);
  //     std::transform(beg, end, obeg,
  //                    [](const hsp::Image<float>& im) { return im; });
  return 0;
}
