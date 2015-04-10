/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RMF_SRC_BACKEND_AVRO_ALLJSON_H_779595758__H_
#define RMF_SRC_BACKEND_AVRO_ALLJSON_H_779595758__H_

#include <boost/any.hpp>
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Decoder.hh"

namespace RMF_avro_backend {
struct File {
  std::string description;
  std::string producer;
  int32_t version;
};

struct Node {
  std::string name;
  std::string type;
  std::vector<int32_t> children;
};

struct IntData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<int32_t> > nodes;
};

struct FloatData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<double> > nodes;
};

struct StringData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<std::string> > nodes;
};

struct IndexData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<int32_t> > nodes;
};

struct NodeIDData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<int32_t> > nodes;
};

struct IntsData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<std::vector<int32_t> > > nodes;
};

struct FloatsData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<std::vector<double> > > nodes;
};

struct StringsData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<std::vector<std::string> > > nodes;
};

struct IndexesData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<std::vector<int32_t> > > nodes;
};

struct NodeIDsData {
  std::map<std::string, int32_t> index;
  std::map<std::string, std::vector<std::vector<int32_t> > > nodes;
};

struct Data {
  int32_t frame;
  IntData int_data;
  FloatData float_data;
  StringData string_data;
  IndexData index_data;
  NodeIDData node_id_data;
  IntsData ints_data;
  FloatsData floats_data;
  StringsData strings_data;
  IndexesData indexes_data;
  NodeIDsData node_ids_data;
};

struct All {
  File file;
  std::vector<Node> nodes;
  std::vector<Node> frames;
  std::map<std::string, std::vector<Data> > category;
};
}
namespace internal_avro {
template <>
struct codec_traits<RMF_avro_backend::File> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::File& v) {
    internal_avro::encode(e, v.description);
    internal_avro::encode(e, v.producer);
    internal_avro::encode(e, v.version);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::File& v) {
    internal_avro::decode(d, v.description);
    internal_avro::decode(d, v.producer);
    internal_avro::decode(d, v.version);
  }
};

template <>
struct codec_traits<RMF_avro_backend::Node> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::Node& v) {
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.type);
    internal_avro::encode(e, v.children);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::Node& v) {
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.type);
    internal_avro::decode(d, v.children);
  }
};

template <>
struct codec_traits<RMF_avro_backend::IntData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::IntData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::IntData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::FloatData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::FloatData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::FloatData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::StringData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::StringData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::StringData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::IndexData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::IndexData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::IndexData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::NodeIDData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::NodeIDData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::NodeIDData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::IntsData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::IntsData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::IntsData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::FloatsData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::FloatsData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::FloatsData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::StringsData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::StringsData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::StringsData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::IndexesData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::IndexesData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::IndexesData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::NodeIDsData> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::NodeIDsData& v) {
    internal_avro::encode(e, v.index);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::NodeIDsData& v) {
    internal_avro::decode(d, v.index);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<RMF_avro_backend::Data> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::Data& v) {
    internal_avro::encode(e, v.frame);
    internal_avro::encode(e, v.int_data);
    internal_avro::encode(e, v.float_data);
    internal_avro::encode(e, v.string_data);
    internal_avro::encode(e, v.index_data);
    internal_avro::encode(e, v.node_id_data);
    internal_avro::encode(e, v.ints_data);
    internal_avro::encode(e, v.floats_data);
    internal_avro::encode(e, v.strings_data);
    internal_avro::encode(e, v.indexes_data);
    internal_avro::encode(e, v.node_ids_data);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::Data& v) {
    internal_avro::decode(d, v.frame);
    internal_avro::decode(d, v.int_data);
    internal_avro::decode(d, v.float_data);
    internal_avro::decode(d, v.string_data);
    internal_avro::decode(d, v.index_data);
    internal_avro::decode(d, v.node_id_data);
    internal_avro::decode(d, v.ints_data);
    internal_avro::decode(d, v.floats_data);
    internal_avro::decode(d, v.strings_data);
    internal_avro::decode(d, v.indexes_data);
    internal_avro::decode(d, v.node_ids_data);
  }
};

template <>
struct codec_traits<RMF_avro_backend::All> {
  template <class Encoder>
  static void encode(Encoder& e, const RMF_avro_backend::All& v) {
    internal_avro::encode(e, v.file);
    internal_avro::encode(e, v.nodes);
    internal_avro::encode(e, v.frames);
    internal_avro::encode(e, v.category);
  }
  template <class Decoder>
  static void decode(Decoder& d, RMF_avro_backend::All& v) {
    internal_avro::decode(d, v.file);
    internal_avro::decode(d, v.nodes);
    internal_avro::decode(d, v.frames);
    internal_avro::decode(d, v.category);
  }
};
}
#endif
