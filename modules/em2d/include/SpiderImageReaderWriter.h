/**
 *  \file SpiderImageReaderWriter.h
 *  \brief Management of Images in Spider format
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_SPIDER_IMAGE_READER_WRITER_H
#define IMPEM2D_SPIDER_IMAGE_READER_WRITER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/ImageReaderWriter.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em/ImageHeader.h"
#include "IMP/em/header_converters.h"
#include <IMP/algebra/utility.h>
#include <IMP/algebra/endian.h>
#include <IMP/exception.h>
#include <IMP/log.h>
#include <typeinfo>
#include <complex>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

IMPEM2D_BEGIN_NAMESPACE

//! Class to read and write EM images in Spider format. They are stored in
//! the header and data passed as arguments
class SpiderImageReaderWriter: public ImageReaderWriter
{
public:
  String filename_;
  bool skip_type_check_;
  bool force_reversed_;
  bool skip_extra_checkings_;

  //! Empty constructor. It does not force reversed header and does not
  //! skip any of the tests
  /*!
   *  \note reversed is only used in case that the type_check is skipped
   */
  SpiderImageReaderWriter() {
    skip_type_check_=false;
    force_reversed_=false;
    skip_extra_checkings_=false;
  }

  //! Full constructor.
  /*!
   *  \param[in] filename file to read
   *  \param[in] skip_type_check if true, the check for type of image is skipped
   *  \param[in] force_reversed if true, the reverse mode is enforced
   * for reading and writting
   *  \param[in] skip_extra_checkings if true, the most stringent
   * tests for consistency of images are skipped when reading
   */
  SpiderImageReaderWriter(const String &filename,bool skip_type_check,
                        bool force_reversed,bool skip_extra_checkings) {
    filename_=filename;
    skip_type_check_=skip_type_check;
    force_reversed_=force_reversed;
    skip_extra_checkings_=skip_extra_checkings;
  }


  void read(const String &filename,em::ImageHeader& header,
            cv::Mat &data) const {
    this->read_from_floats(filename, header,data);
  }

  void write(const String &filename,  em::ImageHeader& header,
                                    const cv::Mat &data) const {
    this->write_to_floats(filename, header,data);
  }

  void read_from_ints(const String &, em::ImageHeader&, cv::Mat &) const {
  }

  void write_to_ints(const String &, em::ImageHeader&, const cv::Mat &) const {
  }

  //! Reads an image file in Spider format and stores the content
  //! int the header and data parameters
  /*!
    \param[in] filename file to read
    \param[in] header header to store the info
    \param[in] data a matrix to store the grid of data of the image
  */
  void read_from_floats(const String &filename,
                          em::ImageHeader &header,cv::Mat &data) const {
    IMP_LOG_VERBOSE("reading with SpiderImageReaderWriter" << std::endl);
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (in.fail() || in.bad()) {
      IMP_THROW("Error reading from Spider Image " << filename,IOException);
    }
    //! The header format is already in Spider format, just read it
    bool success=header.read(in,skip_type_check_,force_reversed_,
                             skip_extra_checkings_);
    if (!success) {
      IMP_THROW("Error reading header from Spider Image "
                << filename,IOException);
    }
    IMP_LOG_VERBOSE("Header of image " << filename << std::endl
            << header << std::endl);
    // Adjust size of the matrix according to the header
    unsigned int rows = (int)header.get_number_of_rows();
    unsigned int cols = (int)header.get_number_of_columns();
    data.create(rows,cols,CV_64FC1);
    // Read with casting
    float aux;
    for (cvDoubleMatIterator it=data.begin<double>();
                                it!=data.end<double>();++it) {
      if (!(force_reversed_ ^ algebra::get_is_big_endian())) {
        in.read(reinterpret_cast< char* >(&aux), sizeof(float));
      } else {
        algebra::reversed_read(reinterpret_cast< char* >(&aux),
                                              sizeof(float),1,in,true);
      }
      (*it) = static_cast<double>(aux);
    }
    in.close();
  }


  //! Writes an EM image in Spider format
  /*!
   *  \param[in] filename file to write
   *  \param[in] header header with the image info
   *  \param[in] data a matrix with the grid of data of the image
   */
  void write_to_floats(const String &filename,
                        em::ImageHeader& header,
                         const cv::Mat &data) const {
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    //! The image header is already in Spider format, just write it
    header.write(out, force_reversed_ ^ algebra::get_is_big_endian());

    float aux;
    for (cvDoubleConstMatIterator it=data.begin<double>();
                                it!=data.end<double>();++it) {
      aux = (float)(*it);
      if (!(force_reversed_ ^ algebra::get_is_big_endian())) {
        out.write(reinterpret_cast< char* >(&aux), sizeof(float));
      } else {
        algebra::reversed_write(reinterpret_cast< char* >(&aux),
                       sizeof(float),1,out,true);
      }
    }
    out.close();
  }

  IMP_OBJECT_INLINE(SpiderImageReaderWriter,
                    { out << "SpiderImageReaderWriter"; }, {});
};




IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_SPIDER_IMAGE_READER_WRITER_H */
