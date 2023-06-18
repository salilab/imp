#include <assert.h>
#include <avrocpp/api/Compiler.hh>
#include <avrocpp/api/DataFile.hh>
#include <backend/avro/raw_frame.h>
#include <memory>
#include <generated/embed_jsons.h>
#include <iostream>
#include <string>

#include "RMF/ID.h"
#include "RMF/internal/paths.h"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Stream.hh"
#include "avrocpp/api/ValidSchema.hh"
#include "backend/avro/data_file.h"
#include "backend/avro/types.h"

namespace {
void write(std::string name) {
  internal_avro::DataFileWriterBase writer(
      name.c_str(),
      internal_avro::compileJsonSchemaFromString(RMF::data_avro::frame_json),
      16 * 1024);
  RMF::avro2::FileDataChanges file_data;
  RMF::avro2::Frame frame;
  file_data.description = "description0";
  RMF::avro2::write(&writer, file_data);
  frame.id = RMF::FrameID(0);
  RMF::avro2::write(&writer, frame);
  frame.id = RMF::FrameID(1);
  RMF::avro2::write(&writer, frame);
  file_data.description = "description1";
  RMF::avro2::write(&writer, file_data);
  frame.id = RMF::FrameID(2);
  RMF::avro2::write(&writer, frame);
}
unsigned read(std::string name) {
  internal_avro::DataFileReader<RMF::avro2::FileData> reader(
      name.c_str(),
      internal_avro::compileJsonSchemaFromString(RMF::data_avro::frame_json));
  RMF::avro2::FileData file_data;
  RMF::avro2::load_file_data(reader, file_data);
  assert(file_data.description == "description1");
  assert(file_data.max_id.get_index() == 2);
  return file_data.max_id.get_index();
}
void read_frames(std::string name, unsigned max_id) {
  internal_avro::DataFileReader<RMF::avro2::Frame> frame_reader(
      name.c_str(),
      internal_avro::compileJsonSchemaFromString(RMF::data_avro::frame_json));
  for (unsigned int i = 0; i <= max_id; ++i) {
    RMF::avro2::Frame frame;
    load_frame(RMF::FrameID(i), frame_reader, frame);
  }
}
void read_raw(std::string name) {
  internal_avro::ValidSchema schema =
      internal_avro::compileJsonSchemaFromString(RMF::data_avro::frame_json);
  internal_avro::DataFileReader<rmf_raw_avro2::Frame> reader(name.c_str(),
                                                             schema);
  rmf_raw_avro2::Frame frame;
  int read = 0;
  while (reader.read(frame)) {
    internal_avro::EncoderPtr encoder = internal_avro::jsonEncoder(schema);
    std::shared_ptr<internal_avro::OutputStream> os =
        internal_avro::ostreamOutputStream(std::cout);
    encoder->init(*os);
    internal_avro::encode(*encoder, frame);
    encoder->flush();
    os->flush();
    ++read;
  }
  assert(read == 5);
}
}

int main(int, char * []) {
  std::string name = RMF::internal::get_unique_path();
  std::cout << "File is " << name << std::endl;
  write(name);
  unsigned max_id = read(name);
  read_frames(name, max_id);
  read_raw(name);
  return 0;
}
