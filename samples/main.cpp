/**
 * @file main.cpp
 * @author xiaoyc
 * @brief 测试文件。
 * @date 2023-11-28
 *
 * @copyright Copyright (c) 2023
 *
 */
// C++ Standard
#include <chrono>
#include <iostream>
#include <string>

// Boost
#include <boost/filesystem.hpp>

// OpenCV
#include <opencv2/imgcodecs.hpp>

// hsp
#include "../hsp/algorithm/AHSI_specific.hpp"
#include "../hsp/algorithm/radiometric.hpp"
#include "../hsp/core.hpp"
#include "../hsp/decoder/AHSIData.hpp"

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;

namespace fs = boost::filesystem;

const std::string filename =
    "/home/xiaoyc/dataset/hsp_unittest/GF501A/"
    "GF5A_AHSI_SW_20230722_354_621_L00000041058.DAT";
const std::string dstfile =
    "/home/xiaoyc/dataset/results/"
    "GF5A_AHSI_SW_20230722_354_621_L00000041058_subdark_IDW.tif";
const std::string dark_a =
    "/home/xiaoyc/dataset/hsp_unittest/GF501A/coeff/SWIR/dark_a.tif";
const std::string dark_b =
    "/home/xiaoyc/dataset/hsp_unittest/GF501A/coeff/SWIR/dark_b.tif";
const std::string badpixel =
    "/home/xiaoyc/dataset/hsp_unittest/GF501A/coeff/SWIR/badpixel.tif";

int main(int argc, char* argv[]) {
  using DataType = uint16_t;
  // try {
  GDALAllRegister();

  auto start = system_clock::now();

  // 建立待处理数据的行输入迭代器
  // auto src_dataset = GDALDatasetUniquePtr(
  //     GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_ReadOnly)));
  // if (!src_dataset) {
  //   return -1;
  // }
  // hsp::LineInputIterator<uint16_t> beg(src_dataset.get(), 0),
  //     end(src_dataset.get());

  hsp::AHSIData L0_data(filename);
  L0_data.Traverse();

  auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
  if (!poDriver) {
    return -1;
  }
  auto dst_dataset =
      GDALDatasetUniquePtr(GDALDataset::FromHandle(poDriver->Create(
          dstfile.c_str(), L0_data.samples(), L0_data.lines(), L0_data.bands(),
          hsp::gdal::DataType<uint16_t>::type(), nullptr)));
  if (!dst_dataset) {
    return -1;
  }
  hsp::LineOutputIterator<uint16_t> output_it(dst_dataset.get(), 0);

  hsp::GF501A_DBC dbc;
  dbc.load(dark_a, dark_b);
  hsp::DefectivePixelCorrectionIDW dpc;
  dpc.load(badpixel);

  for (auto&& frame : L0_data) {
    *output_it++ = dpc(dbc(frame));
  }

  // 输出计算耗时
  auto end_time = system_clock::now();
  auto duration = duration_cast<microseconds>(end_time - start);
  std::cout << "Cost: "
            << double(duration.count()) * microseconds::period::num /
                   microseconds::period::den
            << "s" << std::endl;
  // } catch (const std::exception& e) {
  //   std::cerr << e.what();
  // }
}
