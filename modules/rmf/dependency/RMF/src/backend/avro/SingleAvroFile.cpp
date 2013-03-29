/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "avro_schemas.h"
#include "SingleAvroFile.h"
#include <RMF/internal/paths.h>
#include <RMF/decorators.h>
#include <avro/Compiler.hh>
#include <boost/scoped_ptr.hpp>
#include <stdexcept>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

SingleAvroFile::SingleAvroFile(std::string path, bool create,
                               bool /*read_only*/): AvroKeysAndCategories(path),
                                                    dirty_(false),
                                                    text_(get_is_text(path)),
                                                buffer_(NULL),
                                                write_to_buffer_(false) {
  if (!create) {
    reload();
  } else {
    initialize_frames();
    initialize_categories();
    initialize_node_keys();
    all_.file.version=1;
  }
  null_static_frame_data_.frame = ALL_FRAMES;
}

SingleAvroFile::SingleAvroFile(std::string &path, bool create):
  AvroKeysAndCategories("buffer"),
  dirty_(false), text_(false),
  buffer_(&path),
  write_to_buffer_(true) {
  if (!create) {
    reload();
  } else {
    initialize_frames();
    initialize_categories();
    initialize_node_keys();
    all_.file.version=1;
  }
  null_static_frame_data_.frame = ALL_FRAMES;
}


SingleAvroFile::SingleAvroFile(const std::string &path):
  AvroKeysAndCategories("buffer"),
  dirty_(false), text_(false),
  buffer_(const_cast<std::string*>(&path)),
  write_to_buffer_(true) {
  reload();
  null_static_frame_data_.frame = ALL_FRAMES;

  // so we don't write to it
  buffer_ = NULL;
}

  void SingleAvroFile::initialize_frames() {
    all_.frames.push_back(RMF_avro_backend::Node());
    access_frame(ALL_FRAMES).name = "static";
    access_frame(ALL_FRAMES).type = "static";
  }

void SingleAvroFile::initialize_categories() {
  for (std::map<std::string, std::vector<RMF_avro_backend::Data > >::const_iterator
       it = all_.category.begin(); it != all_.category.end(); ++it) {
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
  if (!write_to_buffer_) {
    if (!text_) {
      write(all_, get_All_schema(), get_file_path());
    } else {
      write_text(all_, get_All_schema(), get_file_path());
    }
  } else {
    buffer_->clear();
    std::ostringstream oss(std::ios_base::binary);
    boost::scoped_ptr<avro::OutputStream>
      os(avro::ostreamOutputStream(oss).release());
    boost::shared_ptr<avro::Encoder> encoder = avro::binaryEncoder();
    encoder->init(*os);
    avro::encode(*encoder, all_);
    os->flush();
    encoder.reset();
    os.reset();
    *buffer_ = oss.str();
  }
  dirty_ = false;
}

void SingleAvroFile::reload() {
  if (!write_to_buffer_ && !text_) {
    bool success;
    try {
      avro::DataFileReader<RMF_avro_backend::All>
      rd(get_file_path().c_str(), get_All_schema());
      success = rd.read(all_);
    } catch (std::exception &e) {
      RMF_THROW(Message(e.what()) << File(get_file_path()),
                IOException);
    }
    if (!success) {
      RMF_THROW(Message("Can't read input file on reload"), IOException);
    }
  } else if (!write_to_buffer_ && text_) {
    boost::shared_ptr<avro::Decoder> decoder
      = avro::jsonDecoder(get_All_schema());
    std::auto_ptr<avro::InputStream> stream
      = avro::fileInputStream(get_file_path().c_str());
    decoder->init(*stream);
    bool success=false;
    try {
      avro::decode(*decoder, all_);
      success = true;
    } catch (std::exception &e) {
      RMF_THROW(Message(e.what()) << File(get_file_path()),
                IOException);
    }
    if (!success) {
      RMF_THROW(Message("Can't read input file on reload"), IOException);
    }
  } else {
    std::istringstream iss(*buffer_, std::ios_base::binary);
    boost::scoped_ptr<avro::InputStream>
      is(avro::istreamInputStream(iss).release());
    boost::shared_ptr<avro::Decoder> decoder = avro::binaryDecoder();
    decoder->init(*is);
    avro::decode(*decoder, all_);
  }
  initialize_categories();
  initialize_node_keys();
  dirty_ = false;
}


int SingleAvroFile::add_child_frame(int node, std::string name, int t) {
  unsigned int index = get_number_of_frames();
  access_frame(index).name = name;
  access_frame(index).type = boost::lexical_cast<std::string>(FrameType(t));
  access_frame(node).children.push_back(index);
  RMF_INTERNAL_CHECK(get_number_of_frames() == index + 1,
                     "No frame added");
  return index;
}

void SingleAvroFile::add_child_frame(int node, int child_node) {
  access_frame(node).children.push_back(child_node);
}

Ints SingleAvroFile::get_children_frame(int node) const {
  return Ints(get_frame(node).children.begin(),
              get_frame(node).children.end());
}

  std::string SingleAvroFile::get_frame_name(int i) const {
    return get_frame(i).name;
  }
  unsigned int SingleAvroFile::get_number_of_frames() const {
    return get_frames().size() - 1;
  }


}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
