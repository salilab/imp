/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <RMF/FileHandle.h>
#include <RMF/utility.h>
#include "common.h"
#include <backend/avro/AvroCpp/api/Compiler.hh>
#include <backend/avro/AvroCpp/api/DataFile.hh>
#include <backend/avro/avro_schemas.h>
#include <backend/avro/AllJSON.h>
#include <backend/avro/FrameJSON.h>

namespace {
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
template <class Type>
bool try_read(std::string type, std::string input,
              rmf_avro::ValidSchema schema, bool count) {
  std::cout << "Trying " << type << std::endl;
  boost::shared_ptr<rmf_avro::Encoder> encoder
      = rmf_avro::jsonEncoder(RMF::avro_backend::get_Data_schema());
  std::auto_ptr<rmf_avro::OutputStream> stream
      = rmf_avro::ostreamOutputStream(std::cout);
  encoder->init(*stream);
  try {
    rmf_avro::DataFileReader<Type > reader(input.c_str(), schema);
    Type data;
    bool ok=false;
    int frame=0;
    do {
      try {
        // a bit silly
        if (!reader.read(data)) break;
      } catch (const std::exception &e) {
        break;
      }
      ok=true;
      std::cout << "frame: " << frame << std::endl;
      if (!count) {
        rmf_avro::encode(*encoder, data);
        encoder->flush();
        stream->flush();
      }
      ++frame;
    } while (true);
    return ok;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return false;
  }
  return true;
}
}

#define RMF_SHOW_TYPE(lcname, Ucname, PassValue, ReturnValue,      \
                      PassValues, ReturnValues)                    \
  shown = show_type<RMF::Ucname##Traits>(it->first, shown,         \
                                         it->second.lcname##_data, \
                                         data.index.lcname##_index)



int main(int argc, char **argv) {
  RMF_ADD_INPUT_FILE("data");
  options.add_options()("count,c", "Just count the frames.");
  process_options(argc, argv);
  bool count= variables_map.count("count");
  if (try_read<RMF_avro_backend::Data>("data", input,
                                   RMF::avro_backend::get_Data_schema(),
                                   count)) {
    return 0;
  } else if (try_read<RMF_avro_backend::File>("file", input,
                                              RMF::avro_backend::get_File_schema(),
                                              count)) {
    return 0;
  } else if (try_read<RMF_avro_backend::Node>("node", input,
                                              RMF::avro_backend::get_Nodes_schema(),
                                              count)) {
    return 0;
  } else if (try_read<RMF_avro_backend::All>("all", input,
                                             RMF::avro_backend::get_All_schema(),
                                             count)) {
    return 0;
  } else if (try_read<RMF_avro_backend::Frame>("frame", input,
                                               RMF::avro_backend::get_Frame_schema(),
                                               count)) {
    return 0;
  }
  return 1;
}
