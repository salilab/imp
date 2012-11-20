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

template <class AvroType>
bool show_type(std::string node_name,
               bool shown,
               const std::map<std::string, AvroType > &data) {
  if (data.empty()) return shown;
  std::cout << "  node: " << node_name << std::endl;
  for (typename std::map<std::string, AvroType >::const_iterator
         it = data.begin(); it != data.end(); ++it) {
    std::cout << "    " << it->first << ": " << RMF::Showable(it->second)
              << std::endl;
  }
  return true;
}

#define RMF_SHOW_TYPE(lcname, Ucname, PassValue, ReturnValue,    \
                      PassValues, ReturnValues)                  \
  shown= show_type(it->first, shown, it->second.lcname##_data)

int main(int argc, char **argv) {
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
      std::cout << "frame: " << data.frame << std::endl;
      for (std::map<std::string, RMF_internal::NodeData >::const_iterator it
             = data.nodes.begin(); it != data.nodes.end(); ++it) {
        bool shown=false;
        RMF_FOREACH_TYPE(RMF_SHOW_TYPE);
      }
    } while (true);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
