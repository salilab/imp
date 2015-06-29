/**
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <avrocpp/api/Compiler.hh>
#include <avrocpp/api/Encoder.hh>
#include <boost/shared_ptr.hpp>
#include <generated/embed_jsons.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "RMF/ID.h"
#include "RMF/Vector.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/types.h"
#include "avrocpp/api/Decoder.hh"
#include "avrocpp/api/Decoder.hh"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Stream.hh"
#include "avrocpp/api/Stream.hh"
#include "avrocpp/api/ValidSchema.hh"
#include "avrocpp/api/ValidSchema.hh"
#include "backend/avro/encode_decode.h"
#include "backend/avro/raw_frame.h"
#include "backend/avro/types.h"

namespace {
using namespace RMF;
using namespace RMF::avro2;
template <class Tout, class Tin>
void validate_one(Tout fr, Tin) {
  fr.data.int_data[IntKey(0)][NodeID(0)] = 1;
  fr.data.float_data[FloatKey(1)][NodeID(1)] = 2.0;
  fr.data.string_data[StringKey(2)][NodeID(2)] = "hi";
  fr.data.ints_data[IntsKey(3)][NodeID(3)] = Ints(2, 3);
  fr.data.floats_data[FloatsKey(4)][NodeID(4)] = Floats(3, 2.0);
  fr.data.strings_data[StringsKey(5)][NodeID(5)] = Strings(4, "his");
  fr.data.vector3_data[Vector3Key(8)][NodeID(1)] = Vector3(2, 3, 4);
  fr.data.vector4_data[Vector4Key(8)][NodeID(1)] = Vector4(2, 3, 4, 5);
  fr.data.vector3s_data[Vector3sKey(9)][NodeID(1)] =
      Vector3s(1, Vector3(2, 3, 4));
  fr.keys.push_back(KeyInfo());
  fr.keys.back().id = 1;
  fr.keys.back().category = Category(0);
  fr.keys.back().type = RMF::avro2::INT;

  fr.nodes.push_back(HierarchyNode());
  fr.nodes.back().id = NodeID(1);
  fr.nodes.back().type = ROOT;
  fr.nodes.back().parents.push_back(NodeID(1));
  boost::shared_ptr<internal_avro::OutputStream> out_stream =
      internal_avro::memoryOutputStream();
  {
    internal_avro::EncoderPtr encoder = internal_avro::binaryEncoder();
    encoder->init(*out_stream);
    internal_avro::EncoderPtr ve = internal_avro::validatingEncoder(
        internal_avro::compileJsonSchemaFromString(data_avro::frame_json),
        encoder);
    internal_avro::encode(*ve, fr);
  }
  {
    boost::shared_ptr<internal_avro::InputStream> in_stream =
        internal_avro::memoryInputStream(*out_stream);
    internal_avro::DecoderPtr decoder = internal_avro::binaryDecoder();
    decoder->init(*in_stream);
    internal_avro::DecoderPtr ve = internal_avro::validatingDecoder(
        internal_avro::compileJsonSchemaFromString(data_avro::frame_json),
        decoder);
    Tin fd;
    internal_avro::decode(*ve, fd);
  }
}
template <class T>
void validate_raw(T fr) {
  boost::shared_ptr<internal_avro::OutputStream> out_stream =
      internal_avro::memoryOutputStream();
  {
    internal_avro::EncoderPtr encoder = internal_avro::binaryEncoder();
    encoder->init(*out_stream);
    internal_avro::EncoderPtr ve = internal_avro::validatingEncoder(
        internal_avro::compileJsonSchemaFromString(data_avro::frame_json),
        encoder);
    internal_avro::encode(*ve, fr);
  }
  {
    boost::shared_ptr<internal_avro::InputStream> in_stream =
        internal_avro::memoryInputStream(*out_stream);
    internal_avro::DecoderPtr decoder = internal_avro::binaryDecoder();
    decoder->init(*in_stream);
    internal_avro::DecoderPtr ve = internal_avro::validatingDecoder(
        internal_avro::compileJsonSchemaFromString(data_avro::frame_json),
        decoder);
    rmf_raw_avro2::Frame fd;
    internal_avro::decode(*ve, fd);
  }
}

void validate() {
  Frame fr;
  fr.parents.push_back(FrameID(2));
  fr.id = FrameID(1);
  validate_one(fr, Frame());
  FileDataChanges changes;
  changes.categories.push_back(std::make_pair(Category(0), "hi"));
  changes.node_types.push_back(std::make_pair(ROOT, "root"));
  changes.frame_types.push_back(std::make_pair(STATIC, "static"));
  changes.node_sets.push_back(std::make_pair(1, NodeIDs(2, NodeID(10))));
  FileData data;
  validate_one(changes, data);
  validate_raw(fr);
  validate_raw(changes);
}
}

int main(int, char * []) {
  validate();
  return 0;
}
