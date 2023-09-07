#include "LineScanData.h"

#include <gdal.h>
#include <gdal_priv.h>

#include <boost/filesystem.hpp>
#include <exception>
#include <fstream>

#include "gdal_traits.hpp"
#include "gdalex.hpp"

namespace hsp {

namespace fs = boost::filesystem;

void LineScanData::Traverse() {
  std::ifstream raw_in(raw_data_, std::ios::binary);
  if (!raw_in.is_open()) {
    throw std::runtime_error("Unable to open raw data file!");
  }
  std::vector<char> aux_buffer(max_aux_size());

  // read aux of the first frame to get basic infomation
  raw_in.read(aux_buffer.data(), aux_buffer.size());
  aux_dec->set_data(aux_buffer);
  if (!aux_dec->is_leading_bytes_matched(leading_bytes())) {
    throw std::exception();
  }
  set_frame_aux_size(aux_dec->frame_aux_size());
  set_sensor_height(aux_dec->height());
  set_sensor_width(aux_dec->width());
  raw_in.seekg(0, raw_in.beg);

  const auto data_size = static_cast<std::size_t>(
      band_aux_size() * sensor_height() +
      sensor_width() * sensor_height() * bytes_per_pixel());
  int frame_count = 0;

  //
  while (raw_in.read(aux_buffer.data(), frame_aux_size())) {
    if (aux_dec->is_leading_bytes_matched(leading_bytes())) {
      raw_in.seekg(data_size, raw_in.cur);
      ++frame_count;
    } else {
      break;
    }
  }
  set_n_frames(frame_count);
  set_traversed(true);
}

// std::vector<char> LineScanData::PrepareData(const std::vector<char>& data) {
//   for()
// }


void LineScanData::ToRaster(const std::string& dst_file, int begin, int end) {
  if (!is_traversed()) {
    Traverse();
  }

  GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(
      GetGDALDescription(fs::path(dst_file).extension().c_str(), "ENVI"));
  if (!poDriver) {
    throw std::exception();
  }
  auto dataset = GDALDatasetUniquePtr(
      poDriver->Create(dst_file.c_str(), n_samples(), n_lines(), n_bands(),
                       gdal::DataType<uint16_t>::type(), nullptr));
  if (!dataset) {
    throw std::exception();
  }

  const auto data_size = static_cast<std::size_t>(
      band_aux_size() * sensor_height() +
      sensor_width() * sensor_height() * bytes_per_pixel());
  std::vector<char> buffer(data_size);
  std::ifstream raw_in(raw_data_, std::ios::binary);
  for (int i = 0; i != n_lines(); ++i) {
    raw_in.seekg(frame_aux_size(), raw_in.cur);
    raw_in.read(buffer.data(), buffer.size());
    // auto prepared = PrepareData(buffer);
    // dataset->RasterIO(GF_Write, 0, i, n_samples(), 1, prepared.data(),
    //                   n_samples(), 1, gdal::DataType<uint16_t>::type(),
    //                   n_bands(), nullptr, 0, 0, 0);
  }
}

}  // namespace hsp
