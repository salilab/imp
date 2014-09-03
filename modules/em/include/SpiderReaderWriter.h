/**
 *  \file IMP/em/SpiderReaderWriter.h
 *  \brief Management of Spider Headers Electron Microscopy. Compatible with
 *  Spider and Xmipp formats
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM_SPIDER_READER_WRITER_H
#define IMPEM_SPIDER_READER_WRITER_H

#include "IMP/em/em_config.h"
#include "IMP/em/ImageHeader.h"
#include "IMP/em/header_converters.h"
#include "IMP/em/MapReaderWriter.h"
#include "IMP/em/DensityHeader.h"
#include <IMP/algebra/utility.h>
#include <IMP/algebra/endian.h>
#include <IMP/base/exception.h>
#include <IMP/base/log.h>
#include <typeinfo>
#include <complex>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

IMPEM_BEGIN_NAMESPACE

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
    skip_type_check_ = false;
    force_reversed_ = false;
    skip_extra_checkings_ = false;
  }

  //! Full constructor.
  /**
   *  \param[in] filename file to read
   *  \param[in] skip_type_check if true, the check for type of image is skipped
   *  \param[in] force_reversed if true, the reverse mode is enforced
   * for reading and writing
   *  \param[in] skip_extra_checkings if true, the most stringent
   * tests for consistency of images are skipped when reading
   */
  SpiderMapReaderWriter(String filename, bool skip_type_check,
                        bool force_reversed, bool skip_extra_checkings) {
    filename_ = filename;
    skip_type_check_ = skip_type_check;
    force_reversed_ = force_reversed;
    skip_extra_checkings_ = skip_extra_checkings;
  }
#if !defined(DOXYGEN) && !defined(SWIG)
  //! Reads a map in Spider format and transfers the data and header.
  /**
   *  \param[in] filename file to read
   *  \param[in] data pointer to the structure where to store the data
   *  \param[in] header DensityHeader class where to store the info
   *  from the header.
   */
  void read(const char *filename, float **data, DensityHeader &header);

  //! Writes a map in Spider format from data and header.
  /**
   *  \param[in] filename file to write
   *  \param[in] data pointer to the structure where data are stored
   *  \param[in] header DensityHeader class where the header info is stored
   *  data from the header.
   */
  void write(const char *filename, const float *data,
             const DensityHeader &header);
#endif
  IMP_OBJECT_METHODS(SpiderMapReaderWriter);
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_SPIDER_READER_WRITER_H */
