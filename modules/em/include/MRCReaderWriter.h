/**
 *  \file IMP/em/MRCReaderWriter.h
 *  \brief Classes to read or write MRC files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_MRC_READER_WRITER_H
#define IMPEM_MRC_READER_WRITER_H

#include <IMP/em/em_config.h>
#include "MapReaderWriter.h"
#include "internal/MRCHeader.h"
#include <string>

IMPEM_BEGIN_NAMESPACE

/** A class to read and write MRC files. */
class IMPEMEXPORT MRCReaderWriter : public MapReaderWriter
{
public:
  //! Empty constructor
  MRCReaderWriter() {}
  //! Constructor
  /**
   * param[in] fn name of the file to open or write
   */
  MRCReaderWriter(std::string fn) {
    filename=fn;
  }
#if !defined(DOXYGEN) && !defined(SWIG)
  //! Reads an MRC file and translates the header to the general DensityHeader
  void read(const char *fn_in, float **data, DensityHeader &head);
  //! Writes an MRC file from the data and the general DensityHeader
  void write(const char *fn_out, const float *data, const DensityHeader &head);
#endif

private:


  //! By default the data are read into the grid of the class, but an external
  //! pointer to another grid can be specified
  void read() {
    read(&grid);
  }

  void read(float **pt);
  //! reads the header
  void read_header();
  //! reads the data
  void read_data(float *pt);
  //! reads data of size 8-bit
  void read_8_data(float *pt);
  //! reads data of size 32-bit
  void read_32_data(float *pt);
  void read_grid(void *pt,size_t size,size_t n);
  void seek_to_data();
  //! Write function
  /**
   * param[in] fn name of the file to write
   */
  void write(const char *fn) {
    return write(fn,grid);
  }

  //! Write function
  /**
   * param[in] fn name of the file to write
   * param[in] pt pointer to the data to write
   */
  void write(const char *fn,const float *pt);
  //! Writes the header
  /**
   * param[in] s stream to write the header
   */
  void write_header(std::ofstream &s);
  //! Writes data
  /**
   * param[in] s stream to write the data
   * param[in] pt pointer to the data
   */
  void write_data(std::ofstream &s, const float *pt);

  //! Name of the file
  std::string filename;
  //! file stream for the file read
  std::fstream fs;
  //! The header of the file
  internal::MRCHeader header;
  //! The grid of data. The data is stored in the grid with the convention
  //! that the order of indexes is z,y,x
  float *grid;

  IMP_OBJECT_METHODS(MRCReaderWriter);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_MRC_READER_WRITER_H */
