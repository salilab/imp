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

#include <fstream>

#include "cpx.hh"
#include "imaginary.hh"

#include "avro/Compiler.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "avro/Specific.hh"
#include "avro/Generic.hh"

internal_avro::ValidSchema load(const char* filename) {
  std::ifstream ifs(filename);
  internal_avro::ValidSchema result;
  internal_avro::compileJsonSchema(ifs, result);
  return result;
}

int main() {
  internal_avro::ValidSchema cpxSchema = load("cpx.json");
  internal_avro::ValidSchema imaginarySchema = load("imaginary.json");

  boost::shared_ptr<internal_avro::OutputStream> out =
      internal_avro::memoryOutputStream();
  internal_avro::EncoderPtr e = internal_avro::binaryEncoder();
  e->init(*out);
  c::cpx c1;
  c1.re = 100.23;
  c1.im = 105.77;
  internal_avro::encode(*e, c1);

  boost::shared_ptr<internal_avro::InputStream> in =
      internal_avro::memoryInputStream(*out);
  internal_avro::DecoderPtr d = internal_avro::resolvingDecoder(
      cpxSchema, imaginarySchema, internal_avro::binaryDecoder());
  d->init(*in);

  i::cpx c2;
  internal_avro::decode(*d, c2);
  std::cout << "Imaginary: " << c2.im << std::endl;
}
