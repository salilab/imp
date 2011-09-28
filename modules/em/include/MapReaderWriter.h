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
class IMPEMEXPORT MapReaderWriter : public IMP::base::Object
{
public:
  MapReaderWriter(): Object("MapReaderWriter%1%"){}
#if !defined(DOXYGEN) && !defined(SWIG)
  // since swig can't see these methods, it wants to create a wrapper
  // for the class
  virtual void read(const char *filename, float **data, DensityHeader &) {
    IMP_UNUSED(filename); IMP_UNUSED(data);
    IMP_FAILURE("Don't use the base class");
};
  virtual void write(const char *filename, const float *data,
                     const DensityHeader &) {
    IMP_UNUSED(filename); IMP_UNUSED(data);
    IMP_FAILURE("Don't use the base class");
  };
#endif

  IMP_OBJECT_INLINE(MapReaderWriter, {out << "MapReaderWriter";}, {});
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_MAP_READER_WRITER_H */
