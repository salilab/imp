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
#include "AvroCpp/api/Specific.hh"
#include "AvroCpp/api/Encoder.hh"
#include "AvroCpp/api/Decoder.hh"

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
namespace rmf_avro {
template<> struct codec_traits<RMF_avro_backend::File> {
    static void encode(Encoder& e, const RMF_avro_backend::File& v) {
        rmf_avro::encode(e, v.description);
        rmf_avro::encode(e, v.producer);
        rmf_avro::encode(e, v.version);
    }
    static void decode(Decoder& d, RMF_avro_backend::File& v) {
        rmf_avro::decode(d, v.description);
        rmf_avro::decode(d, v.producer);
        rmf_avro::decode(d, v.version);
    }
};

template<> struct codec_traits<RMF_avro_backend::Node> {
    static void encode(Encoder& e, const RMF_avro_backend::Node& v) {
        rmf_avro::encode(e, v.name);
        rmf_avro::encode(e, v.type);
        rmf_avro::encode(e, v.children);
    }
    static void decode(Decoder& d, RMF_avro_backend::Node& v) {
        rmf_avro::decode(d, v.name);
        rmf_avro::decode(d, v.type);
        rmf_avro::decode(d, v.children);
    }
};

template<> struct codec_traits<RMF_avro_backend::IntData> {
    static void encode(Encoder& e, const RMF_avro_backend::IntData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IntData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::FloatData> {
    static void encode(Encoder& e, const RMF_avro_backend::FloatData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::FloatData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::StringData> {
    static void encode(Encoder& e, const RMF_avro_backend::StringData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::StringData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::IndexData> {
    static void encode(Encoder& e, const RMF_avro_backend::IndexData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IndexData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::NodeIDData> {
    static void encode(Encoder& e, const RMF_avro_backend::NodeIDData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::NodeIDData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::IntsData> {
    static void encode(Encoder& e, const RMF_avro_backend::IntsData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IntsData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::FloatsData> {
    static void encode(Encoder& e, const RMF_avro_backend::FloatsData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::FloatsData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::StringsData> {
    static void encode(Encoder& e, const RMF_avro_backend::StringsData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::StringsData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::IndexesData> {
    static void encode(Encoder& e, const RMF_avro_backend::IndexesData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::IndexesData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::NodeIDsData> {
    static void encode(Encoder& e, const RMF_avro_backend::NodeIDsData& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.nodes);
    }
    static void decode(Decoder& d, RMF_avro_backend::NodeIDsData& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.nodes);
    }
};

template<> struct codec_traits<RMF_avro_backend::Data> {
    static void encode(Encoder& e, const RMF_avro_backend::Data& v) {
        rmf_avro::encode(e, v.frame);
        rmf_avro::encode(e, v.int_data);
        rmf_avro::encode(e, v.float_data);
        rmf_avro::encode(e, v.string_data);
        rmf_avro::encode(e, v.index_data);
        rmf_avro::encode(e, v.node_id_data);
        rmf_avro::encode(e, v.ints_data);
        rmf_avro::encode(e, v.floats_data);
        rmf_avro::encode(e, v.strings_data);
        rmf_avro::encode(e, v.indexes_data);
        rmf_avro::encode(e, v.node_ids_data);
    }
    static void decode(Decoder& d, RMF_avro_backend::Data& v) {
        rmf_avro::decode(d, v.frame);
        rmf_avro::decode(d, v.int_data);
        rmf_avro::decode(d, v.float_data);
        rmf_avro::decode(d, v.string_data);
        rmf_avro::decode(d, v.index_data);
        rmf_avro::decode(d, v.node_id_data);
        rmf_avro::decode(d, v.ints_data);
        rmf_avro::decode(d, v.floats_data);
        rmf_avro::decode(d, v.strings_data);
        rmf_avro::decode(d, v.indexes_data);
        rmf_avro::decode(d, v.node_ids_data);
    }
};

template<> struct codec_traits<RMF_avro_backend::All> {
    static void encode(Encoder& e, const RMF_avro_backend::All& v) {
        rmf_avro::encode(e, v.file);
        rmf_avro::encode(e, v.nodes);
        rmf_avro::encode(e, v.frames);
        rmf_avro::encode(e, v.category);
    }
    static void decode(Decoder& d, RMF_avro_backend::All& v) {
        rmf_avro::decode(d, v.file);
        rmf_avro::decode(d, v.nodes);
        rmf_avro::decode(d, v.frames);
        rmf_avro::decode(d, v.category);
    }
};

}
#endif
