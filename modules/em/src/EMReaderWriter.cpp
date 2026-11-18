/**
 *  \file EMReaderWriter.cpp
 *  \brief Classes to read or write density files in EM format.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/EMReaderWriter.h>

#include <boost/version.hpp>
#include <cstdint>
#include <cstring>
#include <vector>

#if BOOST_VERSION < 106900
#include <boost/detail/endian.hpp>
#else
#include <boost/predef/other/endian.h>
#endif


IMPEM_BEGIN_NAMESPACE

void EMReaderWriter::read(const char *filename, float **data,
                          DensityHeader &header) {
  std::ifstream file;
  file.open(filename, std::ifstream::in | std::ifstream::binary);
  IMP_USAGE_CHECK(file.good(), "EMReaderWriter::read >> The file "
                                   << filename << " was not found.");
  file.exceptions(std::ifstream::eofbit | std::ifstream::failbit |
                  std::ifstream::badbit);
  internal::EMHeader eheader;
  read_header(file, eheader);

  if (eheader.Objectpixelsize < EPS) {
    eheader.Objectpixelsize = 1.;
    // TODO - return the warning
    //    IMP_WARN("Objectpixelsize == 0 " << std::endl
    //<< "  set Objectpixelsize = 1 to avoid trouble"<< std::endl);
  }
  eheader.generate_common_header(header);
  read_data(file, data, eheader);
  file.close();
}

void EMReaderWriter::write(const char *filename, const float *data,
                           const DensityHeader &header_) {
  std::ofstream s(filename, std::ofstream::out | std::ofstream::binary);
  internal::EMHeader header(header_);
  write_header(s, header);
  s.write((char *)data, sizeof(float) * header.nx * header.ny * header.nz);
  s.close();
}

namespace {
IMP_CLANG_PRAGMA(diagnostic push)
IMP_CLANG_PRAGMA(diagnostic ignored "-Wcast-align")
/* swap bytes */
void swap(char *x, int size) {
  IMP_INTERNAL_CHECK(reinterpret_cast<size_t>(x) % size == 0,
                     "Pointer is not properly aligned in swap");
  unsigned char c;
  unsigned short s;
  unsigned long l;

  switch (size) {
    case 2:  // swap two bytes
      c = *x;
      *x = *(x + 1);
      *(x + 1) = c;
      break;
    case 4:  // swap two shorts (2-byte words)
      s = *(unsigned short *)x;
      *(unsigned short *)x = *((unsigned short *)x + 1);
      *((unsigned short *)x + 1) = s;
      swap((char *)x, 2);
      swap((char *)((unsigned short *)x + 1), 2);
      break;
    case 8:  // swap two longs (4-bytes words)
      l = *(unsigned long *)x;
      *(unsigned long *)x = *((unsigned long *)x + 1);
      *((unsigned long *)x + 1) = l;
      swap((char *)x, 4);
      swap((char *)((unsigned long *)x + 1), 4);
      break;
  }
}
IMP_CLANG_PRAGMA(diagnostic pop)
}

void EMReaderWriter::write_header(std::ostream &s,
                                  const internal::EMHeader &header) {

  internal::EMHeader::EMHeaderParse ehp;
  ehp.Init(header);

#if defined(BOOST_ENDIAN_LITTLE_BYTE) || defined(BOOST_LITTLE_ENDIAN)
  ehp.emdata[internal::EMHeader::EMHeaderParse::LSWAP_OFFSET] = 0;
#else
  ehp.emdata[internal::EMHeader::EMHeaderParse::LSWAP_OFFSET] = 1;
  // byte-swap all ints in the header on big-endian machines:
  swap((char *)&ehp.nx, sizeof(int));
  swap((char *)&ehp.ny, sizeof(int));
  swap((char *)&ehp.nz, sizeof(int));
  for (int i = 0; i < 40; ++i) {
    swap((char *)&ehp.emdata[i], sizeof(int));
  }
#endif

  s.write((char *)&ehp, sizeof(internal::EMHeader::EMHeaderParse));
  IMP_USAGE_CHECK(
      !s.bad(),
      "EMReaderWriter::write_header >> Error writing header to file.");
}

void EMReaderWriter::read_header(std::ifstream &file,
                                 internal::EMHeader &header) {

  internal::EMHeader::EMHeaderParse ehp;
  file.read((char *)&ehp, sizeof(internal::EMHeader::EMHeaderParse));

#if !defined(BOOST_ENDIAN_LITTLE_BYTE) && !defined(BOOST_LITTLE_ENDIAN)
  // byte-swap all ints in the header on big-endian machines:
  swap((char *)&ehp.nx, sizeof(int));
  swap((char *)&ehp.ny, sizeof(int));
  swap((char *)&ehp.nz, sizeof(int));
  for (int i = 0; i < 40; ++i) {
    swap((char *)&ehp.emdata[i], sizeof(int));
  }
#endif

  ehp.InitEMHeader(header);
  // header.Objectpixelsize = 1.0;
}

void EMReaderWriter::read_data(std::ifstream &file, float **data,
                               const internal::EMHeader &header) {

  int nvox = header.nx * header.ny * header.nz;

  // allocate data
  *data = new float[nvox];
  IMP_USAGE_CHECK(
      *data,
      "EMReaderWriter::read_data >> can not allocated space for data. "
      "Requested size: "
          << nvox * sizeof(float));

  // a density of a single voxel can be represented in 1 to 4 bytes.
  // header.type provides this information.
  // 1 : byte, 2 : short (2-byte) integer, 4 : long (4-byte) integer, 5 : float
  int voxel_data_size;
  switch (header.type) {
    case 1:
      voxel_data_size = sizeof(unsigned char);
      break;
    case 2:
      voxel_data_size = sizeof(int16_t);
      break;
    case 4:
      voxel_data_size = sizeof(int32_t);
      break;
    case 5:
      voxel_data_size = sizeof(float);
      break;
    default:
      IMP_THROW("EMReaderWriter::read_data the requested data type "
                    << header.type << " is not implemented.",
                IOException);
  }

  std::vector<char> voxeldata(static_cast<size_t>(nvox) * voxel_data_size);
  file.read(&voxeldata[0], voxeldata.size());
#if defined(BOOST_ENDIAN_LITTLE_BYTE) || defined(BOOST_LITTLE_ENDIAN)
  bool need_swap = (header.lswap == 1);
#else
  bool need_swap = (header.lswap != 1);
#endif
  // Use a suitably aligned buffer for byte swapping
  union {
    char c[4];
    unsigned char u8;
    int16_t i16;
    int32_t i32;
    float f;
  } tmp;
  for (int i = 0; i < nvox; i++) {
    // Note that the data is binary, so we must not use string functions
    // (such as strncpy) here, as they would stop at the first zero byte
    memcpy(tmp.c, &voxeldata[static_cast<size_t>(i) * voxel_data_size],
           voxel_data_size);
    if (need_swap) {
      swap(tmp.c, voxel_data_size);
    }
    switch (header.type) {
      case 1:
        (*data)[i] = static_cast<float>(tmp.u8);
        break;
      case 2:
        (*data)[i] = static_cast<float>(tmp.i16);
        break;
      case 4:
        (*data)[i] = static_cast<float>(tmp.i32);
        break;
      default:
        (*data)[i] = tmp.f;
        break;
    }
  }
}

IMPEM_END_NAMESPACE
