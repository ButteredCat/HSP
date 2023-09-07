// Copyright (C) 2023 Xiao Yunchen
#pragma once

// C++ Standard
#include <string>

// project
#include "IRawData.h"

namespace hsp {
class LineScanData : public IRawData {
 public:
  LineScanData(const std::string& raw_data, const DecoderPtr& aux_decoder)
      : IRawData(raw_data, aux_decoder) {}
  int n_samples() const { return sensor_width(); }
  int n_lines() const { return sensor_height(); }
  int n_bands() const { return n_frames(); }
  void Traverse() override;
  void ToRaster(const std::string& dst_file, int begin, int end) override;

 protected:
  // virtual std::vector<char> PrepareData(const std::vector<char>& data);
};
}  // namespace hsp
