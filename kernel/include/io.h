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
#include "file.h"
#include "FailureHandler.h"
#include "OptimizerState.h"
#include "internal/utility.h"
#include <boost/format.hpp>

IMP_BEGIN_NAMESPACE

class Model;

/** \name Model IO
    \anchor model_text_io
    Models can be written to files in an easy-to-read
    file format based on \quote{name: value} pairs on separate lines with
    indentation used to indicated nesting (as in Python). The functions
    take a list of the particles to write/read. The particles
    need to be provided in the same order for reading and writing.
    This is not checked.

    The files should have the suffix ".imp".

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
IMPEXPORT void write_particles(const ParticlesTemp &particles,
                               TextOutput out);
IMPEXPORT void read_particles(TextInput in,
                              const ParticlesTemp &particles
#ifndef IMP_DOXYGEN
                    ,Model *m=NULL
#endif
);

IMPEXPORT void write_particles(const ParticlesTemp &particles,
                           const FloatKeys &keys,
                           TextOutput out);

IMPEXPORT void read_particles(TextInput in,
                          const ParticlesTemp &particles,
                          const FloatKeys &keys);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
IMPEXPORT void write_particles(Model *m,
                           TextOutput out);

IMPEXPORT void read_particles(TextInput in, Model *m);
#endif
/** @} */

#ifndef IMP_DOXYGEN
inline void write_model(Model *m,
                     const ParticlesTemp &particles,
                        TextOutput out) {
  write_particles(particles, out);
}
inline void write_model(const ParticlesTemp &particles,
                        TextOutput out) {
  write_particles(particles, out);
}
inline void read_model(TextInput in,
                       const ParticlesTemp &particles
                       ,Model *m=NULL
                       ) {
  read_particles(in, particles, m);
}
inline void write_model(const ParticlesTemp &particles,
                           const FloatKeys &keys,
                        TextOutput out) {
  write_particles(particles, keys, out);
}

inline void read_model(TextInput in,
                          const ParticlesTemp &particles,
                       const FloatKeys &keys) {
  read_particles(in, particles, keys);
}

#endif


#if defined(IMP_USE_NETCDF) || defined(IMP_DOXYGEN)
/** \name Binary I/O
    \anchor model_binary_io
    When lots of data is being written, it can be useful to write the
    data as binary instead of text. Binary writing requires NetCDF.

    The files should have the suffix ".impb".

    For writing if the append is true is a positive integer, then
    the data is added to the file if it already exists.

    For reading, and IOException will be thrown if an invalid frame
    is requested. Frames are always sequential.

    \note Not all particles need to have all the attributes,
    missing attributes will be skipped. However, the set of attributes
    must match on the write and read particles.

    \note These methods should be considered unstable. Use the text
    IO if you want stable long term storage.

    \requires{binary I/O functions, NetCDF}
    @{
*/
//! if append, don't overwrite an existing file
IMPEXPORT void write_particles_binary(const ParticlesTemp &particles,
                                  const FloatKeys &keys,
                                  std::string filename,
                                  bool append=false);
//! load the ith frame if frame is non-negative
IMPEXPORT void read_particles_binary(std::string filename,
                           const ParticlesTemp &particles,
                           const FloatKeys &keys,
                           int frame=-1);

#ifndef IMP_DOXYGEN
inline void write_binary_model(const ParticlesTemp &particles,
                                  const FloatKeys &keys,
                                  std::string filename,
                                  bool append=false) {
  write_particles_binary(particles, keys, filename, append);
}
inline void read_binary_model(std::string filename,
                           const ParticlesTemp &particles,
                           const FloatKeys &keys,
                                  int frame=-1) {
  read_particles_binary(filename, particles, keys, frame);
}
#endif
/** @} */
#endif

/** \name Buffer I/O
    Write/read the state of the particles to/from a buffer in memory.
    \note Not all particles need to have all the attributes,
    missing attributes will be skipped. However, the set of attributes
    must match on the write and read particles.

    \note There is no handling of architectural issues. That is, this
    is only guaranteed to work if it is read and written on the same
    operating system and system bit length. We could probably fix this.

    \note both these methods should be considered unstable.
    @{
*/
//! return a binary buffer with the data
IMPEXPORT std::vector<char>
write_particles_to_buffer(const ParticlesTemp &particles,
                          const FloatKeys &keys);
//! load found attributes into the particles
IMPEXPORT void read_particles_from_buffer( const std::vector<char> &buffer,
                                       const ParticlesTemp &particles,
                                       const FloatKeys &keys);

/** @} */

/** \class WriteParticlesOptimizerState
    Write conformations to different text files.

    \class WriteParticlesFailureHandler
    Write the conformatin when an error occurs.
 */
IMP_MODEL_SAVE(WriteParticles, (const ParticlesTemp &ps, std::string file_name),
               Particles ps_;,
               ps_=ps;,
               ,
               {
                 IMP_LOG(TERSE, "Writing text model file "
                         << file_name << std::endl);
                 write_model(ps_,file_name);
               });
#if defined(IMP_USE_NETCDF) || defined(IMP_DOXYGEN)
/** \class WriteParticlesBinaryOptimizerState
    In contrast to other similar OptimizerStates, this one expectes to write
    all models to the same file. As a result, the file name should not contain
    %1% (if it does, then separate files will be written). The first call will
    overwrite the file.

    \requires{class WriteBinaryOptimizerState, NetCDF}

    \class WriteParticlesBinaryFailureHandler

    \requires{class WriteBinaryFailureHandler, NetCDF}
 */
IMP_MODEL_SAVE(WriteParticlesBinary, (const ParticlesTemp &ps,
                                      const FloatKeys &fks,
                                      std::string file_name),
               Particles ps_; FloatKeys fks_; mutable bool first_;,
               ps_=ps; fks_=fks;first_=-1;,
               ,
               {
                 IMP_LOG(TERSE, "Writing text model file "
                         << file_name << std::endl);
                 write_binary_model(ps_,fks_,file_name, !first_);
                 first_=false;
               });
#endif

#ifndef IMP_DOXYGEN
typedef WriteParticlesBinaryOptimizerState WriteBinaryOptimizerState;
typedef WriteParticlesOptimizerState WriteOptimizerState;
#endif


IMP_END_NAMESPACE

#endif /* IMP_IO_H */
