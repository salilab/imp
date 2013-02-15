/**
 *  \file TIFFImageReaderWriter.h
 *  \brief Management of reading/writing TIFF images
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_TIFF_IMAGE_READER_WRITER_H
#define IMPEM2D_TIFF_IMAGE_READER_WRITER_H

#include "IMP/em2d/ImageReaderWriter.h"
#include <boost/filesystem.hpp>

IMPEM2D_BEGIN_NAMESPACE

class TIFFImageReaderWriter: public ImageReaderWriter {

public:

  TIFFImageReaderWriter() {}

  void read(const String &filename,
            em::ImageHeader& header,
            cv::Mat &data) const {
    this->read_from_ints(filename, header,data);
  }

  void write(const String &filename, em::ImageHeader& header,
                                     const  cv::Mat &data) const {
    this->write_to_ints(filename, header,data);
  }

  void write_to_floats(const String &, em::ImageHeader&, const cv::Mat &) const
  {
  }

  void read_from_floats(const String &, em::ImageHeader&, cv::Mat &) const {}

  //! Reads an image file in TIFF format
  /*!
    \param[in] filename file to read
    \param[in] header header to store the info
    \param[in] data a matrix to store the grid of data of the image
    \warning:  The header passed is filled with standard values
      If you need full header information you must work with other
      format. Eg, Spider.
  */
  void read_from_ints(const String &filename,
                          em::ImageHeader &header,cv::Mat &data) const {
    IMP_LOG_VERBOSE("reading with TIFFImageReaderWriter" << std::endl);
    // read
    cv::Mat temp= cv::imread(filename,0);
    if(temp.empty()) {
      IMP_THROW("Error reading from TIFF Image " << filename,IOException);
    }
    temp.assignTo(data,CV_64FC1);
    // fill the header with default values
    header.clear();
 }

  //! Writes an EM image in TIFF format
  /*!
    \param[in] filename file to write
    \param[in] header header with the image info
    \param[in] data a matrix with the grid of data of the image
    \warning: Careful: This function writes a 16-bit image.
              You might be discarding float information.
   */
  void write_to_ints(const String &filename,
                      em::ImageHeader& header,
                                        const cv::Mat &data) const {
    IMP_UNUSED(header);
    // discard header
    IMP_LOG(IMP::WARNING,"Writing with TIFFImageReaderWriter "
                  "discards image header " << std::endl);
    // check extension
    String ext=boost::filesystem::extension(filename);
    IMP_LOG_VERBOSE("TIFFImageReaderWriter writting to "
            << filename <<std::endl);
    if(ext!=".tiff" && ext!=".tif") {
      IMP_THROW("TIFFImageReaderWriter: The filename extension is not .tiff "
                "or .tif",IOException);
    }

    // Convert the data value to 16-bit so it can be written as TIFF
    cv::Mat TIFF_data;
    double max,min;
    cv::minMaxLoc(data,&min,&max);
    double TIFF_max= 255;
    double TIFF_min=0;
    double alpha = (TIFF_max-TIFF_min)/(max-min);
    double beta = TIFF_min-alpha*min;
    data.convertTo(TIFF_data,CV_8UC1,alpha,beta);
    // write image
    cv::minMaxLoc(TIFF_data,&min,&max);

    cv::imwrite(filename,TIFF_data);
  }

  IMP_OBJECT_INLINE(TIFFImageReaderWriter,
                    { out << "TIFFImageReaderWriter"; }, {});
};


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_TIFF_IMAGE_READER_WRITER_H */
