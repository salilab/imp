/**
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#include <boost/shared_ptr.hpp>
#include <stddef.h>
#include <exception>
#include <iostream>
#include <string>

#include "avrocpp/api/Compiler.hh"
#include "avrocpp/api/DataFile.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Stream.hh"
#include "avrocpp/api/ValidSchema.hh"
#include "backend/avro/raw_frame.h"
#include "common.h"
#include "generated/embed_jsons.h"

namespace {
std::string description("Dump frames from an rmf3 file.");
}

int main(int argc, char** argv) {
  try {
    RMF_ADD_INPUT_FILE("rmf");
    process_options(argc, argv);
    internal_avro::ValidSchema schema =
        internal_avro::compileJsonSchemaFromString(RMF::data_avro::frame_json);
    internal_avro::DataFileReader<rmf_raw_avro2::Frame> reader(input.c_str(),
                                                               schema);
    rmf_raw_avro2::Frame frame;
    try {
      while (reader.read(frame)) {
        std::size_t offset = reader.blockOffsetBytes();
        std::cout << "Block offset " << offset << std::endl;
        if (variables_map.count("verbose")) {
          internal_avro::EncoderPtr encoder =
              internal_avro::jsonEncoder(schema);
          boost::shared_ptr<internal_avro::OutputStream> os =
              internal_avro::ostreamOutputStream(std::cout);
          encoder->init(*os);
          internal_avro::encode(*encoder, frame);

          // apparently these are necessary
          encoder->flush();
          os->flush();
        } else {
          if (frame.info.idx() == 0) {
            std::cout << "Frame: " << frame.info.get_FrameInfo().id;
          } else {
            std::cout << "static";
          }
        }
        std::cout << std::endl;
      }
    }
    catch (std::exception e) {
      std::cerr << "Caught exception " << e.what() << std::endl;
    }
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
