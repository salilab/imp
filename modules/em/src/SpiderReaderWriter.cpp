/**
 *  \file SpiderReaderWriter.cpp
 *  \brief Reader and Writer fro Spider Volumes
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/algebra/endian.h"
#include "IMP/base/exception.h"

IMPEM_BEGIN_NAMESPACE


void SpiderMapReaderWriter::read(const char *filename,
                                float **data, DensityHeader& header) {
  std::ifstream in;
  in.open(filename, std::ios::in | std::ios::binary);
  if (in.fail() || in.bad()) {
    IMP_THROW("Error reading from Spider Map file", IOException);
  }
  ImageHeader h;
  // Read header in Spider format (ImageHeader is in Spider format)
  h.read(in,skip_type_check_,force_reversed_,skip_extra_checkings_);
  // Transfer to format of DensityHeader
  ImageHeader_to_DensityHeader(h,header);
  // Read the data
  size_t n =
    static_cast<size_t>(h.get_number_of_slices()
                        *h.get_number_of_columns()*h.get_number_of_rows());
  (*data)= new float[n];
  IMP::algebra::reversed_read((*data), sizeof(float), n, in,
                              force_reversed_ ^ algebra::get_is_big_endian());
  in.close();
}

void SpiderMapReaderWriter::write(const char *filename,
            const float *data,const DensityHeader &header) {
  std::ofstream out;
  out.open(filename, std::ios::out | std::ios::binary);
  // Transfer from format of DensityHeader
  ImageHeader h;
  DensityHeader_to_ImageHeader(header,h);
  // Write in Spider format (ImageHeader is in Spider format)
  h.write(out, force_reversed_ ^ algebra::get_is_big_endian());
  size_t n =
    static_cast<size_t>(h.get_number_of_slices()
                        *h.get_number_of_columns()*h.get_number_of_rows());
  IMP::algebra::reversed_write(data, sizeof(float), n, out,
                               force_reversed_ ^ algebra::get_is_big_endian());
  out.close();
}


IMPEM_END_NAMESPACE
