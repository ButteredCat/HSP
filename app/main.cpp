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

#include "../hsp/algorithm/cuda.hpp"
#include "../hsp/algorithm/radiometric.hpp"
#include "../hsp/core.hpp"

int main(void) {
  using namespace std::chrono;
  using DataType = uint16_t;

  GDALAllRegister();
  const std::string filename =
      "/home/xiaoyc/dataset/testdata/"
      "HGY_SWIR-20230429_110205-00000_outdark_mod.dat";
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
  std::cout << "samples: " << n_samples << "\nlines: " << n_lines
            << "\nbands: " << n_bands << "\n";
  const auto type = dataset->GetRasterBand(1)->GetRasterDataType();
  hsp::LineInputIterator<DataType> beg(dataset.get(), 0), end(dataset.get());

  auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
  if (!poDriver) {
    throw std::exception();
  }
  auto out_dataset =
      GDALDatasetUniquePtr(GDALDataset::FromHandle(poDriver->Create(
          out_raster.c_str(), n_samples, n_lines, n_bands, type, nullptr)));
  hsp::LineOutputIterator<DataType> obeg(out_dataset.get(), 0);

  auto dbc = hsp::make_op<hsp::cuda::DarkBackgroundCorrection<DataType> >();
  dbc->load(dark_coeff);
  auto nuc = hsp::make_op<hsp::cuda::NonUniformityCorrection<DataType, float> >();
  nuc->load(rel_a_coeff, rel_b_coeff);
  hsp::cuda::UnaryOpCombo ops;
  ops.add(dbc).add(nuc);
  // std::copy(beg, end, obeg);
  // std::transform(beg, end, obeg, ops);
  // *(++obeg);
  // int j = 0;
  for (auto it = beg; it != end; ++it) {
    *obeg++ = *it;
  }

  auto end_time = system_clock::now();
  auto duration = duration_cast<microseconds>(end_time - start);
  std::cout << "Cost: "
            << double(duration.count()) * microseconds::period::num /
                   microseconds::period::den
            << "s" << std::endl;
  return 0;
}
