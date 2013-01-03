/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <RMF/FileHandle.h>
#include <RMF/utility.h>
#include "common.h"
#include <avro/Compiler.hh>
#include <avro/DataFile.hh>
#include <backend/avro/avro_schemas.h>
#include <backend/avro/AllJSON.h>

std::string
  description("Dump the data from an avro archive with frame data");

template <class TypeTraits>
bool show_type(std::string node_name,
               bool shown,
               const std::vector<typename TypeTraits::AvroType > &data,
               const std::map<std::string, int> &index) {
  if (data.empty()) return shown;
  std::cout << "  node: " << node_name << std::endl;
  for (unsigned int i = 0; i < data.size(); ++i) {

    if (!TypeTraits::get_is_null_value(data[i])) {
      for (typename std::map<std::string, int>::const_iterator it = index.begin();
           it != index.end(); ++it) {
        if (it->second == i) {
          std::cout << "    " << it->first << ": " << RMF::Showable(data[i])
                    << std::endl;
        }
      }
    }
  }
  return true;
}

#define RMF_SHOW_TYPE(lcname, Ucname, PassValue, ReturnValue,      \
                      PassValues, ReturnValues)                    \
  shown = show_type<RMF::Ucname##Traits>(it->first, shown,         \
                                         it->second.lcname##_data, \
                                         data.index.lcname##_index)

int main(int argc, char **argv) {
  boost::shared_ptr<avro::Encoder> encoder = avro::jsonEncoder(RMF::internal::get_Data_schema());
  std::auto_ptr<avro::OutputStream> stream = avro::ostreamOutputStream(std::cout);
  encoder->init(*stream);
  try {
    RMF_ADD_INPUT_FILE("data");
    process_options(argc, argv);
    avro::DataFileReader<RMF_internal::Data >
    reader(input.c_str(),
           RMF::internal::get_Data_schema());
    RMF_internal::Data data;
    do {
      try {
        // a bit silly
        if (!reader.read(data)) break;
      } catch (const std::exception &e) {
        break;
      }
      avro::encode(*encoder, data);
    } while (true);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
