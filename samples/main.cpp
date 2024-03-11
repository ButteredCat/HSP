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
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>

// Boost
#include <boost/filesystem.hpp>
#include <boost/json/src.hpp>
#include <boost/program_options.hpp>

// OpenCV
#include <opencv2/imgcodecs.hpp>

// cmake-git-version-tracking
#include "git.h"

// spdlog
#include "spdlog/spdlog.h"

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
namespace po = boost::program_options;

/**
 * @brief 对高光谱影像数据辐射校正
 *
 * @param input
 * @param coeff
 * @param output
 */

void img_process(Input input, Coeff coeff, std::string output) {
  auto src_dataset = GDALDatasetUniquePtr(
      GDALDataset::FromHandle(GDALOpen(input.filename.c_str(), GA_ReadOnly)));
  int n_samples = src_dataset->GetRasterXSize();
  int n_lines = src_dataset->GetRasterYSize();
  int n_bands = src_dataset->GetRasterCount();

  auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
  auto dst_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
      poDriver->Create(output.c_str(), n_samples, n_lines, n_bands,
                       hsp::gdal::DataType<uint16_t>::type(), nullptr)));

  hsp::LineInputIterator<uint16_t> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  hsp::LineOutputIterator<uint16_t> obeg(dst_dataset.get(), 0);
  auto dbc = hsp::make_op<hsp::DarkBackgroundCorrection<uint16_t>>();
  dbc->load(coeff.dark_b);
  auto etalon = hsp::make_op<hsp::NonUniformityCorrection<double, double>>();
  etalon->load(coeff.etalon_a, coeff.etalon_b);
  auto nuc = hsp::make_op<hsp::NonUniformityCorrection<uint16_t, double>>();
  nuc->load(coeff.rel_a, coeff.rel_b);
  auto dpc = hsp::make_op<hsp::DefectivePixelCorrectionIDW>();
  dpc->load(coeff.badpixel);

  hsp::UnaryOpCombo ops;
  ops.add(dbc).add(etalon).add(nuc).add(dpc);
  std::transform(beg, end, obeg, ops);
}

/**
 * @brief 解析原始数据，并辐射校正
 *
 * @param input
 * @param coeff
 * @param output
 */
void raw_process(Input input, Coeff coeff, std::string output) {
  hsp::AHSIData L0_data(input.filename);
  L0_data.Traverse();

  auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
  char** papszOptions{nullptr};
  // papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BIL");
  if (!poDriver) {
    return;
  }
  auto dst_dataset =
      GDALDatasetUniquePtr(GDALDataset::FromHandle(poDriver->Create(
          output.c_str(), L0_data.samples(), L0_data.lines(), L0_data.bands(),
          hsp::gdal::DataType<uint16_t>::type(), papszOptions)));
  if (!dst_dataset) {
    return;
  }
  hsp::LineOutputIterator<uint16_t> output_it(dst_dataset.get(), 0);

  hsp::GF501A_DBC dbc;
  dbc.load(coeff.dark_a, coeff.dark_b);
  hsp::DefectivePixelCorrectionIDW dpc;
  dpc.load(coeff.badpixel);
  int i{0};
  for (auto&& frame : L0_data) {
    *output_it++ = dpc(dbc(frame));
    spdlog::debug("Frame {}", i++);
  }
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char* argv[]) {
  auto start = system_clock::now();
  //try {
    po::options_description generic("Generic options");
    generic.add_options()("version,v", "print version string")(
        "help", "produce help message")("config,c", po::value<std::string>(),
                                        "config file");

    po::options_description hidden("Hidden options");
    hidden.add_options()("input-file", po::value<std::vector<std::string>>(),
                         "input file");

    po::positional_options_description positional;
    positional.add("input-file", -1);

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                  .options(cmdline_options)
                  .positional(positional)
                  .run(),
              vm);
    if (vm.count("version")) {
      if (git_IsPopulated()) {
        printf("Branch: %s\nCommit: %s", git_Branch(), git_CommitSHA1());
        if (git_AnyUncommittedChanges()) {
          printf(" (has uncommited changes)\n");
        } else {
          printf("\n");
        }
      }
      printf("Build Date: %s, %s\n", __TIME__, __DATE__);
      return 0;
    }

    // spdlog settings
    // spdlog::set_pattern("[hsp] %+");
    spdlog::set_pattern("%v");
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Branch: {}", git_Branch());
    spdlog::info("Commit: {}", git_CommitSHA1());
    spdlog::info("Built on: {} {}", __TIME__, __DATE__);

    // GDAL init
    GDALAllRegister();

    std::vector<std::string> input_files;
    if (vm.count("input-file")) {
      input_files = vm["input-file"].as<decltype(input_files)>();
      for (auto&& each : input_files) {
        std::ifstream ifs(each);
        std::string input(std::istreambuf_iterator<char>(ifs), {});
        json::parse_options opt;
        opt.allow_comments = true;
        opt.allow_trailing_commas = true;
        Order order = json::value_to<Order>(json::parse(input, {}, opt));

//#pragma omp parallel for
        for (int i = 0; i < order.inputs.size(); ++i) {
          if (order.inputs[i].is_raw) {
            raw_process(order.inputs[i], order.coeff, order.outputs.at(i));
          } else {
            img_process(order.inputs[i], order.coeff, order.outputs.at(i));
          }
        }
      }
    }

  //} catch (const std::exception& e) {
  //  std::cerr << e.what();
  //}
  // 输出计算耗时
  auto end_time = system_clock::now();
  auto duration = duration_cast<microseconds>(end_time - start);
  std::cout << "Cost: "
            << double(duration.count()) * microseconds::period::num /
                   microseconds::period::den
            << "s" << std::endl;
}
