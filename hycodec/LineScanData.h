#pragma once

#include "IRawData.h"

namespace hsp {
class LineScanData : public IRawData {
 public:
  LineScanData(const std::string& raw_data, const DecoderPtr& aux_decoder)
      : IRawData(raw_data, aux_decoder) {}
  int n_samples() const { return sensor_width(); }
  int n_lines() const { return sensor_height(); };
  int n_bands() const { return n_frames(); };
  void traverse() override;
  void decode(const std::string& dst_file, int begin, int end) override{};
};
}  // namespace hsp
