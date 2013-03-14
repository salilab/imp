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


#ifndef _USR_LOCAL_GOOGLE_HOME_DRUSSEL_SRC_IMP_DEBUG_SRC_DEPENDENCY_RMF_SRC_BACKEND_AVRO_ALLJSON_H_779595758__H_
#define _USR_LOCAL_GOOGLE_HOME_DRUSSEL_SRC_IMP_DEBUG_SRC_DEPENDENCY_RMF_SRC_BACKEND_AVRO_ALLJSON_H_779595758__H_


#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

namespace RMF_avro_backend {
struct File {
    std::string description;
    std::string producer;
    int32_t version;
};

struct Node {
    std::string name;
    std::string type;
    std::vector<int32_t > children;
};

struct IntData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<int32_t > > nodes;
};

struct FloatData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<double > > nodes;
};

struct StringData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<std::string > > nodes;
};

struct IndexData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<int32_t > > nodes;
};

struct NodeIDData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<int32_t > > nodes;
};

struct IntsData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<std::vector<int32_t > > > nodes;
};

struct FloatsData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<std::vector<double > > > nodes;
};

struct StringsData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<std::vector<std::string > > > nodes;
};

struct IndexesData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<std::vector<int32_t > > > nodes;
};

struct NodeIDsData {
    std::map<std::string, int32_t > index;
    std::map<std::string, std::vector<std::vector<int32_t > > > nodes;
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
    std::vector<Node > nodes;
    std::vector<Node > frames;
    std::map<std::string, std::vector<Data > > category;
};

}
namespace avro {
template<> struct codec_traits<RMF_avro_backend::File> {
    static void encode(Encoder& e, const RMF_avro_backend::File& v) {
        avro::encode(e, v.description);
        avro::encode(e, v.producer);
        avro::encode(e, v.version);
    }
    static void decode(Decoder& d, RMF_avro_backend::File& v) {
        avro::decode(d, v.description);
        avro::decode(d, v.producer);
        avro::decode(d, v.version);
    }
};

template<> struct codec_traits<RMF_avro_backend::Node> {
    static void encode(Encoder& e, const RMF_avro_backend::Node& v) {
        avro::encode(e, v.name);
        avro::encode(e, v.type);
        avro::encode(e, v.children);
    }
    static void decode(Decoder& d, RMF_avro_backend::Node& v) {
        avro::decode(d, v.name);
        avro::decode(d, v.type);
        avro::decode(d, v.children);
    }
};

template<> struct codec_traits<RMF_avro_backend::IntData> {
    static void encode(Encoder& e, const RMF_avro_backend::IntData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IntData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::FloatData> {
    static void encode(Encoder& e, const RMF_avro_backend::FloatData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::FloatData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::StringData> {
    static void encode(Encoder& e, const RMF_avro_backend::StringData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::StringData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::IndexData> {
    static void encode(Encoder& e, const RMF_avro_backend::IndexData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IndexData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::NodeIDData> {
    static void encode(Encoder& e, const RMF_avro_backend::NodeIDData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::NodeIDData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::IntsData> {
    static void encode(Encoder& e, const RMF_avro_backend::IntsData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IntsData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::FloatsData> {
    static void encode(Encoder& e, const RMF_avro_backend::FloatsData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::FloatsData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::StringsData> {
    static void encode(Encoder& e, const RMF_avro_backend::StringsData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::StringsData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::IndexesData> {
    static void encode(Encoder& e, const RMF_avro_backend::IndexesData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IndexesData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::NodeIDsData> {
    static void encode(Encoder& e, const RMF_avro_backend::NodeIDsData& v) {
        avro::encode(e, v.index);
        avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::NodeIDsData& v) {
        avro::decode(d, v.index);
        avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::Data> {
    static void encode(Encoder& e, const RMF_avro_backend::Data& v) {
        avro::encode(e, v.frame);
        avro::encode(e, v.int_data);
        avro::encode(e, v.float_data);
        avro::encode(e, v.string_data);
        avro::encode(e, v.index_data);
        avro::encode(e, v.node_id_data);
        avro::encode(e, v.ints_data);
        avro::encode(e, v.floats_data);
        avro::encode(e, v.strings_data);
        avro::encode(e, v.indexes_data);
        avro::encode(e, v.node_ids_data);
    }
    static void decode(Decoder& d, RMF_avro_backend::Data& v) {
        avro::decode(d, v.frame);
        avro::decode(d, v.int_data);
        avro::decode(d, v.float_data);
        avro::decode(d, v.string_data);
        avro::decode(d, v.index_data);
        avro::decode(d, v.node_id_data);
        avro::decode(d, v.ints_data);
        avro::decode(d, v.floats_data);
        avro::decode(d, v.strings_data);
        avro::decode(d, v.indexes_data);
        avro::decode(d, v.node_ids_data);
    }
};

template<> struct codec_traits<RMF_avro_backend::All> {
    static void encode(Encoder& e, const RMF_avro_backend::All& v) {
        avro::encode(e, v.file);
        avro::encode(e, v.nodes);
        avro::encode(e, v.frames);
        avro::encode(e, v.category);
    }
    static void decode(Decoder& d, RMF_avro_backend::All& v) {
        avro::decode(d, v.file);
        avro::decode(d, v.nodes);
        avro::decode(d, v.frames);
        avro::decode(d, v.category);
    }
};

}
#endif
