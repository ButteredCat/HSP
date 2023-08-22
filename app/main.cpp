#include <gdal.h>
#include <gdal_priv.h>

#include <boost/foreach.hpp>
#include <boost/iterator/function_input_iterator.hpp>
#include <boost/range/istream_range.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "iterator/LineIterator.hpp"

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
  dataset->GetRasterBand(1)->GetRasterDataType();
  hsp::raster::LineInputIterator<float> beg(dataset.get(), 0),
      end(dataset.get());

  auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
  if (!poDriver) {
    throw std::exception();
  }
  auto out_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
      poDriver->CreateCopy(out_raster.c_str(), dataset.get(), FALSE, 0, 0, 0)));
  hsp::raster::LineOutputIterator<float> obeg(out_dataset.get(), 0),
      oend(out_dataset.get());

  //std::copy(beg, end, obeg);
  std::transform(beg, end, obeg, [](const hsp::Image<float>& im) {return im;});
  return 0;
}
