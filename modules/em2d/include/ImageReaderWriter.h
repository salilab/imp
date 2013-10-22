/**
 *  \file em2d/ImageReaderWriter.h
 *  \brief Virtual class for reader/writers of images
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_IMAGE_READER_WRITER_H
#define IMPEM2D_IMAGE_READER_WRITER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/base/Object.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em/ImageHeader.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

//! Virtual class for reader/writers of images
class ImageReaderWriter : public IMP::base::Object
{
public:
  ImageReaderWriter(): Object("ImageReaderWriter%1%"){}
  virtual void read(const String &filename, em::ImageHeader& header,
                    cv::Mat &data) const = 0;

  virtual void read_from_floats(const String &filename, em::ImageHeader& header,
                                cv::Mat &data) const = 0;

  virtual void read_from_ints(const String &filename, em::ImageHeader& header,
                              cv::Mat &data) const = 0;

  virtual void write(const String &filename, em::ImageHeader& header,
                     const cv::Mat &data) const = 0;

  virtual void write_to_floats(const String &filename,
                               em::ImageHeader& header,
                               const cv::Mat &data) const = 0;

  virtual void write_to_ints(const String &filename,
                             em::ImageHeader& header,
                             const cv::Mat &data) const = 0;

  IMP_OBJECT_METHODS(ImageReaderWriter);
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_IMAGE_READER_WRITER_H */
