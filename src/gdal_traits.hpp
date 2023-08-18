#ifndef GDAL_TRAITS_HPP
#define GDAL_TRAITS_HPP

#include <gdal.h>

#include <complex>

namespace hsp {
namespace gdal {

template <typename _Tp>
class DataType {
 public:
  typedef _Tp value_type;
  static GDALDataType type() { return GDT_Unknown; };
  // enum{
  //   type         = GDT_Unknown
  // };
};

template <>
class DataType<unsigned char> {
 public:
  typedef unsigned char value_type;
  static GDALDataType type() { return GDT_Byte; };
  // enum {
  //   type         = GDT_Byte
  // };
};

template <>
class DataType<unsigned short> {
 public:
  typedef unsigned short value_type;
  static GDALDataType type() { return GDT_UInt16; };
  // enum {
  //   type         = GDT_UInt16
  // };
};

template <>
class DataType<short> {
 public:
  typedef short value_type;
  static GDALDataType type() { return GDT_Int16; };
  // enum {
  //   type         = GDT_Int16
  // };
};

template <>
class DataType<unsigned int> {
 public:
  typedef unsigned int value_type;
  static GDALDataType type() { return GDT_UInt32; };
  // enum {
  //   type         = GDT_UInt32
  // };
};

template <>
class DataType<int> {
 public:
  typedef int value_type;
  static GDALDataType type() { return GDT_Int32; };
  // enum {
  //   type         = GDT_Int32
  // };
};

template <>
class DataType<float> {
 public:
  typedef float value_type;
  static GDALDataType type() { return GDT_Float32; };
  // enum {
  //   type         = GDT_Float32
  // };
};

template <>
class DataType<double> {
 public:
  typedef double value_type;
  static GDALDataType type() { return GDT_Float64; };
  // enum {
  //   type         = GDT_Float64
  // };
};

template <>
class DataType<std::complex<short> > {
 public:
  typedef std::complex<short> value_type;
  static GDALDataType type() { return GDT_CInt16; };
  // enum {
  //   type         = GDT_CInt16
  // };
};

template <>
class DataType<std::complex<int> > {
 public:
  typedef std::complex<int> value_type;
  static GDALDataType type() { return GDT_CInt32; };
  // enum {
  //   type         = GDT_CInt32
  // };
};

template <>
class DataType<std::complex<float> > {
 public:
  typedef std::complex<float> value_type;
  static GDALDataType type() { return GDT_CFloat32; };
  // enum {
  //   type         = GDT_CFloat32
  // };
};

template <>
class DataType<std::complex<double> > {
 public:
  typedef std::complex<double> value_type;
  static GDALDataType type() { return GDT_CFloat64; };
  // enum {
  //   type         = GDT_CFloat64
  // };
};

}  // namespace gdal
}  // namespace hsp
#endif
