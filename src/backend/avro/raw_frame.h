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

#ifndef SRC_BACKEND_AVRO2_RAW_FRAME_H_2514367941__H_
#define SRC_BACKEND_AVRO2_RAW_FRAME_H_2514367941__H_

#include "boost/any.hpp"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Decoder.hh"

namespace rmf_raw_avro2 {
struct FrameInfo {
  int32_t id;
  std::string name;
  int32_t type;
  std::vector<int32_t> parents;
};

struct Label {
  int32_t id;
  std::string name;
};

struct NodeSet {
  int32_t id;
  std::vector<int32_t> nodes;
};

struct FileInfo {
  std::string description;
  std::string producer;
  std::vector<Label> categories;
  std::vector<Label> node_types;
  std::vector<Label> frame_types;
  std::vector<NodeSet> node_sets;
};

struct _Frame_json_Union__0__ {
 private:
  size_t idx_;
  boost::any value_;

 public:
  size_t idx() const { return idx_; }
  FrameInfo get_FrameInfo() const;
  void set_FrameInfo(const FrameInfo& v);
  FileInfo get_FileInfo() const;
  void set_FileInfo(const FileInfo& v);
  _Frame_json_Union__0__();
};

struct Node {
  int32_t id;
  std::string name;
  int32_t type;
  std::vector<int32_t> parents;
};

enum Type {
  INT,
  FLOAT,
  STRING,
  INTS,
  FLOATS,
  STRINGS,
  VECTOR3,
  VECTOR4,
  VECTOR3S,
};

struct KeyInfo {
  int32_t id;
  std::string name;
  int32_t category;
  Type type;
};

struct IntValue {
  int32_t id;
  int32_t value;
};

struct IntNodeData {
  int32_t key;
  std::vector<IntValue> values;
};

struct FloatValue {
  int32_t id;
  float value;
};

struct FloatNodeData {
  int32_t key;
  std::vector<FloatValue> values;
};

struct StringValue {
  int32_t id;
  std::string value;
};

struct StringNodeData {
  int32_t key;
  std::vector<StringValue> values;
};

struct Vector3 {
  float x;
  float y;
  float z;
};

struct Vector3Value {
  int32_t id;
  Vector3 value;
};

struct Vector3NodeData {
  int32_t key;
  std::vector<Vector3Value> values;
};

struct Vector4 {
  float w;
  float x;
  float y;
  float z;
};

struct Vector4Value {
  int32_t id;
  Vector4 value;
};

struct Vector4NodeData {
  int32_t key;
  std::vector<Vector4Value> values;
};

struct IntsValue {
  int32_t key;
  std::vector<int32_t> value;
};

struct IntsNodeData {
  int32_t key;
  std::vector<IntsValue> values;
};

struct FloatsValue {
  int32_t id;
  std::vector<float> value;
};

struct FloatsNodeData {
  int32_t id;
  std::vector<FloatsValue> values;
};

struct StringsValue {
  int32_t id;
  std::vector<std::string> value;
};

struct StringsNodeData {
  int32_t id;
  std::vector<StringsValue> values;
};

struct Vector3sValue {
  int32_t id;
  std::vector<Vector3> value;
};

struct Vector3sNodeData {
  int32_t key;
  std::vector<Vector3sValue> values;
};

struct Vector4sValue {
  int32_t id;
  std::vector<Vector4> value;
};

struct Vector4sNodeData {
  int32_t key;
  std::vector<Vector4sValue> values;
};

struct TypeDatas {
  std::vector<IntNodeData> int_data;
  std::vector<FloatNodeData> float_data;
  std::vector<StringNodeData> string_data;
  std::vector<Vector3NodeData> vector3_data;
  std::vector<Vector4NodeData> vector4_data;
  std::vector<IntsNodeData> ints_data;
  std::vector<FloatsNodeData> floats_data;
  std::vector<StringsNodeData> strings_data;
  std::vector<Vector3sNodeData> vector3s_data;
  std::vector<Vector4sNodeData> vector4s_data;
};

struct Frame {
  typedef _Frame_json_Union__0__ info_t;
  info_t info;
  std::vector<Node> nodes;
  std::vector<KeyInfo> keys;
  TypeDatas data;
};

inline FrameInfo _Frame_json_Union__0__::get_FrameInfo() const {
  if (idx_ != 0) {
    throw internal_avro::Exception("Invalid type for union");
  }
  return boost::any_cast<FrameInfo>(value_);
}

inline void _Frame_json_Union__0__::set_FrameInfo(const FrameInfo& v) {
  idx_ = 0;
  value_ = v;
}

inline FileInfo _Frame_json_Union__0__::get_FileInfo() const {
  if (idx_ != 1) {
    throw internal_avro::Exception("Invalid type for union");
  }
  return boost::any_cast<FileInfo>(value_);
}

inline void _Frame_json_Union__0__::set_FileInfo(const FileInfo& v) {
  idx_ = 1;
  value_ = v;
}

inline _Frame_json_Union__0__::_Frame_json_Union__0__()
    : idx_(0), value_(FrameInfo()) {}
}
namespace internal_avro {
template <>
struct codec_traits<rmf_raw_avro2::FrameInfo> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::FrameInfo& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.type);
    internal_avro::encode(e, v.parents);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::FrameInfo& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.type);
    internal_avro::decode(d, v.parents);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Label> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Label& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Label& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.name);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::NodeSet> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::NodeSet& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.nodes);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::NodeSet& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.nodes);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::FileInfo> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::FileInfo& v) {
    internal_avro::encode(e, v.description);
    internal_avro::encode(e, v.producer);
    internal_avro::encode(e, v.categories);
    internal_avro::encode(e, v.node_types);
    internal_avro::encode(e, v.frame_types);
    internal_avro::encode(e, v.node_sets);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::FileInfo& v) {
    internal_avro::decode(d, v.description);
    internal_avro::decode(d, v.producer);
    internal_avro::decode(d, v.categories);
    internal_avro::decode(d, v.node_types);
    internal_avro::decode(d, v.frame_types);
    internal_avro::decode(d, v.node_sets);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::_Frame_json_Union__0__> {
  template <class Encoder>
  static void encode(Encoder& e, rmf_raw_avro2::_Frame_json_Union__0__ v) {
    e.encodeUnionIndex(v.idx());
    switch (v.idx()) {
      case 0:
        internal_avro::encode(e, v.get_FrameInfo());
        break;
      case 1:
        internal_avro::encode(e, v.get_FileInfo());
        break;
    }
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::_Frame_json_Union__0__& v) {
    size_t n = d.decodeUnionIndex();
    if (n >= 2) {
      throw internal_avro::Exception("Union index too big");
    }
    switch (n) {
      case 0: {
        rmf_raw_avro2::FrameInfo vv;
        internal_avro::decode(d, vv);
        v.set_FrameInfo(vv);
      } break;
      case 1: {
        rmf_raw_avro2::FileInfo vv;
        internal_avro::decode(d, vv);
        v.set_FileInfo(vv);
      } break;
    }
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Node> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Node& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.type);
    internal_avro::encode(e, v.parents);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Node& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.type);
    internal_avro::decode(d, v.parents);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Type> {
  template <class Encoder>
  static void encode(Encoder& e, rmf_raw_avro2::Type v) {
    e.encodeEnum(v);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Type& v) {
    v = static_cast<rmf_raw_avro2::Type>(d.decodeEnum());
  }
};

template <>
struct codec_traits<rmf_raw_avro2::KeyInfo> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::KeyInfo& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.name);
    internal_avro::encode(e, v.category);
    internal_avro::encode(e, v.type);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::KeyInfo& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.name);
    internal_avro::decode(d, v.category);
    internal_avro::decode(d, v.type);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::IntValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::IntValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::IntValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::IntNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::IntNodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::IntNodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::FloatValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::FloatValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::FloatValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::FloatNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::FloatNodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::FloatNodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::StringValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::StringValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::StringValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::StringNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::StringNodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::StringNodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector3> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector3& v) {
    internal_avro::encode(e, v.x);
    internal_avro::encode(e, v.y);
    internal_avro::encode(e, v.z);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector3& v) {
    internal_avro::decode(d, v.x);
    internal_avro::decode(d, v.y);
    internal_avro::decode(d, v.z);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector3Value> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector3Value& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector3Value& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector3NodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector3NodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector3NodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector4> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector4& v) {
    internal_avro::encode(e, v.w);
    internal_avro::encode(e, v.x);
    internal_avro::encode(e, v.y);
    internal_avro::encode(e, v.z);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector4& v) {
    internal_avro::decode(d, v.w);
    internal_avro::decode(d, v.x);
    internal_avro::decode(d, v.y);
    internal_avro::decode(d, v.z);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector4Value> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector4Value& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector4Value& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector4NodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector4NodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector4NodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::IntsValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::IntsValue& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::IntsValue& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::IntsNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::IntsNodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::IntsNodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::FloatsValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::FloatsValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::FloatsValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::FloatsNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::FloatsNodeData& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::FloatsNodeData& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::StringsValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::StringsValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::StringsValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::StringsNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::StringsNodeData& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::StringsNodeData& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector3sValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector3sValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector3sValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector3sNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector3sNodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector3sNodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector4sValue> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector4sValue& v) {
    internal_avro::encode(e, v.id);
    internal_avro::encode(e, v.value);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector4sValue& v) {
    internal_avro::decode(d, v.id);
    internal_avro::decode(d, v.value);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Vector4sNodeData> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Vector4sNodeData& v) {
    internal_avro::encode(e, v.key);
    internal_avro::encode(e, v.values);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Vector4sNodeData& v) {
    internal_avro::decode(d, v.key);
    internal_avro::decode(d, v.values);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::TypeDatas> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::TypeDatas& v) {
    internal_avro::encode(e, v.int_data);
    internal_avro::encode(e, v.float_data);
    internal_avro::encode(e, v.string_data);
    internal_avro::encode(e, v.vector3_data);
    internal_avro::encode(e, v.vector4_data);
    internal_avro::encode(e, v.ints_data);
    internal_avro::encode(e, v.floats_data);
    internal_avro::encode(e, v.strings_data);
    internal_avro::encode(e, v.vector3s_data);
    internal_avro::encode(e, v.vector4s_data);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::TypeDatas& v) {
    internal_avro::decode(d, v.int_data);
    internal_avro::decode(d, v.float_data);
    internal_avro::decode(d, v.string_data);
    internal_avro::decode(d, v.vector3_data);
    internal_avro::decode(d, v.vector4_data);
    internal_avro::decode(d, v.ints_data);
    internal_avro::decode(d, v.floats_data);
    internal_avro::decode(d, v.strings_data);
    internal_avro::decode(d, v.vector3s_data);
    internal_avro::decode(d, v.vector4s_data);
  }
};

template <>
struct codec_traits<rmf_raw_avro2::Frame> {
  template <class Encoder>
  static void encode(Encoder& e, const rmf_raw_avro2::Frame& v) {
    internal_avro::encode(e, v.info);
    internal_avro::encode(e, v.nodes);
    internal_avro::encode(e, v.keys);
    internal_avro::encode(e, v.data);
  }
  template <class Decoder>
  static void decode(Decoder& d, rmf_raw_avro2::Frame& v) {
    internal_avro::decode(d, v.info);
    internal_avro::decode(d, v.nodes);
    internal_avro::decode(d, v.keys);
    internal_avro::decode(d, v.data);
  }
};
}
#endif
