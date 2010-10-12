/**
 *  \file ImageReaderWriter.h
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

  virtual void read(String filename, em::ImageHeader& header,
                                     cv::Mat& data) {
  }

  virtual void read_from_floats(String filename, em::ImageHeader& header,
                                     cv::Mat& data) {
  }

  virtual void read_from_ints(String filename, em::ImageHeader& header,
                                     cv::Mat& data) {
  }

  virtual void write(String filename, em::ImageHeader& header,
                                     cv::Mat& data) {
  }

  virtual void write_to_floats(String filename, em::ImageHeader& header,
                                     cv::Mat& data) {
  }

  virtual void write_to_ints(String filename, em::ImageHeader& header,
                                     cv::Mat& data) {
  }
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_IMAGE_READER_WRITER_H */
