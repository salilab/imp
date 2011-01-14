/**
 *  \file XplorReaderWriter.h
 *  \brief Classes to read or write density files in XPLOR format.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_XPLOR_READER_WRITER_H
#define IMPEM_XPLOR_READER_WRITER_H

#include "em_config.h"
#include "MapReaderWriter.h"
#include "DensityHeader.h"
#include "internal/XplorHeader.h"
#include <math.h>
#include <iostream>
#include <iomanip>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT XplorReaderWriter : public MapReaderWriter
{
public:
  void Read(const char *filename, float **data, DensityHeader &header);
  void Write(const char *filename, const float *data,
             const DensityHeader &header );
protected:
  int ReadHeader(std::ifstream & XPLORstream, internal::XplorHeader &header);
  int ReadMap(std::ifstream &XPLORstream, float *data,
              internal::XplorHeader &header);

  IMP_OBJECT_INLINE(XplorReaderWriter, {out << "XplorReaderWriter";}, {});
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_XPLOR_READER_WRITER_H */
