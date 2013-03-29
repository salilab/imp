#include <avro/Compiler.hh>
#include <avro/Stream.hh>
#include <avro/Encoder.hh>
#include <avro/Decoder.hh>
#include <avro/Specific.hh>
#include <sstream>

namespace {
  const char * schema = "{\"type\" : \"array\", \"items\" : \"double\"}";
  ::avro::ValidSchema get_valid_schema() {
    return ::avro::compileJsonSchemaFromString(schema);
  }
  std::string encode(std::vector<double> data) {
    boost::shared_ptr<avro::Encoder> encoder
      = avro::jsonEncoder(get_valid_schema());
    std::ostringstream oss;
    std::auto_ptr<avro::OutputStream> stream
      = avro::ostreamOutputStream(oss);
    encoder->init(*stream);
    avro::encode(*encoder, data);
    encoder->flush();
    stream->flush();
    return oss.str();
  }
  std::vector<double> decode(std::string buffer) {
    boost::shared_ptr<avro::Decoder> decoder
      = avro::jsonDecoder(get_valid_schema());
    std::istringstream iss(buffer);
    std::auto_ptr<avro::InputStream> stream
      = avro::istreamInputStream(iss);
    decoder->init(*stream);
    std::vector<double> data;
    avro::decode(*decoder, data);
    return data;
  }
}

int main(int, char*[]) {
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
