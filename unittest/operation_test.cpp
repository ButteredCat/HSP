// Copyright (C) 2023 Xiao Yunchen

// GTest
#include <gtest/gtest.h>

// C++ Standard
#include <fstream>

// Boost
#include <boost/filesystem.hpp>

// OpenCV
#include <opencv2/imgcodecs.hpp>

// project
#include "../hsp/algorithm/cuda.hpp"
#include "../hsp/algorithm/radiometric.hpp"
#include "../hsp/iterator.hpp"

extern bool filecmp(const std::string&, const std::string&);

namespace fs = boost::filesystem;

class OperationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    GDALAllRegister();
    ASSERT_TRUE(fs::exists(src_file));
    ASSERT_TRUE(fs::exists(dark_coeff));
    ASSERT_TRUE(fs::exists(rel_a_coeff));
    ASSERT_TRUE(fs::exists(rel_b_coeff));
    src_dataset = GDALDatasetUniquePtr(
        GDALDataset::FromHandle(GDALOpen(src_file.c_str(), GA_Update)));
    ASSERT_NE(nullptr, src_dataset) << "Source dataset should be created.";

    n_samples = src_dataset->GetRasterXSize();
    n_lines = src_dataset->GetRasterYSize();
    n_bands = src_dataset->GetRasterCount();
    type = src_dataset->GetRasterBand(1)->GetRasterDataType();

    if (!fs::exists(work_dir)) {
      fs::create_directory(work_dir);
    }
  }

  void TearDown() override {
    if (fs::exists(work_dir)) {
      // fs::remove_all(work_dir);
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
  const fs::path testdata_dir = fs::path(std::getenv("HSP_UNITTEST"));
  const fs::path work_dir = fs::path("/tmp/hsp_unittest/");
  const fs::path src_file =
      testdata_dir / fs::path("/HGY/HGY_SWIR-20230429_110205-00000_out.dat");
  const fs::path dst_file = work_dir / src_file.filename();
  const fs::path coeff_path = testdata_dir / fs::path("/HGY/coeff/SWIR/");
  const fs::path dark_coeff = coeff_path / fs::path("dark.tif");
  const fs::path rel_a_coeff = coeff_path / fs::path("rel_a.tif");
  const fs::path rel_b_coeff = coeff_path / fs::path("rel_b.tif");
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
  dbc->load(dark_coeff.string());
  auto nuc = hsp::make_op<hsp::NonUniformityCorrection<uint16_t, float> >();
  nuc->load(rel_a_coeff.string(), rel_b_coeff.string());
  hsp::UnaryOpCombo ops;
  ops.add(dbc).add(nuc);
  std::transform(beg, end, obeg, ops);
  int j = 0;
  for (auto it = beg; it != end; ++it) {
    cv::Mat res = ops(*it);
    auto err =
        dst_dataset->RasterIO(GF_Write, 0, j++, n_samples, 1, res.data,
                              n_samples, 1, type, n_bands, nullptr, 0, 0, 0);
  }
  GDALClose(dst_dataset);
  EXPECT_FALSE(filecmp(src_file.string(), dst_file.string()));
}

TEST(DPCTest, FindConsecutive) {
  GDALAllRegister();
  const fs::path testdata_dir = fs::path(std::getenv("HSP_UNITTEST"));
  const fs::path work_dir = fs::path("/tmp/hsp_unittest/");
  const fs::path badpixel =
      testdata_dir / fs::path("/GF501A/coeff/SWIR/badpixel.tif");

  if (!fs::exists(work_dir)) {
    fs::create_directory(work_dir);
  }
  hsp::DefectivePixelCorrection dpc;
  dpc.load(badpixel.string());
  cv::imwrite((work_dir / fs::path("col_labeled.tif")).string(),
              dpc.get_col_label());
  cv::imwrite((work_dir / fs::path("row_labeled.tif")).string(),
              dpc.get_row_label());
}
