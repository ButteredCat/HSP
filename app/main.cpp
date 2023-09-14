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
#include "../src/algorithm/radiometric.hpp"
#include "../src/iterator.hpp"

int main(void) {
  GDALAllRegister();
  std::string filename =
      "/home/xiaoyc/dataset/testdata/HGY_SWIR-20230429_110205-00000_out.dat";
  std::string out_raster = "/home/xiaoyc/dataset/testdata/out_raster.dat";
  std::string dark_coeff = "/home/xiaoyc/dataset/testdata/dark.tif";
  auto dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_Update)));
  hsp::LineInputIterator<uint16_t> beg(dataset.get(), 0), end(dataset.get());

  auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
  if (!poDriver) {
    throw std::exception();
  }
  auto out_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
      poDriver->CreateCopy(out_raster.c_str(), dataset.get(), false, 0, 0, 0)));
  hsp::LineOutputIterator<uint16_t> obeg(out_dataset.get(), 0);

  auto dbc = hsp::make_op<hsp::DarkBackgroundCorrection<uint16_t> >();
  dbc->load(dark_coeff);
  hsp::UnaryOpCombo ops;
  ops.add(dbc);
  //std::copy(beg, end, obeg);
  std::transform(beg, end, obeg, [](cv::Mat m) { 
    cv::Mat res = m * 2;
    return res; 
    });

  return 0;
}
