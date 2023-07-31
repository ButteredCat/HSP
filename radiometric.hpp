#pragma once

#include "pch.h"

namespace hsp {

template<typename T>
void load(const std::string& file_path, cv::Mat output) {
    auto poDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(file_path.c_str(), GA_ReadOnly)));
    const int n_samples = poDataset->GetRasterXSize();
    const int n_lines = poDataset->GetRasterYSize();
    const int n_bands = poDataset->GetRasterCount();
    cv::Mat coeff = cv::Mat_<T>(cv::Size(n_samples, n_lines));

    output = coeff;
}


void read(const std::string& file_path) {
    //auto poDataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(GDALOpen(file_path.c_str(), GA_ReadOnly)));
    auto poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
    auto dataset = GDALDatasetUniquePtr(GDALDataset::FromHandle(poDriver->Create(file_path.c_str(), 10, 10, 10, GDT_UInt16, nullptr )));
    auto n = dataset->GetRasterCount();
}


} // namespace hsp


