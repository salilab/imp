/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <boost/filesystem/path.hpp>

#include "MultipleAvroFileBase.h"
#include "RMF/internal/paths.h"
#include "AllJSON.h"
#include "AvroKeysAndCategories.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

std::string MultipleAvroFileBase::get_category_dynamic_file_path(Category cat)
    const {
  boost::filesystem::path base = get_file_path();
  boost::filesystem::path full = base / (get_name(cat) + ".frames");
  return full.string();
}
std::string MultipleAvroFileBase::get_category_static_file_path(Category cat)
    const {
  boost::filesystem::path base = get_file_path();
  boost::filesystem::path full = base / (get_name(cat) + ".static");
  return full.string();
}
std::string MultipleAvroFileBase::get_file_file_path() const {
  boost::filesystem::path base = get_file_path();
  boost::filesystem::path full = base / "file.rmf2info";
  return full.string();
}
std::string MultipleAvroFileBase::get_nodes_file_path() const {
  boost::filesystem::path base = get_file_path();
  boost::filesystem::path full = base / "nodes";
  return full.string();
}
std::string MultipleAvroFileBase::get_static_file_path() const {
  boost::filesystem::path base = get_file_path();
  boost::filesystem::path full = base / "static_data";
  return full.string();
}
std::string MultipleAvroFileBase::get_frames_file_path() const {
  boost::filesystem::path base = get_file_path();
  boost::filesystem::path full = base / "frames";
  return full.string();
}

MultipleAvroFileBase::MultipleAvroFileBase(std::string path)
    : AvroKeysAndCategories(path) {
  null_static_data_.frame = -1;
  null_data_.frame = 0;
  null_frame_data_.name = "static";
  null_frame_data_.type = "static";
}

void MultipleAvroFileBase::set_loaded_frame(FrameID frame) {
  null_data_.frame = frame.get_index();
  AvroKeysAndCategories::set_loaded_frame(frame);
}

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
