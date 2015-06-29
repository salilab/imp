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
#include <complex>

#include "cpx.hh"

#include "avro/Compiler.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "avro/Specific.hh"
#include "avro/Generic.hh"

int main() {
  std::ifstream ifs("cpx.json");

  internal_avro::ValidSchema cpxSchema;
  internal_avro::compileJsonSchema(ifs, cpxSchema);

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
  internal_avro::DecoderPtr d = internal_avro::binaryDecoder();
  d->init(*in);

  internal_avro::GenericDatum datum(cpxSchema);
  internal_avro::decode(*d, datum);
  std::cout << "Type: " << datum.type() << std::endl;
  if (datum.type() == internal_avro::AVRO_RECORD) {
    const internal_avro::GenericRecord& r =
        datum.value<internal_avro::GenericRecord>();
    std::cout << "Field-count: " << r.fieldCount() << std::endl;
    if (r.fieldCount() == 2) {
      const internal_avro::GenericDatum& f0 = r.fieldAt(0);
      if (f0.type() == internal_avro::AVRO_DOUBLE) {
        std::cout << "Real: " << f0.value<double>() << std::endl;
      }
      const internal_avro::GenericDatum& f1 = r.fieldAt(1);
      if (f1.type() == internal_avro::AVRO_DOUBLE) {
        std::cout << "Imaginary: " << f1.value<double>() << std::endl;
      }
    }
  }
  return 0;
}
