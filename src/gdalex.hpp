// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_GDALEX_HPP_
#define SRC_GDALEX_HPP_

// GDAL
#include <gdal_priv.h>

// C++ Standard
#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace hsp {
namespace gdal {

inline char* strlwr(char* str) {
  char* orig = str;
  char d = 'a' - 'A';
  for (; *str != '\0'; str++) {
    if (*str >= 'A' && *str <= 'Z') {
      *str = *str + d;
    }
  }
  return orig;
}

inline const char* GetGDALDescription(const char* ext,
                                      const char* default_ret) {
  static const std::map<std::string, std::string> dictionary = {
      {"tif", "GTiff"},
      {"tiff", "GTiff"},
      {"dat", "ENVI"},
      {"bmp", "BMP"},
      {"jpg", "JPEG"}};
  std::string ext_str(ext);
  std::string extension(ext_str.begin() + ext_str.find_first_not_of("."),
                        ext_str.end());
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return dictionary.at(extension).c_str();
}

inline bool IsRasterDataset(const char* filepath) {
  FILE* fp = fopen(filepath, "r");
  if (fp == nullptr) {
    return false;
  }
  fclose(fp);
  char path[512];
  strcpy(path, filepath);  // NOLINT
  char* p = strrchr(path, '.');
  if (p && GetGDALDescription(p + 1, nullptr)) {
    return true;
  }
  if (p == nullptr) {
    p = path + strlen(path);
  }
  strcpy(p, ".hdr");  // NOLINT
  fp = fopen(path, "r");
  if (fp == nullptr) {
    return false;
  }
  fclose(fp);
  return true;
}

inline std::pair<double, double> GetDataTypeMinMax(GDALDataType type) {
  switch (type) {
    case GDT_Byte:
      return std::make_pair(0, 255);
    case GDT_Int16:
      return std::make_pair(std::numeric_limits<int16_t>::min(),
                            std::numeric_limits<int16_t>::max());
    case GDT_UInt16:
      return std::make_pair(std::numeric_limits<uint16_t>::min(),
                            std::numeric_limits<uint16_t>::max());
    case GDT_Float64:
    default:
      return std::make_pair(std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max());
  }
}

inline GDALDataset* GDALCreate(const char* filepath, int cols, int rows,
                               int bands, GDALDataType type) {
  GDALDataset* dataset;
  const char* ext = strrchr(filepath, '.');
  if (ext == nullptr) {
    ext = "tif";
  }
  GDALDriver* poDriver;
  poDriver =
      GetGDALDriverManager()->GetDriverByName(GetGDALDescription(ext, "ENVI"));
  dataset = poDriver->Create(filepath, cols, rows, bands, type, nullptr);
  return dataset;
}

}  // namespace gdal
}  // namespace hsp

#endif  // SRC_GDALEX_HPP_
