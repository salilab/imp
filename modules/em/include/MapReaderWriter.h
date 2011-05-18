/**
 *  \file MapReaderWriter.h
 *  \brief An abstract class for reading a map
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_MAP_READER_WRITER_H
#define IMPEM_MAP_READER_WRITER_H

#include "../Object.h"
#include "em_config.h"
#include "DensityHeader.h"
#include <iostream>
#include <fstream>

IMPEM_BEGIN_NAMESPACE

/** \brief The base class to handle reading and writing of density maps.

    They should never be stored, only created immediately and passed to
    the read or write function.
 */
class IMPEMEXPORT MapReaderWriter : public Object
{
public:
  MapReaderWriter(): Object("MapReaderWriter%1%"){}
#ifndef DOXYGEN
  virtual void read(const char *filename, float **data, DensityHeader &header)
    =0;
  virtual void write(const char *filename, const float *data,
                     const DensityHeader &header) =0;
#endif

  IMP_OBJECT_INLINE(MapReaderWriter, {out << "MapReaderWriter";}, {});
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_MAP_READER_WRITER_H */
