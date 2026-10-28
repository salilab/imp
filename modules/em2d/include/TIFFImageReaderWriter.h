/**
 *  \file IMP/em2d/TIFFImageReaderWriter.h
 *  \brief Management of reading/writing TIFF images
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_TIFF_IMAGE_READER_WRITER_H
#define IMPEM2D_TIFF_IMAGE_READER_WRITER_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/em2d/ImageReaderWriter.h"

IMPEM2D_BEGIN_NAMESPACE

//! Management of reading/writing TIFF images
class IMPEM2DEXPORT TIFFImageReaderWriter : public ImageReaderWriter {

 public:
  TIFFImageReaderWriter() {}

  void read(const String &filename, em::ImageHeader &header,
            cv::Mat &data) const override {
    this->read_from_ints(filename, header, data);
  }

  void write(const String &filename, em::ImageHeader &header,
             const cv::Mat &data) const override {
    this->write_to_ints(filename, header, data);
  }

  void write_to_floats(const String &, em::ImageHeader &,
                       const cv::Mat &) const override {}

  void read_from_floats(const String &, em::ImageHeader &,
                        cv::Mat &) const override {}

  //! Reads an image file in TIFF format
  /*!
    \param[in] filename file to read
    \param[in] header header to store the info
    \param[in] data a matrix to store the grid of data of the image
    \note Only the first image in the file is read, at 8 bits per pixel.
          Color images are converted to grayscale.
    \warning:  The header passed is filled with standard values
      If you need full header information you must work with other
      format. Eg, Spider.
  */
  void read_from_ints(const String &filename, em::ImageHeader &header,
                      cv::Mat &data) const override;

  //! Writes an EM image in TIFF format
  /*!
    \param[in] filename file to write
    \param[in] header header with the image info
    \param[in] data a matrix with the grid of data of the image
    \warning: Careful: This function writes a 8-bit image.
              You might be discarding float information.
   */
  void write_to_ints(const String &filename, em::ImageHeader &header,
                     const cv::Mat &data) const override;

  IMP_OBJECT_METHODS(TIFFImageReaderWriter);
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_TIFF_IMAGE_READER_WRITER_H */
