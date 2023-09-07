// Copyright (C) 2023 Xiao Yunchen
#pragma once
// C++ Standard
#include <string>

// Boost
#include <boost/endian/conversion.hpp>

// project
#include "IDecoder.h"

namespace hsp {
class HgyNIRDecoder : public IDecoder {
  int width() const override {
    const auto head = reinterpret_cast<const u_char*>(get_data().data());
    return boost::endian::load_little_u16(head + 16);
  }
  int height() const override {
    const auto head = reinterpret_cast<const u_char*>(get_data().data());
    return boost::endian::load_little_u16(head + 18);
  }
  int frame_aux_size() const override { return 1024; }
  bool is_leading_bytes_matched(const std::string& bytes) const override {
    return true;
  }
};
}  // namespace hsp
