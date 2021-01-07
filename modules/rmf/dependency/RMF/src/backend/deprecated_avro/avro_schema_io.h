/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_AVRO_SCHEMA_IO_H
#define RMF_INTERNAL_AVRO_SCHEMA_IO_H

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>

#include "AllJSON.h"
#include "Encoder.hh"
#include "RMF/config.h"
#include "RMF/exceptions.h"
#include "RMF/log.h"
#include "Specific.hh"
#include "avrocpp/api/DataFile.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Stream.hh"
#include "avrocpp/api/ValidSchema.hh"

namespace RMF_avro_backend {
struct Data;
}  // namespace RMF_avro_backend

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

void show(const RMF_avro_backend::Data& data, std::ostream& out = std::cout);

#if BOOST_VERSION < 104500

#define RMF_RENAME(old, new)                                               \
  int success = std::rename(old.c_str(), new.c_str());                     \
  if (success != 0) {                                                      \
    RMF_THROW(Message("Could not rename") << Component(new), IOException); \
  }                                                                        \
  RMF_TRACE("Renamed " << old << " to " << new)
#else

#define RMF_RENAME(old, new)                                        \
  try {                                                             \
    boost::filesystem::rename(old, new);                            \
  }                                                                 \
  catch (const std::exception& e) {                                 \
    RMF_THROW(Message(std::string("Could not rename: ") + e.what()) \
                  << Component(new),                                \
              IOException);                                         \
  }                                                                 \
  RMF_TRACE("Renamed " << old << " to " << new)

#endif

/** Write a schema to a file in a safe manner (with renaming
    after writing).

    Should be in another header.*/
template <class Data>
void write(const Data& data, internal_avro::ValidSchema schema,
           std::string path) {
  std::string temppath = path + ".new";
  {
    RMF_TRACE("Writing file " << temppath);
    try {
      internal_avro::DataFileWriter<Data> wr(temppath.c_str(), schema);
      wr.write(data);
      wr.flush();
    }
    catch (std::exception& e) {
      RMF_THROW(Message(e.what()) << Component(temppath), IOException);
    }
  }
  RMF_RENAME(temppath, path);
}

template <class Data>
void write_text(const Data& data, internal_avro::ValidSchema schema,
                std::string path) {
  std::string temppath = path + ".new";
  {
    boost::shared_ptr<internal_avro::Encoder> encoder =
        internal_avro::jsonEncoder(schema);
    boost::shared_ptr<internal_avro::OutputStream> stream =
        internal_avro::fileOutputStream(temppath.c_str());
    encoder->init(*stream);
    try {
      internal_avro::encode(*encoder, data);
      encoder->flush();
      stream->flush();
    }
    catch (std::exception& e) {
      RMF_THROW(Message(e.what()) << Component(temppath), IOException);
    }
  }
  RMF_RENAME(temppath, path);
}
}
}

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_AVRO_SCHEMA_IO_H */
