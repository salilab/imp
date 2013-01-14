/**
 *  \file IMP/em/XplorReaderWriter.h
 *  \brief Classes to read or write density files in XPLOR format.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_XPLOR_READER_WRITER_H
#define IMPEM_XPLOR_READER_WRITER_H

#include <IMP/em/em_config.h>
#include "MapReaderWriter.h"
#include "DensityHeader.h"
#include "internal/XplorHeader.h"
#include <math.h>
#include <iostream>
#include <iomanip>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT XplorReaderWriter : public MapReaderWriter
{
#if !defined(DOXYGEN) && !defined(SWIG)
public:
  void read(const char *filename, float **data, DensityHeader &header);
  void write(const char *filename, const float *data,
             const DensityHeader &header );
protected:
  int read_header(std::ifstream & XPLORstream, internal::XplorHeader &header);
  int read_map(std::ifstream &XPLORstream, float *data,
               internal::XplorHeader &header);
#endif
  IMP_OBJECT_METHODS(XplorReaderWriter);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_XPLOR_READER_WRITER_H */
