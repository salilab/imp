/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_AVRO_SCHEMAS_H
#define RMF_INTERNAL_AVRO_SCHEMAS_H

#include <RMF/config.h>
#include <avro/ValidSchema.hh>
#include <backend/avro/AllJSON.h>

// should be in another header, but I'm lazy
#include <avro/DataFile.hh>
#include <avro/Stream.hh>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/version.hpp>
#include <cstdio>
#include <RMF/exceptions.h>
#include <RMF/log.h>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {
RMFEXPORT avro::ValidSchema get_All_schema();
RMFEXPORT avro::ValidSchema get_File_schema();
RMFEXPORT avro::ValidSchema get_Nodes_schema();
RMFEXPORT avro::ValidSchema get_Data_schema();
RMFEXPORT avro::ValidSchema get_Frame_schema();

void show(const RMF_avro_backend::Data &data,
          std::ostream             &out = std::cout);


#if BOOST_VERSION < 104500

#  define RMF_RENAME(old, new)                               \
  int success = std::rename(old.c_str(), new.c_str());       \
  if (success != 0) {                                        \
    RMF_THROW(Message("Could not rename") << Component(new), \
              IOException);                                  \
  }                                                          \
  RMF_TRACE(get_avro_logger(), "Renamed " << old             \
                                                << " to "    \
               << new)
#else

#  define RMF_RENAME(old, new)                                          \
  try {                                                                 \
    boost::filesystem::rename(old, new);                                \
  } catch (const std::exception &e) {                                   \
    RMF_THROW(Message(std::string("Could not rename: ")+e.what())       \
              << Component(new),                                        \
              IOException);                                             \
  }                                                                     \
  RMF_TRACE(get_avro_logger(), "Renamed " << old                        \
            << " to "                                                   \
            << new)

#endif

/** Write a schema to a file in a safe manner (with renaming
    after writing).

    Should be in another header.*/
template <class Data>
void write(const Data &data, avro::ValidSchema schema, std::string path) {
  std::string temppath = path + ".new";
  {
    RMF_TRACE(get_avro_logger(), "Writing file " << temppath);
    try {
      avro::DataFileWriter<Data> wr(temppath.c_str(), schema);
      wr.write(data);
      wr.flush();
    } catch (std::exception &e) {
      RMF_THROW(Message(e.what()) << Component(temppath),
                IOException);
    }
  }
  RMF_RENAME(temppath, path);
}

template <class Data>
void write_text(const Data &data, avro::ValidSchema schema, std::string path) {
  std::string temppath = path + ".new";
  {
    boost::shared_ptr<avro::Encoder> encoder = avro::jsonEncoder(schema);
    std::auto_ptr<avro::OutputStream> stream
        = avro::fileOutputStream(temppath.c_str());
    encoder->init(*stream);
    try {
      avro::encode(*encoder, data);
      encoder->flush();
      stream->flush();
    } catch (std::exception &e) {
      RMF_THROW(Message(e.what()) << Component(temppath),
                IOException);
    }
  }
  RMF_RENAME(temppath, path);
}
}
}

RMF_DISABLE_WARNINGS

#endif  /* RMF_INTERNAL_UTILITY_H */
