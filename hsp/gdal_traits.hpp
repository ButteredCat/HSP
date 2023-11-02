/**
 * @file gdal_traits.hpp
 * @author xiaoyc
 * @brief 提供了一系列在编译期将C++数据类型转化为GDAL所定义的数据类型值的模板
 * @version 0.1
 * @date 2023-09-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef HSP_GDAL_TRAITS_HPP_
#define HSP_GDAL_TRAITS_HPP_

// GDAL
#include <gdal.h>

// C++ Standard
#include <complex>

namespace hsp {
namespace gdal {

template <typename _Tp>
class DataType {
 public:
  typedef _Tp value_type;
  static GDALDataType type() { return GDT_Unknown; }
};

template <>
class DataType<unsigned char> {
 public:
  typedef unsigned char value_type;
  static GDALDataType type() { return GDT_Byte; }
};

template <>
class DataType<uint16_t> {
 public:
  typedef uint16_t value_type;
  static GDALDataType type() { return GDT_UInt16; }
};

template <>
class DataType<int16_t> {
 public:
  typedef int16_t value_type;
  static GDALDataType type() { return GDT_Int16; }
};

template <>
class DataType<uint32_t> {
 public:
  typedef uint32_t value_type;
  static GDALDataType type() { return GDT_UInt32; }
};

template <>
class DataType<int32_t> {
 public:
  typedef int32_t value_type;
  static GDALDataType type() { return GDT_Int32; }
};

template <>
class DataType<float> {
 public:
  typedef float value_type;
  static GDALDataType type() { return GDT_Float32; }
};

template <>
class DataType<double> {
 public:
  typedef double value_type;
  static GDALDataType type() { return GDT_Float64; }
};

template <>
class DataType<std::complex<int16_t> > {
 public:
  typedef std::complex<int16_t> value_type;
  static GDALDataType type() { return GDT_CInt16; }
};

template <>
class DataType<std::complex<int32_t> > {
 public:
  typedef std::complex<int32_t> value_type;
  static GDALDataType type() { return GDT_CInt32; }
};

template <>
class DataType<std::complex<float> > {
 public:
  typedef std::complex<float> value_type;
  static GDALDataType type() { return GDT_CFloat32; }
};

template <>
class DataType<std::complex<double> > {
 public:
  typedef std::complex<double> value_type;
  static GDALDataType type() { return GDT_CFloat64; }
};

}  // namespace gdal
}  // namespace hsp

#endif  // HSP_GDAL_TRAITS_HPP_
