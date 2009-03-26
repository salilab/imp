/**
 *  \file SpiderReaderWriter.cpp
 *  \brief Reader and Writer fro Spider and Xmipp Images and Volumes
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#include <IMP/em/SpiderReaderWriter.h>

IMPEM_BEGIN_NAMESPACE


void SpiderMapReaderWriter::Read(const char *filename,
                                float **data, DensityHeader& header) {
  std::ifstream in;
  in.open(filename, std::ios::in | std::ios::binary);
  ImageHeader h;
  // Read header in Spider format (ImageHeader is in Spider format)
  h.read(in,skip_type_check_,force_reversed_,skip_extra_checkings_);
  // Transfer to format of DensityHeader
  ImageHeader_to_DensityHeader(h,header);
  // Read the data
  size_t n = h.get_slices()*h.get_columns()*h.get_rows();
  (*data)= new float[n];
  IMP::algebra::reversed_read((*data), sizeof(float), n, in,force_reversed_);
  in.close();
}

void SpiderMapReaderWriter::Write(const char *filename,
            const float *data,const DensityHeader &header) {
  std::ofstream out;
  out.open(filename, std::ios::out | std::ios::binary);
  // Transfer from format of DensityHeader
  ImageHeader h;
  DensityHeader_to_ImageHeader(header,h);
  // Write in Spider format (ImageHeader is in Spider format)
  h.write(out, force_reversed_);
  size_t n = h.get_slices()*h.get_columns()*h.get_rows();
  IMP::algebra::reversed_write(data, sizeof(float), n, out,force_reversed_);
  out.close();
}


IMPEM_END_NAMESPACE
