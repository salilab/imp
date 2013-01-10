/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "avro_schemas.h"
#include <RMF/internal/paths.h>
#include <RMF/decorators.h>
#include <avro/Compiler.hh>
#include <stdexcept>
#include <avro/Encoder.hh>
#include <avro/Stream.hh>

namespace RMF {
namespace internal {
#define RMF_SCHEMA(name)                                                       \
  avro::ValidSchema get_##name##_schema() {                                    \
    std::string path = get_data_path(#name ".json");                           \
    std::auto_ptr<avro::InputStream> is = avro::fileInputStream(path.c_str()); \
    return avro::compileJsonSchemaFromStream(*is);                             \
  }

RMF_SCHEMA(All);
RMF_SCHEMA(File);
RMF_SCHEMA(Nodes);
RMF_SCHEMA(Data);

void show(const RMF_internal::Data &data,
          std::ostream             &out) {
  std::auto_ptr<avro::OutputStream> os
    = avro::ostreamOutputStream(out);
  avro::EncoderPtr encoder
    = avro::jsonEncoder(RMF::internal::get_Data_schema());
  encoder->init(*os);
  avro::codec_traits<RMF_internal::Data>::encode(*encoder, data);
  os->flush();
}

}   // namespace internal
} /* namespace RMF */
