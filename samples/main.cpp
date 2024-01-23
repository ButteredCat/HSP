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
#include <boost/json/src.hpp>

// OpenCV
#include <opencv2/imgcodecs.hpp>

// hsp
#include "../hsp/algorithm/AHSI_specific.hpp"
#include "../hsp/algorithm/radiometric.hpp"
#include "../hsp/core.hpp"
#include "../hsp/decoder/AHSIData.hpp"
#include "./order_parser.hpp"

using parser::Coeff;
using parser::Input;
using parser::Order;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;

namespace fs = boost::filesystem;

void img_process(Input input, Coeff coeff, std::string output) {}

void raw_process(Input input, Coeff coeff, std::string output) {
  hsp::AHSIData L0_data(input.filename);
  L0_data.Traverse();

  auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
  if (!poDriver) {
    return;
  }
  auto dst_dataset =
      GDALDatasetUniquePtr(GDALDataset::FromHandle(poDriver->Create(
          output.c_str(), L0_data.samples(), L0_data.lines(), L0_data.bands(),
          hsp::gdal::DataType<uint16_t>::type(), nullptr)));
  if (!dst_dataset) {
    return;
  }
  hsp::LineOutputIterator<uint16_t> output_it(dst_dataset.get(), 0);

  hsp::GF501A_DBC dbc;
  dbc.load(coeff.dark_a, coeff.dark_b);
  hsp::DefectivePixelCorrectionIDW dpc;
  dpc.load(coeff.badpixel);

  for (auto&& frame : L0_data) {
    *output_it++ = dpc(dbc(frame));
  }
}

int main(int argc, char* argv[]) {
  // try {
  if (argc < 2) {
    std::cerr << "no input order\n";
    return -1;
  }
  auto start = system_clock::now();
  std::ifstream ifs(argv[1]);
  std::string input(std::istreambuf_iterator<char>(ifs), {});
  json::parse_options opt;
  opt.allow_comments = true;
  opt.allow_trailing_commas = true;
  Order order = json::value_to<Order>(json::parse(input, {}, opt));

  GDALAllRegister();

#pragma omp parallel for
  for (int i = 0; i < order.inputs.size(); ++i) {
    if (order.inputs[i].is_raw) {
      raw_process(order.inputs[i], order.coeff, order.outputs.at(i));
    } else {
      img_process(order.inputs[i], order.coeff, order.outputs.at(i));
    }
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
