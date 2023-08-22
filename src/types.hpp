// Copyright (C) 2023 Xiao Yunchen
#ifndef SRC_TYPES_HPP
#define SRC_TYPES_HPP

#include <boost/smart_ptr/shared_ptr.hpp>


namespace hsp{
template <typename T>
struct Image {
  Image() = default;
  Image(boost::shared_ptr<T[]> d, int w, int h)
      : data{d}, width{w}, height{h} {}
  boost::shared_ptr<T[]> data{nullptr};
  int width = 0;
  int height = 0;
};
}  // namespace hsp

#endif  // SRC_TYPES_HPP