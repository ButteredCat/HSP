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

// OpenCV
#include <opencv2/imgcodecs.hpp>

// hsp
#include "../hsp/algorithm/cuda.hpp"
#include "../hsp/algorithm/radiometric.hpp"
#include "../hsp/core.hpp"

int main(void) {
  using namespace std::chrono;  // NO LINT
  using DataType = uint16_t;

  GDALAllRegister();
  const std::string filename =
      "/home/xiaoyc/dataset/GF501A/"
      "GF5A_AHSI_VN_20230722_354_621_L00000041058.tif";
  const std::string dstfile = "/tmp/out.dat";
  const std::string badpixel =
      "/home/xiaoyc/dataset/hsp_unittest/GF501A/coeff/SWIR/badpixel.tif";

  auto start = system_clock::now();

  // 建立待处理数据的行输入迭代器
  auto src_dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(filename.c_str(), GA_ReadOnly)));
  if (!src_dataset) {
    return -1;
  }
  hsp::LineInputIterator<uint16_t> beg(src_dataset.get(), 0),
      end(src_dataset.get());

  // 建立处理后数据的行输出迭代器
  auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
  if (!poDriver) {
    return -1;
  }
  auto dst_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
      poDriver->CreateCopy(dstfile.c_str(), src_dataset.get(), FALSE, nullptr,
                           nullptr, nullptr)));
  if (!dst_dataset) {
    return -1;
  }
  hsp::LineOutputIterator<uint16_t> output_it(dst_dataset.get(), 0);

  // 新建盲元修复算法对象，并载入tiff格式盲元列表
  hsp::DefectivePixelCorrection dpc;
  dpc.load(badpixel);
  // cv::imwrite("/tmp/col_labeled.tif", dpc.get_col_label());
  // cv::imwrite("/tmp/row_labeled.tif", dpc.get_row_label());
  // 对每行数据进行盲元修复
  for (auto line = beg; line != end; ++line) {
    *output_it++ = dpc(*line);
  }

  // 输出计算耗时
  auto end_time = system_clock::now();
  auto duration = duration_cast<microseconds>(end_time - start);
  std::cout << "Cost: "
            << double(duration.count()) * microseconds::period::num /
                   microseconds::period::den
            << "s" << std::endl;
  return 0;
}
