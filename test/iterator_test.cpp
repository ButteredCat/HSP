// Copyright (C) 2023 Xiao Yunchen
#include "../src/iterator.hpp"

// GTest
#include <gtest/gtest.h>

// C++ Standard
#include <fstream>

// Boost
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

bool filecmp(const std::string& a, const std::string& b) {
  std::ifstream in_a(a, std::ios::binary);
  std::ifstream in_b(b, std::ios::binary);
  in_a.seekg(0, std::ios::end);
  auto size_a = in_a.tellg();
  in_b.seekg(0, std::ios::end);
  auto size_b = in_b.tellg();
  if (size_a != size_b) {
    return false;
  }
  in_a.seekg(0, std::ios::beg);
  in_b.seekg(0, std::ios::beg);

  const int buffer_size = 4096;
  auto buffer_a = std::make_unique<char[]>(buffer_size);
  auto buffer_b = std::make_unique<char[]>(buffer_size);
  while (in_a >> buffer_a.get()) {
    in_b >> buffer_b.get();
    if (memcmp(buffer_a.get(), buffer_b.get(), buffer_size) != 0) {
      return false;
    }
  }
  return true;
}

class IteratorTest : public ::testing::Test {
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
      fs::remove(dst_file);
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
      "/home/xiaoyc/dataset/HGY/"
      "HGY_SWIR-20230429_110205-00000_outdark_mod_ref.dat";
  const std::string dst_file = "/tmp/out.dat";
  int n_samples = 0;
  int n_lines = 0;
  int n_bands = 0;
  GDALDataType type;
};

TEST_F(IteratorTest, SampleInputIteratorCanBeCreated) {
  hsp::SampleInputIterator<float> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  SUCCEED() << "Failed to create iterator";
}

TEST_F(IteratorTest, LineInputIteratorCanBeCreated) {
  hsp::LineInputIterator<float> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  SUCCEED() << "Failed to create iterator";
}

TEST_F(IteratorTest, BandInputIteratorCanBeCreated) {
  hsp::BandInputIterator<float> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  SUCCEED() << "Failed to create iterator";
}

TEST_F(IteratorTest, SampleInputIteratorCanBeDereferenced) {
  hsp::SampleInputIterator<float> it(src_dataset.get(), 0);
  *it;
  SUCCEED() << "Failed to dereference iterator";
}

TEST_F(IteratorTest, LineInputIteratorCanBeDereferenced) {
  hsp::LineInputIterator<float> it(src_dataset.get(), 0);
  *it;
  SUCCEED() << "Failed to dereference iterator";
}

TEST_F(IteratorTest, BandInputIteratorCanBeDereferenced) {
  hsp::BandInputIterator<float> it(src_dataset.get(), 0);
  *it;
  SUCCEED() << "Failed to dereference iterator";
}

TEST_F(IteratorTest, SampleInputIteratorIncrement) {
  hsp::SampleInputIterator<float> it(src_dataset.get(), 0);
  ++it;
  it++;
  SUCCEED() << "Failed to increment iterator";
}

TEST_F(IteratorTest, LineInputIteratorIncrement) {
  hsp::LineInputIterator<float> it(src_dataset.get(), 0);
  ++it;
  it++;
  SUCCEED() << "Failed to increment iterator";
}

TEST_F(IteratorTest, BandInputIteratorIncrement) {
  hsp::BandInputIterator<float> it(src_dataset.get(), 0);
  ++it;
  it++;
  SUCCEED() << "Failed to increment iterator";
}

TEST_F(IteratorTest, LineInputIteratorCopy) {
  hsp::LineInputIterator<float> beg(src_dataset.get(), 0);
  CreateDst();
  for (int i = 0; i < n_lines; ++i) {
    auto err =
        dst_dataset->RasterIO(GF_Write, 0, i, n_samples, 1, beg->data,
                              n_samples, 1, type, n_bands, nullptr, 0, 0, 0);
    ++beg;
  }
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file));
}

TEST_F(IteratorTest, BandInputIteratorCopy) {
  hsp::BandInputIterator<float> beg(src_dataset.get(), 0);
  CreateDst();
  for (int i = 0; i < n_bands; ++i) {
    auto err = dst_dataset->GetRasterBand(i + 1)->RasterIO(
        GF_Write, 0, 0, n_samples, n_lines, beg->data, n_samples, n_lines, type,
        0, 0);
    ++beg;
  }
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file));
}

TEST_F(IteratorTest, SampleInputIteratorCopy) {
  hsp::SampleInputIterator<float> beg(src_dataset.get(), 0);
  CreateDst();
  for (int i = 0; i < n_samples; ++i) {
    auto err = dst_dataset->RasterIO(GF_Write, i, 0, 1, n_lines, beg->data, 1,
                                     n_lines, type, n_bands, nullptr, 0, 0, 0);
    beg++;
  }
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file));
}

TEST_F(IteratorTest, SampleOutputIteratorCanBeCreated) {
  CreateDst();
  hsp::SampleOutputIterator<float> beg(dst_dataset, 0), end(dst_dataset);
  GDALClose(dst_dataset);
  SUCCEED() << "Failed to create iterator";
}

TEST_F(IteratorTest, LineOutputIteratorCanBeCreated) {
  CreateDst();
  hsp::LineOutputIterator<float> beg(dst_dataset, 0), end(dst_dataset);
  GDALClose(dst_dataset);
  SUCCEED() << "Failed to create iterator";
}

TEST_F(IteratorTest, BandOutputIteratorCanBeCreated) {
  CreateDst();
  hsp::BandOutputIterator<float> beg(dst_dataset, 0), end(dst_dataset);
  GDALClose(dst_dataset);
  SUCCEED() << "Failed to create iterator";
}

TEST_F(IteratorTest, CreateIteratorWithNullptr) {
  EXPECT_THROW(hsp::BandOutputIterator<float> beg(nullptr, 0),
               std::runtime_error);
  EXPECT_THROW(hsp::BandOutputIterator<float> end(nullptr), std::runtime_error);
}

TEST_F(IteratorTest, BandOutputIteratorCopy) {
  cv::Mat img =
      cv::Mat::zeros(cv::Size(n_samples, n_lines), cv::DataType<float>::type);
  CreateDst();
  hsp::BandOutputIterator<float> beg(dst_dataset, 0);
  for (int i = 0; i < n_bands; ++i) {
    auto err = src_dataset->GetRasterBand(i + 1)->RasterIO(
        GF_Read, 0, 0, n_samples, n_lines, img.data, n_samples, n_lines, type,
        0, 0);
    *beg;
    ++beg;
  }
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file));
}

TEST_F(IteratorTest, SampleIteratorCopy) {
  hsp::SampleInputIterator<float> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  CreateDst();
  hsp::SampleOutputIterator<float> obeg(dst_dataset, 0);
  std::copy(beg, end, obeg);
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file))
      << "Destination file is not identical with source.";
}

TEST_F(IteratorTest, LineIteratorCopy) {
  hsp::LineInputIterator<float> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  CreateDst();
  hsp::LineOutputIterator<float> obeg(dst_dataset, 0);
  std::copy(beg, end, obeg);
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file))
      << "Destination file is not identical with source.";
}

TEST_F(IteratorTest, BandIteratorCopy) {
  hsp::BandInputIterator<float> beg(src_dataset.get(), 0),
      end(src_dataset.get());
  CreateDst();
  hsp::BandOutputIterator<float> obeg(dst_dataset, 0);
  std::copy(beg, end, obeg);
  GDALClose(dst_dataset);
  EXPECT_TRUE(filecmp(src_file, dst_file))
      << "Destination file is not identical with source.";
}
