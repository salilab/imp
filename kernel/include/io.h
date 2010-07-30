/**
 *  \file IMP/io.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_IO_H
#define IMP_IO_H

#include "kernel_config.h"
#include "Model.h"
#include "file.h"
#include "FailureHandler.h"

IMP_BEGIN_NAMESPACE
/** \name Model IO
    \anchor model_text_io
    Models can be written to files in an easy-to-read
    file format based on \quote{name: value} pairs on separate lines with
    indentation used to indicated nesting (as in Python). The functions
    take a list of the particles to write/read. The particles
    need to be provided in the same order for reading and writing.
    This is not checked.

    If a passed particle has a particle attribute which is a particle
    not in the list, an IOException is thrown. This can change to
    silently ignoring such attributes if desired.

    If a list of attributes is passed to read or write, only those
    attributes are read.

    \note If you are just interested in saving the state of a model during
    runtime, use an IMP::ConfigurationSet instead. It should be
    substantially faster.
    @{
*/
#ifndef IMP_DOXYGEN
IMPEXPORT void write_model(Model *m,
                           const ParticlesTemp &particles,
                           TextOutput out);
#endif
IMPEXPORT void write_model(const ParticlesTemp &particles,
                           TextOutput out);
IMPEXPORT void read_model(TextInput in,
                          const ParticlesTemp &particles
#ifndef IMP_DOXYGEN
                          ,Model *m=NULL
#endif
);

IMPEXPORT void write_model(const ParticlesTemp &particles,
                           const FloatKeys &keys,
                           TextOutput out);

IMPEXPORT void read_model(TextInput in,
                          const ParticlesTemp &particles,
                          const FloatKeys &keys);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
IMPEXPORT void write_model(Model *m,
                           TextOutput out);

IMPEXPORT void read_model(TextInput in, Model *m);
#endif
/** @} */

#if defined(IMP_USE_NETCDF) || defined(IMP_DOXYGEN)
/** \name Binary I/O
    \anchor model_binary_io
    When lots of data is being written, it can be useful to write the
    data as binary instead of text. Binary writing requires NetCDF.

    For writing if the frame is a positive integer, then the data is
    added to the file if it already exists.
    @{
*/
IMPEXPORT void write_binary_model(const ParticlesTemp &particles,
                                  const FloatKeys &keys,
                                  std::string filename,
                                  int frame=-1);
IMPEXPORT void read_binary_model(std::string filename,
                                 const ParticlesTemp &particles,
                                 const FloatKeys &keys,
                                 int frame=-1);
/** @} */
#endif

/** \brief Dump the state of the model to a file on an error and then
    go on the the other handlers.

    When an error (check or assertion failure) occurs, the model is
    dumped to the specified file.

    \verbinclude dump_on_error.py
 */
class IMPEXPORT DumpModelOnFailure: public FailureHandler {
  Model *m_;
  TextOutput file_name_;
 public:
  DumpModelOnFailure(Model *m, TextOutput out);
  IMP_FAILURE_HANDLER(DumpModelOnFailure);
};

IMP_OBJECTS(DumpModelOnFailure,DumpModelOnFailures);

IMP_END_NAMESPACE

#endif /* IMP_IO_H */
