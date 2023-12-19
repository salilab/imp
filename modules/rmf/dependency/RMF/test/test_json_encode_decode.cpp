#include <cassert>
#include <avrocpp/api/Compiler.hh>
#include <avrocpp/api/Decoder.hh>
#include <avrocpp/api/Encoder.hh>
#include <avrocpp/api/Specific.hh>
#include <avrocpp/api/Stream.hh>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "avrocpp/api/ValidSchema.hh"

namespace {
const char* schema = "{\"type\" : \"array\", \"items\" : \"double\"}";
::internal_avro::ValidSchema get_valid_schema() {
  return ::internal_avro::compileJsonSchemaFromString(schema);
}
std::string encode(std::vector<double> data) {
  std::shared_ptr<internal_avro::Encoder> encoder =
      internal_avro::jsonEncoder(get_valid_schema());
  std::ostringstream oss;
  std::shared_ptr<internal_avro::OutputStream> stream =
      internal_avro::ostreamOutputStream(oss);
  encoder->init(*stream);
  internal_avro::encode(*encoder, data);
  encoder->flush();
  stream->flush();
  return oss.str();
}
std::vector<double> decode(std::string buffer) {
  std::shared_ptr<internal_avro::Decoder> decoder =
      internal_avro::jsonDecoder(get_valid_schema());
  std::istringstream iss(buffer);
  std::shared_ptr<internal_avro::InputStream> stream =
      internal_avro::istreamInputStream(iss);
  decoder->init(*stream);
  std::vector<double> data;
  internal_avro::decode(*decoder, data);
  return data;
}
}

int main(int, char * []) {
  {
    std::vector<double> v;
    v.push_back(1.5);
    v.push_back(2.5);
    std::string encoded = encode(v);
    std::cout << "buffer is \"" << encoded << "\"" << std::endl;
    std::vector<double> vv = decode(encoded);
    assert(v.size() == vv.size());
    for (unsigned int i = 0; i < v.size(); ++i) {
      assert(v[i] == vv[i]);
    }
  }
  {
    std::vector<double> v;
    v.push_back(1.0);
    v.push_back(2.0);
    std::string encoded = encode(v);
    std::cout << "buffer is \"" << encoded << "\"" << std::endl;
    std::vector<double> vv = decode(encoded);
    assert(v.size() == vv.size());
    for (unsigned int i = 0; i < v.size(); ++i) {
      assert(v[i] == vv[i]);
    }
  }
  return 0;
}
