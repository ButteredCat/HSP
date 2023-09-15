#include <gdal.h>
#include <gdal_priv.h>

#include <boost/foreach.hpp>
#include <boost/iterator/function_input_iterator.hpp>
#include <boost/range/istream_range.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// #include "../src/iterator/BandIterator.hpp"
// #include "../src/iterator/LineIterator.hpp"
#include "../src/algorithm/cuda.hpp"
#include "../src/algorithm/radiometric.hpp"
#include "../src/iterator.hpp"

int main(void) {
  using namespace std::chrono;

  GDALAllRegister();
  const std::string filename =
      "/home/xiaoyc/dataset/testdata/HGY_SWIR-20230429_110205-00000_out.dat";
  const std::string out_raster = "/home/xiaoyc/dataset/testdata/out_raster.dat";
  const std::string dark_coeff = "/home/xiaoyc/dataset/testdata/dark.tif";
  const std::string rel_a_coeff = "/home/xiaoyc/dataset/testdata/rel_a.tif";
  const std::string rel_b_coeff = "/home/xiaoyc/dataset/testdata/rel_b.tif";

  auto start = std::chrono::system_clock::now();
  auto dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_Update)));

  const int n_samples = dataset->GetRasterXSize();
  const int n_lines = dataset->GetRasterYSize();
  const int n_bands = dataset->GetRasterCount();
  const auto type = dataset->GetRasterBand(1)->GetRasterDataType();
  hsp::LineInputIterator<uint16_t> beg(dataset.get(), 0), end(dataset.get());

  auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
  if (!poDriver) {
    throw std::exception();
  }
  auto out_dataset =
      GDALDatasetUniquePtr(GDALDataset::FromHandle(poDriver->Create(
          out_raster.c_str(), n_samples, n_lines, n_bands, type, nullptr)));
  hsp::LineOutputIterator<uint16_t> obeg(out_dataset.get(), 0);

  auto dbc = hsp::make_op<hsp::cuda::DarkBackgroundCorrection<uint16_t> >();
  dbc->load(dark_coeff);
  auto nuc = hsp::make_op<hsp::NonUniformityCorrection<uint16_t, float> >();
  nuc->load(rel_a_coeff, rel_b_coeff);
  hsp::UnaryOpCombo ops;
  ops.add(dbc).add(nuc);
  std::transform(beg, end, obeg, ops);
  *(++obeg);
  // int j = 0;
  // for (auto it = beg; it != end; ++it) {
  //   cv::Mat res = ops(*it);
  //   auto err =
  //       out_dataset->RasterIO(GF_Write, 0, j++, n_samples, 1, res.data,
  //                             n_samples, 1, type, n_bands, nullptr, 0, 0, 0);
  // }

  auto end_time = system_clock::now();
  auto duration = duration_cast<microseconds>(end_time - start);
  std::cout << "Cost: "
            << double(duration.count()) * microseconds::period::num /
                   microseconds::period::den
            << "s" << std::endl;
  return 0;
}
