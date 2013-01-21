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
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstdio>
#include <RMF/exceptions.h>

namespace RMF {
namespace internal {
RMFEXPORT avro::ValidSchema get_All_schema();
RMFEXPORT avro::ValidSchema get_File_schema();
RMFEXPORT avro::ValidSchema get_Nodes_schema();
RMFEXPORT avro::ValidSchema get_Data_schema();

void show(const RMF_internal::Data &data,
          std::ostream             &out = std::cout);


#if BOOST_VERSION < 104400

#  define RMF_RENAME(old, new)                               \
  int success = std::rename(old.c_str(), new.c_str());       \
  if (success != 0) {                                        \
    RMF_THROW(Message("Could not rename") << Component(new), \
              IOException);                                  \
  }                                                          \

#else

#  define RMF_RENAME(old, new)                               \
  try {                                                      \
    boost::filesystem::rename(old, new);                     \
  } catch (const std::exception &e) {                        \
    RMF_THROW(Message("Could not rename") << Component(new), \
              IOException);                                  \
  }

#endif

/** Write a schema to a file in a safe manner (with renaming
    after writing).

    Should be in another header.*/
template <class Data>
void write(const Data &data, avro::ValidSchema schema, std::string path) {
  std::string temppath = path + ".new";
  try {
    avro::DataFileWriter<Data> wr(temppath.c_str(), schema);
    wr.write(data);
    wr.flush();
  } catch (std::exception &e) {
    RMF_THROW(Message(e.what()) << Component(temppath),
              IOException);
  }
  RMF_RENAME(temppath, path);
}
}
}


#endif  /* RMF_INTERNAL_UTILITY_H */
