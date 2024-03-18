/**
 * @file cuda_test.cpp
 * @author xiaoyc
 * @brief CUDA 算法测试用例。
 * @version 0.1
 * @date 2023-11-09
 *
 * @copyright Copyright (c) 2023
 *
 */
// GDAL
#include <gdal.h>
#include <gdal_priv.h>

// GTest
#include <gtest/gtest.h>

// C++ Standard

// Boost
#include <boost/filesystem.hpp>

// project
#include "../hsp/algorithm/AHSI_specific.hpp"
#include "../hsp/algorithm/cuda.hpp"
#include "../hsp/algorithm/radiometric.hpp"
#include "../hsp/core.hpp"
#include "../hsp/decoder/AHSIData.hpp"

namespace fs = boost::filesystem;
using hsp::AHSIData;
using hsp::AHSIFrame;

class GF501AVNIRTest : public ::testing::Test {
 protected:
  void SetUp() override {
    GDALAllRegister();
    L0_data.Traverse();

    if (!fs::exists(work_dir)) {
      fs::create_directory(work_dir);
    }

    auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    ASSERT_NE(nullptr, poDriver);
    papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
    dataset = GDALDataset::FromHandle(poDriver->Create(
        dst_file.string().c_str(), L0_data.samples(), L0_data.lines(),
        L0_data.bands(), GDT_UInt16, papszOptions));
    ASSERT_NE(nullptr, dataset);
  }

  void TearDown() override {
    GDALClose(dataset);
    CSLDestroy(papszOptions);
    if (fs::exists(work_dir)) {
      // fs::remove_all(work_dir);
    }
  }

 protected:
  GDALDataset* dataset{nullptr};
  char** papszOptions{nullptr};
  const fs::path testdata_dir = fs::path(std::getenv("HSP_UNITTEST"));
  const fs::path coeff_path = testdata_dir / fs::path("/GF501A/coeff/VNIR/");
  const fs::path work_dir = fs::path("/tmp/hsp_unittest/");
  const fs::path src_file =
      testdata_dir /
      fs::path("/GF501A/GF5A_AHSI_VN_20230722_354_621_L00000041058.DAT");
  const fs::path dst_file =
      work_dir / src_file.filename().replace_extension("tif");
  AHSIData L0_data{src_file.string()};

  const fs::path dark_a = coeff_path / fs::path("/dark_a.tif");
  const fs::path dark_b = coeff_path / fs::path("/dark_b.tif");
  const fs::path etalon_a = coeff_path / fs::path("/etalon_a.tif");
  const fs::path etalon_b = coeff_path / fs::path("/etalon_b.tif");
  const fs::path rel_a = coeff_path / fs::path("/rel_a.tif");
  const fs::path rel_b = coeff_path / fs::path("/rel_b.tif");
};

TEST_F(GF501AVNIRTest, Traverse) {
  ASSERT_EQ(L0_data.sensor_type(), AHSIData::SensorType::VNIR);
  ASSERT_EQ(L0_data.samples(), 2048);
  ASSERT_EQ(L0_data.lines(), 2412);
  ASSERT_EQ(L0_data.bands(), 150);
}

TEST_F(GF501AVNIRTest, IteratorIncrementAndCompare) {
  AHSIData::FrameIterator it(&L0_data, 0);
  auto beg = L0_data.begin();
  ASSERT_EQ(it, beg);
  it++;
  ASSERT_NE(it, beg);
  ++beg;
  ASSERT_EQ(it, beg);
}

TEST_F(GF501AVNIRTest, IteratorOffsetDereference) {
  AHSIData::FrameIterator it(&L0_data, 0);
  auto frame = it[5];
  SUCCEED();
}

TEST_F(GF501AVNIRTest, L0Decoding) {
  hsp::LineOutputIterator<uint16_t> it(dataset, 0);
  std::transform(L0_data.begin(), L0_data.end(), it,
                 [](const AHSIFrame& frame) { return frame.data; });
  SUCCEED();
}

TEST_F(GF501AVNIRTest, VNIRProcessing) {
  hsp::UnaryOpCombo ops;
  hsp::GF501A_DBC dbc;
  dbc.load(dark_a.string(), dark_b.string());
  auto etalon = hsp::make_op<hsp::NonUniformityCorrection<double, double>>();
  etalon->load(etalon_a.string(), etalon_b.string());
  auto nuc = hsp::make_op<hsp::NonUniformityCorrection<uint16_t, double>>();
  nuc->load(rel_a.string(), rel_b.string());
  ops.add(etalon).add(nuc);
  hsp::LineOutputIterator<uint16_t> it(dataset, 0);
  for (auto&& frame : L0_data) {
    *it++ = ops(dbc(frame));
  }
  SUCCEED();
}

TEST(GF501ATest, Exception) {
  const fs::path testdata_dir = fs::path(std::getenv("HSP_UNITTEST"));
  const fs::path src_file =
      testdata_dir /
      fs::path("/GF501A/GF5A_AHSI_VN_20230722_354_621_L00000041058.DAT");

  hsp::AHSIData data(src_file.string());
  auto it = data.begin();
  EXPECT_THROW(*it, std::runtime_error);
}

class GF501ASWIRTest : public ::testing::Test {
 protected:
  void SetUp() override {
    GDALAllRegister();
    L0_data.Traverse();

    if (!fs::exists(work_dir)) {
      fs::create_directory(work_dir);
    }

    auto poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    ASSERT_NE(nullptr, poDriver);
    papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
    dataset = GDALDataset::FromHandle(poDriver->Create(
        dst_file.string().c_str(), L0_data.samples(), L0_data.lines(),
        L0_data.bands(), GDT_UInt16, papszOptions));
    ASSERT_NE(nullptr, dataset);
  }

  void TearDown() override {
    GDALClose(dataset);
    CSLDestroy(papszOptions);
    if (fs::exists(work_dir)) {
      // fs::remove_all(work_dir);
    }
  }

 protected:
  GDALDataset* dataset{nullptr};
  char** papszOptions{nullptr};
  const fs::path testdata_dir = fs::path(std::getenv("HSP_UNITTEST"));
  const fs::path coeff_path = testdata_dir / fs::path("/GF501A/coeff/SWIR/");
  const fs::path work_dir = fs::path("/tmp/hsp_unittest/");
  const fs::path src_file =
      testdata_dir /
      fs::path("/GF501A/GF5A_AHSI_SW_20230708_353_625_L00000038437.DAT");
  const fs::path dst_file =
      work_dir / src_file.filename().replace_extension("tif");
  const fs::path dpc_baseline =
      testdata_dir /
      fs::path(
          "/GF501A/GF5A_AHSI_SW_20230708_353_625_L00000038437_DBC_IDW.dat");
  AHSIData L0_data{src_file.string()};

  const fs::path dark_a = coeff_path / fs::path("/dark_a.tif");
  const fs::path dark_b = coeff_path / fs::path("/dark_b.tif");
  const fs::path etalon_a = coeff_path / fs::path("/etalon_a.tif");
  const fs::path etalon_b = coeff_path / fs::path("/etalon_b.tif");
  const fs::path rel_a = coeff_path / fs::path("/rel_a.tif");
  const fs::path rel_b = coeff_path / fs::path("/rel_b.tif");
  const fs::path badpixel = coeff_path / fs::path("/badpixel.tif");
};

TEST_F(GF501ASWIRTest, DefectivePixelCorrectionSpectral) {
  hsp::GF501A_DBC dbc;
  dbc.load(dark_a.string(), dark_b.string());
  hsp::DefectivePixelCorrectionSpectral dpc;
  dpc.set_inpaint(hsp::Inpaint::NEIGHBORHOOD_AVERAGING);
  dpc.load(badpixel.string());

  hsp::LineOutputIterator<uint16_t> it(dataset, 0);
  for (auto&& frame : L0_data) {
    *it++ = dpc(dbc(frame));
  }
}

TEST_F(GF501ASWIRTest, DISABLED_DefectivePixelCorrectionIDW) {
  hsp::GF501A_DBC dbc;
  dbc.load(dark_a.string(), dark_b.string());
  hsp::DefectivePixelCorrectionIDW dpc_idw;
  dpc_idw.load(badpixel.string());
  cv::Mat dpm = dpc_idw.get_col_label().clone();
  dpm.setTo(cv::Scalar::all(1), dpm != 0);
  auto baseline_dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(
      GDALOpen(dpc_baseline.string().c_str(), GA_ReadOnly)));
  hsp::LineInputIterator<uint16_t> baseline_beg(baseline_dataset.get(), 0);
  for (auto&& frame : L0_data) {
    cv::Mat m1 = dpc_idw(dbc(frame));//.getUMat(cv::AccessFlag::ACCESS_READ);
    cv::Mat m2 = (*baseline_beg++);//.getUMat(cv::AccessFlag::ACCESS_READ);
    cv::Mat diff, masked;
    cv::absdiff(m1, m2, diff);
    cv::multiply(diff, dpm, masked);
    cv::Mat masked_f, m2_f, ratio;
    masked.convertTo(masked_f, CV_32F);
    m2.convertTo(m2_f, CV_32F);
    cv::divide(masked_f, m2_f, ratio);
    double min_val{0.}, max_val{0.};
    cv::minMaxLoc(ratio, &min_val, &max_val);
    // ASSERT_TRUE(max_val < 0.2);
  }
}

#ifdef HAVE_CUDA
TEST_F(GF501AVNIRTest, VNIRProcessingCUDAv1) {
  hsp::cuda::UnaryOpCombo ops;
  hsp::cuda::GF501A_DBC dbc;
  dbc.load(dark_a.string(), dark_b.string());
  auto etalon =
      hsp::make_op<hsp::cuda::NonUniformityCorrection<double, double>>();
  etalon->load(etalon_a.string(), etalon_b.string());
  auto nuc =
      hsp::make_op<hsp::cuda::NonUniformityCorrection<uint16_t, double>>();
  nuc->load(rel_a.string(), rel_b.string());
  ops.add(etalon).add(nuc);
  hsp::LineOutputIterator<uint16_t> it(dataset, 0);
  for (auto&& frame : L0_data) {
    *it++ = hsp::cuda::GpuDownloader(ops(dbc(frame)));
  }
  SUCCEED();
}

TEST_F(GF501AVNIRTest, VNIRProcessingCUDAv2) {
  hsp::cuda::GF501A_VN_proc proc;
  proc.load(dark_a.string(), dark_b.string(), etalon_a.string(),
            etalon_b.string(), rel_a.string(), rel_b.string());
  hsp::LineOutputIterator<uint16_t> it(dataset, 0);
  std::transform(L0_data.begin(), L0_data.end(), it, proc);
}

#endif  // HAVE_CUDA
