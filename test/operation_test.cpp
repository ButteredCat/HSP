// Copyright (C) 2023 Xiao Yunchen

// GTest
#include <gtest/gtest.h>

// C++ Standard
#include <fstream>

// Boost
#include <boost/filesystem.hpp>

// project
#include "../src/algorithm/cuda.hpp"
#include "../src/algorithm/radiometric.hpp"
#include "../src/iterator.hpp"

extern bool filecmp(const std::string&, const std::string&);

namespace fs = boost::filesystem;

class OperationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    GDALAllRegister();
    ASSERT_TRUE(fs::exists(src_file));
    src_dataset = GDALDatasetUniquePtr(
        GDALDataset::FromHandle(GDALOpen(src_file.c_str(), GA_Update)));
    ASSERT_NE(nullptr, src_dataset) << "Source dataset should be created.";

    n_samples = src_dataset->GetRasterXSize();
    n_lines = src_dataset->GetRasterYSize();
    n_bands = src_dataset->GetRasterCount();
    type = src_dataset->GetRasterBand(1)->GetRasterDataType();
  }

  void TearDown() override {
    if (fs::exists(dst_file)) {
      // fs::remove(dst_file);
    }
  }

  void CreateDst() {
    auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
    ASSERT_NE(nullptr, poDriver);

    if (fs::exists(dst_file)) {
      fs::remove(dst_file);
    }
    dst_dataset = poDriver->CreateCopy(dst_file.c_str(), src_dataset.get(),
                                       false, 0, 0, 0);
    ASSERT_NE(nullptr, dst_dataset);
  }

 protected:
  GDALDatasetUniquePtr src_dataset{nullptr};
  GDALDataset* dst_dataset{nullptr};
  const std::string src_file =
      "/home/xiaoyc/dataset/testdata/HGY_SWIR-20230429_110205-00000_out.dat";
  const std::string dst_file =
      "/home/xiaoyc/dataset/testdata/operation_test_out.dat";
  const std::string dark_coeff = "/home/xiaoyc/dataset/testdata/dark.tif";
  const std::string rel_a_coeff = "/home/xiaoyc/dataset/testdata/rel_a.tif";
  const std::string rel_b_coeff = "/home/xiaoyc/dataset/testdata/rel_b.tif";
  int n_samples = 0;
  int n_lines = 0;
  int n_bands = 0;
  GDALDataType type;
};

TEST_F(OperationTest, DarkBackgroundCorrection) {
  hsp::LineInputIterator<uint16_t> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  CreateDst();
  hsp::LineOutputIterator<uint16_t> obeg(dst_dataset, 0);
  auto dbc = hsp::make_op<hsp::DarkBackgroundCorrection<uint16_t> >();
  dbc->load(dark_coeff);
  auto nuc = hsp::make_op<hsp::NonUniformityCorrection<uint16_t, float> >();
  nuc->load(rel_a_coeff, rel_b_coeff);
  hsp::UnaryOpCombo ops;
  ops.add(dbc).add(nuc);
  // std::transform(beg, end, obeg, ops);
  int j = 0;
  for (auto it = beg; it != end; ++it) {
    cv::Mat res = ops(*it);
    auto err =
        dst_dataset->RasterIO(GF_Write, 0, j++, n_samples, 1, res.data, n_samples,
                              1, type, n_bands, nullptr, 0, 0, 0);
  }
  GDALClose(dst_dataset);
  EXPECT_FALSE(filecmp(src_file, dst_file));
}
