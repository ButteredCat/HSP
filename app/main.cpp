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
      "/home/xiaoyc/dataset/HGY/HGY_SWIR-20230429_110205-00000_outdark.dat";
  std::string raw_file =
      "/home/xiaoyc/dataset/HGY/nir/Goldeye-20230103_142007-00000.dat";
  std::string out_file = "/home/xiaoyc/dataset/HGY/nir/out.dat";
  auto dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_Update)));
  hsp::raster::LineInputIterator<uint16_t> beg(dataset.get(), 0), end(dataset.get());
  for(auto it = beg; it != end; ++it) {
    auto line = *it;
  }

  return 0;
}
