/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <sstream>

#include "DataFile.hh"
#include "MultipleAvroFileWriter.h"
#include "RMF/internal/paths.h"
#include "RMF/log.h"
#include "ValidSchema.hh"
#include "avro_schema_io.h"
#include "avrocpp/api/Compiler.hh"
#include "avrocpp/api/DataFile.hh"
#include "avrocpp/api/ValidSchema.hh"
#include "AllJSON.h"
#include "FrameJSON.h"
#include "MultipleAvroFileBase.h"
#include "generated/embed_jsons.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

void MultipleAvroFileWriter::set_loaded_frame(FrameID frame) {
  if (frame == get_loaded_frame()) return;
  RMF_USAGE_CHECK(
      frame.get_index() == static_cast<unsigned int>(frame_.index + 1) ||
          frame.get_index() == static_cast<unsigned int>(frame_.index),
      "Bad frame set. You probably didn't add a new frame.");
  MultipleAvroFileBase::set_loaded_frame(frame);
  if (frame_.index != -1 &&
      frame.get_index() != static_cast<unsigned int>(frame_.index)) {
    commit();
  }
}

MultipleAvroFileWriter::MultipleAvroFileWriter(std::string path, bool create,
                                               bool read_only)
    : MultipleAvroFileBase(path) {
  RMF_INTERNAL_CHECK(create, "Can only create files");
  RMF_INTERNAL_CHECK(!read_only, "Can only create files");
  RMF_UNUSED(create);
  RMF_UNUSED(read_only);
  boost::filesystem::remove_all(path);
  boost::filesystem::create_directory(path);
  frame_.index = -1;
  frame_.name = "static";
  frame_.type = "static";
  file_.version = 2;
  file_dirty_ = true;
  frames_dirty_ = true;
  nodes_dirty_ = true;
}

MultipleAvroFileWriter::~MultipleAvroFileWriter() { commit(); }

#define RMF_COMMIT(UCName, lcname)                               \
  if (lcname##_dirty_) {                                         \
    write(lcname##_, internal_avro::compileJsonSchemaFromString( \
                         data_deprecated_avro::lcname##_json),   \
          get_##lcname##_file_path());                           \
  }

void MultipleAvroFileWriter::commit() {
  RMF_TRACE("Writing frame " << frame_.index);
  for (unsigned int i = 0; i < categories_.size(); ++i) {
    if (categories_[i].dirty) {
      if (!categories_[i].writer) {
        std::string name = get_category_dynamic_file_path(Category(i));
        try {
          categories_[i].writer.reset(
              new internal_avro::DataFileWriter<RMF_avro_backend::Data>(
                  name.c_str(), internal_avro::compileJsonSchemaFromString(
                                    data_deprecated_avro::data_json)));
        }
        catch (const std::exception& e) {
          RMF_THROW(Message(e.what()) << Component(name), IOException);
        }
      }
      // show(categories_[i].data);
      RMF_INTERNAL_CHECK(categories_[i].data.frame == frame_.index,
                         "Trying to write category that is at wrong frame.");
      categories_[i].writer->write(categories_[i].data);
      categories_[i].writer->flush();
    }
    categories_[i].data = RMF_avro_backend::Data();
    // go to the about to be added frame
    categories_[i].data.frame = frame_.index + 1;
  }
  for (unsigned int i = 0; i < static_categories_.size(); ++i) {
    if (static_categories_dirty_[i]) {
      std::string name = get_category_static_file_path(Category(i));
      try {
        internal_avro::DataFileWriter<RMF_avro_backend::Data> writer(
            name.c_str(), internal_avro::compileJsonSchemaFromString(
                              data_deprecated_avro::data_json));
        writer.write(static_categories_[i]);
        writer.flush();
      }
      catch (const std::exception& e) {
        RMF_THROW(Message(e.what()) << Component(name), IOException);
      }
      // std::cout << "Writing data for " << get_category_name(Category(i)) <<
      // std::endl;
      // show(static_categories_[i]);
      static_categories_dirty_[i] = false;
    }
  }
  RMF_COMMIT(File, file);
  RMF_COMMIT(Nodes, nodes);
  if (frames_dirty_) {
    if (!frame_writer_) {
      frame_writer_.reset(
          new internal_avro::DataFileWriter<RMF_avro_backend::Frame>(
              get_frames_file_path().c_str(),
              internal_avro::compileJsonSchemaFromString(
                  data_deprecated_avro::frame_json)));
    }
    frame_writer_->write(frame_);
    frames_dirty_ = false;
  }
}

FrameID MultipleAvroFileWriter::add_frame(std::string name, FrameType t) {
  unsigned int index = get_number_of_frames();
  RMF_TRACE("Adding frame " << index << " under " << get_loaded_frame());
  set_loaded_frame(FrameID(index));
  frame_.name = name;
  frame_.type = boost::lexical_cast<std::string>(FrameType(t));
  if (get_loaded_frame() != FrameID()) {
    unsigned int findex = get_loaded_frame().get_index();
    frame_.parents.push_back(findex);
  }
  frames_dirty_ = true;
  frame_.index = index;
  return FrameID(index);
}
void MultipleAvroFileWriter::add_child_frame(FrameID) {
  RMF_THROW(Message(
                "RMF2 writer doesn't support adding other frames than "
                "the current as a child."),
            UsageException);
}
FrameIDs MultipleAvroFileWriter::get_children(FrameID /*node*/) const {
  RMF_THROW(Message("RMF2 writer doesn't support getting frame children."),
            UsageException);
}

std::string MultipleAvroFileWriter::get_loaded_frame_name() const {
  return frame_.name;
}
FrameType MultipleAvroFileWriter::get_loaded_frame_type() const {
  return boost::lexical_cast<FrameType>(frame_.type);
}
unsigned int MultipleAvroFileWriter::get_number_of_frames() const {
  return frame_.index + 1;
}

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
