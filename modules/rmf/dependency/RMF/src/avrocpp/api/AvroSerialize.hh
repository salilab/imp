/*
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

#ifndef avro_AvroSerialize_hh__
#define avro_AvroSerialize_hh__

#include "Config.hh"
#include "AvroTraits.hh"

/// \file
///
/// Standalone serialize functions for Avro types.

namespace internal_avro {

/// The main serializer entry point function.  Takes a serializer (either
/// validating or
/// plain) and the object that should be serialized.

template <typename Writer, typename T>
void serialize(Writer &s, const T &val) {
  serialize(s, val, is_serializable<T>());
}

/// Type trait should be set to is_serializable in otherwise force the compiler
/// to complain.

template <typename Writer, typename T>
void serialize(Writer &s, const T &val, const boost::false_type &) {
  static_assert(sizeof(T) == 0, "val should be empty");
}

/// The remainder of the file includes default implementations for serializable
/// types.

// @{

template <typename Writer, typename T>
void serialize(Writer &s, T val, const boost::true_type &) {
  s.writeValue(val);
}

template <typename Writer>
void serialize(Writer &s, const std::vector<uint8_t> &val,
               const boost::true_type &) {
  s.writeBytes(&val[0], val.size());
}

// @}

}  // namespace internal_avro

#endif
