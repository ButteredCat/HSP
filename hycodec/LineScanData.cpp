#include "LineScanData.h"

#include <exception>
#include <fstream>

namespace hsp {
void LineScanData::traverse() {
  std::ifstream raw_in(raw_data_, std::ios::binary);
  if (!raw_in.is_open()) {
    throw std::runtime_error("Unable to open raw data file!");
  }
  std::vector<char> aux_buffer(frame_aux_size());

  // read aux of the first frame to get basic infomation
  raw_in.read(aux_buffer.data(), frame_aux_size());
  aux_dec->set_data(aux_buffer);
  if (!aux_dec->is_leading_bytes_matched(leading_bytes())) {
    throw std::exception();
  }
  set_sensor_height(aux_dec->height());
  set_sensor_width(aux_dec->width());
  raw_in.seekg(0, raw_in.beg);

  const auto data_size = static_cast<std::size_t>(
      band_aux_size() * sensor_height() +
      sensor_width() * sensor_height() * bytes_per_pixel());
  int frame_count = 0;

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
}  // namespace hsp
