/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include "avrocpp/api/Compiler.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Stream.hh"
#include "generated/embed_jsons.h"

#include "ValidSchema.hh"
#include "avro_schema_io.h"
#include "AllJSON.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

void show(const RMF_avro_backend::Data& data, std::ostream& out) {
  boost::shared_ptr< ::internal_avro::OutputStream> os =
      internal_avro::ostreamOutputStream(out);
  ::internal_avro::EncoderPtr encoder =
      internal_avro::jsonEncoder(internal_avro::compileJsonSchemaFromString(
          data_deprecated_avro::data_json));
  encoder->init(*os);
  ::internal_avro::codec_traits<RMF_avro_backend::Data>::encode(*encoder, data);
  os->flush();
}

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
