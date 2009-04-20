/**
 *  \file MapReaderWriter.h
 *  \brief An abstract class for reading a map
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_MAP_READER_WRITER_H
#define IMPEM_MAP_READER_WRITER_H

#include "config.h"
#include "DensityHeader.h"
#include <iostream>
#include <fstream>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT MapReaderWriter
{
public:
  virtual void Read(const char *filename, float **data, DensityHeader &header)
  {}
  virtual void Write(const char *filename, const float *data,
                     const DensityHeader &header) {}
  virtual ~MapReaderWriter() {}

};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_MAP_READER_WRITER_H */
