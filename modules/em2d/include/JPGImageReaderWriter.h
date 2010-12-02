/**
 *  \file em2d/JPGImageReaderWriter.h
 *  \brief Management of JPG format for EM images
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_JPG_IMAGE_READER_WRITER_H
#define IMPEM2D_JPG_IMAGE_READER_WRITER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em/ImageHeader.h"
#include <boost/filesystem/convenience.hpp>

IMPEM2D_BEGIN_NAMESPACE

//! Class to read and write EM images in JPG format.
template <typename T>
class JPGImageReaderWriter: public ImageReaderWriter<T>
{
public:
  JPGImageReaderWriter() {}

  //! Reads an image file in JPG format
  /*!
    \param[in] filename file to read
    \param[in] header header to store the info
    \param[in] data a matrix to store the grid of data of the image
    \warning:  The header passed is filled with standard values
      If you need full header information you must work with other
      format. Eg, Spider.
  */
  void read_from_floats(const String &filename,
                          em::ImageHeader &header,cv::Mat &data) const {
    IMP_LOG(IMP::VERBOSE,"reading with JPGImageReaderWriter" << std::endl);
    // read
    cv::Mat temp= cv::imread(filename,0);
    if(temp.empty()) {
      IMP_THROW("Error reading from JPG Image " << filename,IOException);
    }
    temp.assignTo(data,CV_64FC1);
    // fill the header with default values
    header.clear();
 }

  //! Writes an EM image in JPG format
  /*!
    \param[in] filename file to write
    \param[in] header header with the image info
    \param[in] data a matrix with the grid of data of the image
    \warning: Careful: This function writes a 8-bit image.
              You might be discarding float information.
   */
  void write_to_floats(const String &filename, em::ImageHeader& header,
                                        cv::Mat &data) const {
    // discard header
    IMP_LOG(IMP::WARNING,"Writing with JPGImageReaderWriter "
                  "discards image header " << std::endl);
    // check extension
    String ext=boost::filesystem::extension(filename);
    IMP_LOG(IMP::VERBOSE,"JPGImageReaderWriter writting to "
            << filename <<std::endl);
    if(ext!=".jpg" && ext!=".jpeg") {
      IMP_THROW("JPGImageReaderWriter: The filename extension is not .jpg "
                "or .jpeg",IOException);
    }
    // write image
    std::vector<int> flags;
    flags.push_back(CV_IMWRITE_JPEG_QUALITY);
    flags.push_back(100); // 100% quality image
    cv::imwrite(filename,data,flags);
  }
};

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_JPG_IMAGE_READER_WRITER_H */
