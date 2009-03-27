/**
 *  \file ImageReaderWriter.h
 *  \brief Virtual class for reader/writers of images
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_IMAGE_READER_WRITER_H
#define IMPEM_IMAGE_READER_WRITER_H

#include "config.h"
#include "ImageHeader.h"
#include <IMP/algebra/Matrix2D.h>

IMPEM_BEGIN_NAMESPACE

template<typename T>
class ImageReaderWriter
{
public:
  virtual ~ImageReaderWriter() {}

  virtual void read(String filename, ImageHeader& header,
                                     algebra::Matrix2D<T>& data) {
  }

  virtual void write(String filename, ImageHeader& header,
                                     algebra::Matrix2D<T>& data) {
  }
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_IMAGE_READER_WRITER_H */
