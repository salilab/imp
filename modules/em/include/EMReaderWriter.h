/**
 *  \file IMP/em/EMReaderWriter.h
 *  \brief Classes to read or write density files in EM format.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_EM_READER_WRITER_H
#define IMPEM_EM_READER_WRITER_H

#include <IMP/em/em_config.h>
#include "MapReaderWriter.h"
#include "DensityHeader.h"
#include "internal/EMHeader.h"
#include "def.h"

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT EMReaderWriter : public MapReaderWriter
{
public:
#if !defined(DOXYGEN) && !defined(SWIG)
 //! Reads a density file in EM format and stores the information
 /**
  \param[in] filename name of the file to read
  \param[in] data pointer to the data
  \param[in] header DensityHeader() to store the header information
  \exception IOException in case that the filename was not found
  */
  void read(const char *filename, float **data, DensityHeader &header);
  //! Writes a density file in EM format with the header information
  /**
     \param[in] filename name of the file to read
     \param[in] data pointer to the data
     \param[in] header DensityHeader() with the header information
  */
  void write(const char* filename, const float *data,
             const DensityHeader &header);
#endif
protected:
  //! Reads the header
  //! Reads the data
  /**
  \param file ifstream of the file to read
  \param header internal::EMHeader to store the header information
  */
  void read_header(std::ifstream &file, internal::EMHeader &header);
  //! Reads the data
  /**
  \param file ifstream of the file to read
  \param data pointer to store the data
  \param header internal::EMHeader to store the header information
  \exception IOException if the data allocation had failed
  \exception  IOException is the requested data type is not implemented
  */
  void read_data(std::ifstream &file, float **data,
                const internal::EMHeader &header);
  //! Writes the header
  /**
  \param[in] file ofstream of the file to write
  \param[in] header internal::EMHeader with the header information
  \exception IOException if the writing operation had failed.
  */
  void write_header(std::ostream& s, const internal::EMHeader &header);

  IMP_OBJECT_METHODS(EMReaderWriter);
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_EM_READER_WRITER_H */
