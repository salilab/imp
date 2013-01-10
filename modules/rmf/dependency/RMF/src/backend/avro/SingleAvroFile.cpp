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
#include <stdexcept>

namespace RMF {
namespace internal {

SingleAvroFile::SingleAvroFile(std::string path, bool create,
                               bool read_only): AvroKeysAndCategories(path),
                                                buffer_(NULL),
                                                write_to_buffer_(false) {
  if (!create) {
    reload();
  } else {
    initialize_categories();
    initialize_node_keys();
  }
  null_static_frame_data_.frame = ALL_FRAMES;
}

SingleAvroFile::SingleAvroFile(std::string &path, bool create,
                               bool read_only,
                               bool): AvroKeysAndCategories("buffer"),
                                      buffer_(&path),
                                      write_to_buffer_(true) {
  if (!create) {
    reload();
  } else {
    initialize_categories();
    initialize_node_keys();
  }
  null_static_frame_data_.frame = ALL_FRAMES;

  if (read_only) {
    // so we don't write to it
    buffer_ = NULL;
  }
}

void SingleAvroFile::initialize_categories() {
  for (std::map<std::string, std::vector<RMF_internal::Data > >::const_iterator
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
    write(all_, get_All_schema(), get_file_path());
  } else {
    buffer_->clear();
    std::ostringstream oss(std::ios_base::binary);
    std::auto_ptr<avro::OutputStream> os = avro::ostreamOutputStream(oss);
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
  if (!write_to_buffer_) {
    bool success;
    try {
      avro::DataFileReader<RMF_internal::All>
      rd(get_file_path().c_str(), get_All_schema());
      success = rd.read(all_);
    } catch (std::exception &e) {
      RMF_THROW(Message(e.what()) << File(get_file_path()),
                IOException);
    }
    if (!success) {
      RMF_THROW(Message("Can't read input file on reload"), IOException);
    }
  } else {
    std::istringstream iss(*buffer_, std::ios_base::binary);
    std::auto_ptr<avro::InputStream> is = avro::istreamInputStream(iss);
    boost::shared_ptr<avro::Decoder> decoder = avro::binaryDecoder();
    decoder->init(*is);
    avro::decode(*decoder, all_);
  }
  initialize_categories();
  initialize_node_keys();
  dirty_ = false;
}

}   // namespace internal
} /* namespace RMF */
