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

#ifndef rmf_avro_Config_hh
#define rmf_avro_Config_hh

// Windows DLL suport

#ifdef _WIN32
#pragma warning (disable: 4275 4251)

#if defined(AVRO_DYN_LINK)
#ifdef AVRO_SOURCE
# define AVRO_DECL __declspec(dllexport)
#else
# define AVRO_DECL __declspec(dllimport)
#endif  // AVRO_SOURCE
#endif  // AVRO_DYN_LINK
#endif  // _WIN32

#ifndef AVRO_DECL
#define AVRO_DECL
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1500
#include <boost/cstdint.hpp>

using boost::int64_t;
using boost::uint64_t;
using boost::int32_t;
using boost::uint32_t;
using boost::int8_t;
using boost::uint8_t;

#define INT32_MIN _I32_MIN
#define INT32_MAX _I32_MAX

#else // _MSC_VER
#include <stdint.h>
#endif // _MSC_VER

#endif
