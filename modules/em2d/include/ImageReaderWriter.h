/**
 *  \file em2d/ImageReaderWriter.h
 *  \brief Virtual class for reader/writers of images
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_IMAGE_READER_WRITER_H
#define IMPEM2D_IMAGE_READER_WRITER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em/ImageHeader.h"

IMPEM2D_BEGIN_NAMESPACE

template<typename T>
class ImageReaderWriter
{
public:
  virtual ~ImageReaderWriter() {}

  virtual void read(const String &filename, em::ImageHeader& header,
                                     cv::Mat &data) const {
  }

  virtual void read_from_floats(const String &filename, em::ImageHeader& header,
                                     cv::Mat &data) const {
  }

  virtual void read_from_ints(const String &filename, em::ImageHeader& header,
                                     cv::Mat &data) const {
  }

  virtual void write(const String &filename, em::ImageHeader& header,
                                     cv::Mat &data) const {
  }

  virtual void write_to_floats(const String &filename, em::ImageHeader& header,
                                     const cv::Mat &data) const {
  }

  virtual void write_to_ints(const String &filename, em::ImageHeader& header,
                                     cv::Mat &data) const {
  }
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_IMAGE_READER_WRITER_H */
