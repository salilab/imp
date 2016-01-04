/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "avrocpp/api/Compiler.hh"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <sstream>

#include "AllJSON.h"
#include "DataFile.hh"
#include "Decoder.hh"
#include "Encoder.hh"
#include "RMF/exceptions.h"
#include "RMF/internal/paths.h"
#include "SingleAvroFile.h"
#include "Specific.hh"
#include "Stream.hh"
#include "ValidSchema.hh"
#include "avro_schema_io.h"
#include "avrocpp/api/DataFile.hh"
#include "avrocpp/api/Decoder.hh"
#include "avrocpp/api/Encoder.hh"
#include "avrocpp/api/Specific.hh"
#include "avrocpp/api/Stream.hh"
#include "avrocpp/api/ValidSchema.hh"
#include "AvroKeysAndCategories.h"
#include "generated/embed_jsons.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

SingleAvroFile::SingleAvroFile(std::string path, bool create,
                               bool /*read_only*/)
    : AvroKeysAndCategories(path), dirty_(false), text_(get_is_text(path)) {
  if (!create) {
    reload();
  } else {
    initialize_frames();
    initialize_categories();
    initialize_node_keys();
    all_.file.version = 1;
  }
  null_static_frame_data_.frame = -1;
}

SingleAvroFile::SingleAvroFile(boost::shared_ptr<std::vector<char> > buffer,
                               bool create, bool)
    : AvroKeysAndCategories("buffer"),
      dirty_(false),
      text_(false),
      buffer_(buffer) {
  if (!create) {
    reload();
  } else {
    initialize_frames();
    initialize_categories();
    initialize_node_keys();
    all_.file.version = 1;
  }
  null_static_frame_data_.frame = -1;
}

void SingleAvroFile::initialize_frames() {
  all_.frames.push_back(RMF_avro_backend::Node());
  access_frame(ALL_FRAMES).name = "static";
  access_frame(ALL_FRAMES).type = "static";
}

void SingleAvroFile::initialize_categories() {
  for (std::map<std::string,
                std::vector<RMF_avro_backend::Data> >::const_iterator it =
           all_.category.begin();
       it != all_.category.end(); ++it) {
    get_category(it->first);
  }
}

void SingleAvroFile::initialize_node_keys() {
  clear_node_keys();
  for (unsigned int i = 0; i < all_.nodes.size(); ++i) {
    add_node_key();
  }
}

void SingleAvroFile::flush() {
  if (!dirty_) return;
  if (!buffer_) {
    if (!text_) {
      write(all_, internal_avro::compileJsonSchemaFromString(
                      data_deprecated_avro::all_json),
            get_file_path());
    } else {
      write_text(all_, internal_avro::compileJsonSchemaFromString(
                           data_deprecated_avro::all_json),
                 get_file_path());
    }
  } else {
    buffer_->clear();
    std::ostringstream oss(std::ios_base::binary);
    boost::shared_ptr<internal_avro::OutputStream> os =
        internal_avro::ostreamOutputStream(oss);
    boost::shared_ptr<internal_avro::Encoder> encoder =
        internal_avro::binaryEncoder();
    encoder->init(*os);
    internal_avro::encode(*encoder, all_);
    os->flush();
    encoder.reset();
    os.reset();
    const std::string& str = oss.str();
    buffer_->insert(buffer_->end(), str.begin(), str.end());
  }
  dirty_ = false;
}

void SingleAvroFile::reload() {
  if (!buffer_ && !text_) {
    bool success;
    try {
      internal_avro::DataFileReader<RMF_avro_backend::All> rd(
          get_file_path().c_str(), internal_avro::compileJsonSchemaFromString(
                                       data_deprecated_avro::all_json));
      success = rd.read(all_);
    }
    catch (std::exception& e) {
      RMF_THROW(Message(e.what()) << File(get_file_path()), IOException);
    }
    if (!success) {
      RMF_THROW(Message("Can't read input file on reload"), IOException);
    }
  } else if (!buffer_ && text_) {
    boost::shared_ptr<internal_avro::Decoder> decoder =
        internal_avro::jsonDecoder(internal_avro::compileJsonSchemaFromString(
            data_deprecated_avro::all_json));
    boost::shared_ptr<internal_avro::InputStream> stream =
        internal_avro::fileInputStream(get_file_path().c_str());
    decoder->init(*stream);
    bool success = false;
    try {
      internal_avro::decode(*decoder, all_);
      success = true;
    }
    catch (std::exception& e) {
      RMF_THROW(Message(e.what()) << File(get_file_path()), IOException);
    }
    if (!success) {
      RMF_THROW(Message("Can't read input file on reload"), IOException);
    }
  } else {
    boost::shared_ptr<internal_avro::InputStream> is =
        internal_avro::memoryInputStream(
            reinterpret_cast<uint8_t*>(&(*buffer_)[0]), buffer_->size());
    boost::shared_ptr<internal_avro::Decoder> decoder =
        internal_avro::binaryDecoder();
    decoder->init(*is);
    internal_avro::decode(*decoder, all_);
  }
  initialize_categories();
  initialize_node_keys();
  dirty_ = false;
}

FrameID SingleAvroFile::add_frame(std::string name, FrameType t) {
  FrameID index = FrameID(get_number_of_frames());
  access_frame(index).name = name;
  access_frame(index).type = boost::lexical_cast<std::string>(t);
  if (get_loaded_frame() != FrameID()) {
    access_frame(get_loaded_frame()).children.push_back(index.get_index());
  }
  RMF_INTERNAL_CHECK(get_number_of_frames() ==
                         static_cast<unsigned int>(index.get_index()) + 1,
                     "No frame added");
  return index;
}

void SingleAvroFile::add_child_frame(FrameID child_node) {
  access_frame(get_loaded_frame()).children.push_back(child_node.get_index());
}

FrameIDs SingleAvroFile::get_children(FrameID node) const {
  return FrameIDs(get_frame(node).children.begin(),
                  get_frame(node).children.end());
}

std::string SingleAvroFile::get_loaded_frame_name() const {
  return get_frame(get_loaded_frame()).name;
}
FrameType SingleAvroFile::get_loaded_frame_type() const {
  return boost::lexical_cast<FrameType>(get_frame(get_loaded_frame()).type);
}
unsigned int SingleAvroFile::get_number_of_frames() const {
  return get_frames().size() - 1;
}

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
