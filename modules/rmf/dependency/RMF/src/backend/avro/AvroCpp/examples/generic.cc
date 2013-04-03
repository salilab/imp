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

int
main()
{
    std::ifstream ifs("cpx.json");

    rmf_avro::ValidSchema cpxSchema;
    rmf_avro::compileJsonSchema(ifs, cpxSchema);

    std::auto_ptr<rmf_avro::OutputStream> out = rmf_avro::memoryOutputStream();
    rmf_avro::EncoderPtr e = rmf_avro::binaryEncoder();
    e->init(*out);
    c::cpx c1;
    c1.re = 100.23;
    c1.im = 105.77;
    rmf_avro::encode(*e, c1);

    std::auto_ptr<rmf_avro::InputStream> in = rmf_avro::memoryInputStream(*out);
    rmf_avro::DecoderPtr d = rmf_avro::binaryDecoder();
    d->init(*in);

    rmf_avro::GenericDatum datum(cpxSchema);
    rmf_avro::decode(*d, datum);
    std::cout << "Type: " << datum.type() << std::endl;
    if (datum.type() == rmf_avro::AVRO_RECORD) {
        const rmf_avro::GenericRecord& r = datum.value<rmf_avro::GenericRecord>();
        std::cout << "Field-count: " << r.fieldCount() << std::endl;
        if (r.fieldCount() == 2) {
            const rmf_avro::GenericDatum& f0 = r.fieldAt(0);
            if (f0.type() == rmf_avro::AVRO_DOUBLE) {
                std::cout << "Real: " << f0.value<double>() << std::endl;
            }
            const rmf_avro::GenericDatum& f1 = r.fieldAt(1);
            if (f1.type() == rmf_avro::AVRO_DOUBLE) {
                std::cout << "Imaginary: " << f1.value<double>() << std::endl;
            }
        }
    }
    return 0;
}
