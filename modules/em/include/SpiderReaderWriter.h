/**
 *  \file SpiderReaderWriter.h
 *  \brief Management of Spider Headers Electron Microscopy. Compatible with
 *  Spider and Xmipp formats
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_SPIDER_READER_WRITER_H
#define IMPEM_SPIDER_READER_WRITER_H

#include "config.h"
#include "ImageHeader.h"
#include "ImageReaderWriter.h"
#include "HeaderConverters.h"
#include "MapReaderWriter.h"
#include "DensityHeader.h"
#include <IMP/algebra/Matrix2D.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/endian.h>
#include <typeinfo>
#include <complex>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

IMPEM_BEGIN_NAMESPACE

//! Class to read EM images in Spider and Xmipp formats. They are stored in
//! the header and data passed as arguments
/**
 * \note Compatible with Xmipp and Spider formats (byte, int, float, complex)
 * \note An ImageHeader class is expected.
 */
template <typename T>
class IMPEMEXPORT SpiderImageReaderWriter: public ImageReaderWriter<T>
{
public:
  String filename_;
  bool skip_type_check_;
  bool force_reversed_;
  bool skip_extra_checkings_;

  //! Empty constructor. It does not force reversed header and does not
  //! skip any of the tests
  /**
   *  \note reversed is only used in case that the type_check is skipped
   */
  SpiderImageReaderWriter(void){
    skip_type_check_=false;
    force_reversed_=false;
    skip_extra_checkings_=false;
  }

  //! Full constructor.
  /**
   *  \param[in] filename file to read
   *  \param[in] skip_type_check if true, the check for type of image is skipped
   *  \param[in] force_reversed if true, the reverse mode is enforced
   * for reading and writting
   *  \param[in] skip_extra_checkings if true, the most stringent
   * tests for consistency of images are skipped when reading
   */
  SpiderImageReaderWriter(String filename,bool skip_type_check,
                        bool force_reversed,bool skip_extra_checkings) {
    filename_=filename;
    skip_type_check_=skip_type_check;
    force_reversed_=force_reversed;
    skip_extra_checkings_=skip_extra_checkings;
  }

  //! Reads a image file in Spider format and stores the content
  //! the header and data parameters
  /**
   *  \param[in] filename file to read
   *  \param[in] header header to store the info
   *  \param[in] data a matrix to store the grid of data of the image
   */
  void read(String filename, ImageHeader& header,
            algebra::Matrix2D<T>& data) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    //! Take advantage that the header format is already in Spider format and
    //! just read it
    header.read(in,skip_type_check_,force_reversed_,skip_extra_checkings_);
    // Adjust size of the matrix according to the header
    data.resize(header.get_rows(),header.get_columns());
    data.read_binary(in,force_reversed_);
    in.close();
  }

  //! Writes an EM image in Spider format
  /**
   *  \param[in] filename file to write
   *  \param[in] header header with the image info
   *  \param[in] data a matrix with the grid of data of the image
   */
  void write(String filename, ImageHeader& header,
            algebra::Matrix2D<T>& data) {
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    //! Take advantage that the image header is already in Spider format and
    //! just write it
    header.write(out, force_reversed_);
    data.write_binary(out,force_reversed_);
    out.close();
  }
};

//! Class to read EM maps (3D) in Spider and Xmipp formats
/**
 * \note: Compatible only with Xmipp 2.2 and Spider floating point maps
 * \note: A DensityHeader class must be provided
 */
class IMPEMEXPORT SpiderMapReaderWriter : public MapReaderWriter {
public:
  String filename_;
  bool skip_type_check_;
  bool force_reversed_;
  bool skip_extra_checkings_;

  //! Empty constructor. It does not force reversed header and does not
  //! skip any of the tests
  SpiderMapReaderWriter() {
    skip_type_check_=false;
    force_reversed_=false;
    skip_extra_checkings_=false;
  }

  //! Full constructor.
  /**
   *  \param[in] filename file to read
   *  \param[in] skip_type_check if true, the check for type of image is skipped
   *  \param[in] force_reversed if true, the reverse mode is enforced
   * for reading and writting
   *  \param[in] skip_extra_checkings if true, the most stringent
   * tests for consistency of images are skipped when reading
   */
  SpiderMapReaderWriter(String filename,bool skip_type_check,
                        bool force_reversed,bool skip_extra_checkings) {
    filename_=filename;
    skip_type_check_=skip_type_check;
    force_reversed_=force_reversed;
    skip_extra_checkings_=skip_extra_checkings;
  }

  //! Reads a map in Spider format and transfers the data and header.
  /**
   *  \param[in] filename file to read
   *  \param[in] data pointer to the structure where to store the data
   *  \param[in] header DensityHeader class where to store the info
   *  from the header.
   */
  void Read(const char *filename, float **data, DensityHeader &header);

  //! Writes a map in Spider format from data and header.
  /**
   *  \param[in] filename file to write
   *  \param[in] data pointer to the structure where data are stored
   *  \param[in] header DensityHeader class where the header info is stored
   *  data from the header.
   */
  void Write(const char *filename,
             const float *data,const DensityHeader &header);
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_SPIDER_READER_WRITER_H */
