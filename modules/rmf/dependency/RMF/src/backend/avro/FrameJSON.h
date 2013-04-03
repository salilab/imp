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


#ifndef _USR_LOCAL_GOOGLE_HOME_DRUSSEL_SRC_IMP_DEBUG_SRC_DEPENDENCY_RMF_SRC_BACKEND_AVRO_FRAMEJSON_H_779595758__H_
#define _USR_LOCAL_GOOGLE_HOME_DRUSSEL_SRC_IMP_DEBUG_SRC_DEPENDENCY_RMF_SRC_BACKEND_AVRO_FRAMEJSON_H_779595758__H_


#include "boost/any.hpp"
#include "AvroCpp/api/Specific.hh"
#include "AvroCpp/api/Encoder.hh"
#include "AvroCpp/api/Decoder.hh"

namespace RMF_avro_backend {
struct Frame {
    int32_t index;
    std::string name;
    std::string type;
    std::vector<int32_t > parents;
};

}
namespace rmf_avro {
template<> struct codec_traits<RMF_avro_backend::Frame> {
    static void encode(Encoder& e, const RMF_avro_backend::Frame& v) {
        rmf_avro::encode(e, v.index);
        rmf_avro::encode(e, v.name);
        rmf_avro::encode(e, v.type);
        rmf_avro::encode(e, v.parents);
    }
    static void decode(Decoder& d, RMF_avro_backend::Frame& v) {
        rmf_avro::decode(d, v.index);
        rmf_avro::decode(d, v.name);
        rmf_avro::decode(d, v.type);
        rmf_avro::decode(d, v.parents);
    }
};

}
#endif
