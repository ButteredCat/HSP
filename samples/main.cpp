#include <gdal.h>
#include <gdal_priv.h>

#include <boost/foreach.hpp>
#include <boost/iterator/function_input_iterator.hpp>
#include <boost/range/istream_range.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
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
  const std::string badpixel =
      "/home/xiaoyc/dataset/hsp_unittest/GF501A/coeff/SWIR/badpixel.tif";

  auto start = std::chrono::system_clock::now();
  hsp::DefectivePixelCorrection dpc;
  dpc.load(badpixel);
  cv::imwrite("/tmp/col_labeled.tif", dpc.get_col_label());
  cv::imwrite("/tmp/row_labeled.tif", dpc.get_row_label());

  auto end_time = system_clock::now();
  auto duration = duration_cast<microseconds>(end_time - start);
  std::cout << "Cost: "
            << double(duration.count()) * microseconds::period::num /
                   microseconds::period::den
            << "s" << std::endl;
  return 0;
}
