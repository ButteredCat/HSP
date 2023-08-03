#pragma once
#include <boost/endian/conversion.hpp>

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
  bool is_leading_bytes_matched(const std::string& bytes) const override {
    return true;
  }
};
}  // namespace hsp