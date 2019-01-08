/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/functional/hash/hash.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/version.hpp>
#include <algorithm>
#include <exception>
#include <functional>
#include <sstream>

#include "DataFile.hh"
#include "MultipleAvroFileReader.h"
#include "RMF/log.h"
#include "ValidSchema.hh"
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

namespace {
void clear_data(RMF_avro_backend::Data& data, FrameID frame) {
  data = RMF_avro_backend::Data();
  data.frame = frame.get_index();
}
}

void MultipleAvroFileReader::set_loaded_frame(FrameID frame) {
  RMF_TRACE("Loading frame " << frame);
  null_data_.frame = frame.get_index();
  for (unsigned int i = 0; i < categories_.size(); ++i) {
    if (!categories_[i].reader) {
      clear_data(categories_[i].data, frame);
    }
    if (frame.get_index() <
        static_cast<unsigned int>(categories_[i].data.frame)) {
      RMF_INTERNAL_CHECK(categories_[i].reader, "No old reader found");
      std::string name = get_category_dynamic_file_path(Category(i));
      try {
        RMF_TRACE("Opening category file for " << get_name(Category(i)));
        categories_[i].reader.reset();
        categories_[i].reader.reset(
            new internal_avro::DataFileReader<RMF_avro_backend::Data>(
                name.c_str(), ::internal_avro::compileJsonSchemaFromString(
                                  data_deprecated_avro::data_json)));
      }
      catch (const std::exception& e) {
        RMF_THROW(Message(e.what()) << Component(name), IOException);
      }
      bool success = categories_[i].reader->read(categories_[i].data);
      if (!success) {
        RMF_THROW(Message("Unable to read data from input"), IOException);
      }
    }
    while (frame.get_index() >
           static_cast<unsigned int>(categories_[i].data.frame)) {
      if (!categories_[i].reader->read(categories_[i].data)) {
        // std::cout << "Out of data looking for " << frame << std::endl;
        RMF_TRACE("Out of data for category " << get_name(Category(i)));
        clear_data(categories_[i].data, frame);
        break;
      } else {
        RMF_TRACE("Loaded category " << get_name(Category(i)));
      }
      if (frame.get_index() <
          static_cast<unsigned int>(categories_[i].data.frame)) {
        RMF_TRACE("Missing frame for category " << get_name(Category(i)));
        clear_data(categories_[i].data, frame);
        break;
      }
    }
  }
  MultipleAvroFileBase::set_loaded_frame(frame);
}

MultipleAvroFileReader::MultipleAvroFileReader(std::string path, bool create,
                                               bool read_only)
    : MultipleAvroFileBase(path) {
  RMF_UNUSED(create);
  RMF_UNUSED(read_only);
  RMF_INTERNAL_CHECK(!create, "Can't create a file for reading");
  RMF_INTERNAL_CHECK(read_only,
                     "RMF2 files open for reading must be read-only");
  reload();
}

template <class It>
std::vector<std::string> get_categories_from_disk(It a, It b) {
  std::vector<std::string> ret;
  for (; a != b; ++a) {
    if (a->path().extension() == ".frames" ||
        a->path().extension() == ".static") {
#if BOOST_VERSION >= 104600
      ret.push_back(a->path().stem().string());
#else
      ret.push_back(a->path().stem());
#endif
    }
  }
  return ret;
}
void MultipleAvroFileReader::initialize_categories() {
  std::string path = get_file_path();
  // std::cout << "Searching in " << path << std::endl;
  std::vector<std::string> categories =
      get_categories_from_disk(boost::filesystem::directory_iterator(path),
                               boost::filesystem::directory_iterator());
  categories_.clear();
  for (unsigned int i = 0; i < categories.size(); ++i) {
    // std::cout << "initializing category " << categories[i] << std::endl;
    Category cat = get_category(categories[i]);
    add_category_data(cat);
  }
}

#define RMF_RELOAD(UCName, lcname)                                          \
  {                                                                         \
    bool success;                                                           \
    try {                                                                   \
      RMF_TRACE("Opening " #lcname " data");                                \
      internal_avro::DataFileReader<UCName> re(                             \
          get_##lcname##_file_path().c_str(),                               \
          internal_avro::compileJsonSchemaFromString(                       \
              data_deprecated_avro::lcname##_json));                        \
      success = re.read(lcname##_);                                         \
    }                                                                       \
    catch (const std::exception& e) {                                       \
      RMF_THROW(Message(e.what()) << Component(get_##lcname##_file_path()), \
                IOException);                                               \
    }                                                                       \
    if (!success) {                                                         \
      RMF_THROW(Message("Error parsing data")                               \
                    << Component(get_##lcname##_file_path()),               \
                IOException);                                               \
    }                                                                       \
  }

void MultipleAvroFileReader::reload() {
  RMF_RELOAD(File, file);
  RMF_RELOAD(Nodes, nodes);
  if (file_.version >= 2) {
    // In old RMF we used a monolithic frame file that we won't bother parsing
    try {
      internal_avro::DataFileReader<RMF_avro_backend::Frame> re(
          get_frames_file_path().c_str(),
          internal_avro::compileJsonSchemaFromString(
              data_deprecated_avro::frame_json));
      do {
        RMF_avro_backend::Frame frame;
        if (!re.read(frame)) break;
        frames_[frame.index] = frame;
        number_of_frames_ = frame.index + 1;
        for (unsigned int i = 0; i < frame.parents.size(); ++i) {
          frame_children_[frame.parents[i]].push_back(frame.index);
        }
      } while (true);
    }
    catch (const std::exception& e) {
      RMF_THROW(Message(e.what()) << Component(get_frames_file_path()),
                IOException);
    }
  } else {
    RMF_WARN("Ignoring frames data in old rmf2 file.");
  }

  initialize_categories();
  initialize_node_keys();
  // dance to read the correct data in
  FrameID current = get_loaded_frame();
  if (current != FrameID()) {
    set_loaded_frame(current);
  }
}

void MultipleAvroFileReader::add_category_data(Category cat) {
  if (categories_.size() <= cat.get_index()) {
    categories_.resize(cat.get_index() + 1);
    static_categories_.resize(cat.get_index() + 1);
  }

  std::string dynamic_path = get_category_dynamic_file_path(cat);
  // std::cout << "Checking dynamic path " << dynamic_path << std::endl;
  if (boost::filesystem::exists(dynamic_path)) {
    // std::cout << "Dynamic data found" << std::endl;
    try {
      // make sure it is closed before reopening on windows
      categories_[cat.get_index()].reader.reset();
      categories_[cat.get_index()].reader.reset(
          new internal_avro::DataFileReader<RMF_avro_backend::Data>(
              dynamic_path.c_str(), internal_avro::compileJsonSchemaFromString(
                                        data_deprecated_avro::data_json)));
    }
    catch (const std::exception& e) {
      RMF_THROW(Message(e.what()) << Component(dynamic_path), IOException);
    }
    bool success = categories_[cat.get_index()]
                       .reader->read(categories_[cat.get_index()].data);
    if (!success) {
      RMF_THROW(Message("Error reading from data file")
                    << Component(dynamic_path),
                IOException);
    }
  } else {
    categories_[cat.get_index()].data.frame = 0;
  }

  std::string static_path = get_category_static_file_path(cat);
  // std::cout << "Checking static path " << static_path << std::endl;
  if (boost::filesystem::exists(static_path)) {
    // std::cout << "Static data found" << std::endl;
    bool success;
    try {
      internal_avro::DataFileReader<RMF_avro_backend::Data> reader(
          static_path.c_str(), internal_avro::compileJsonSchemaFromString(
                                   data_deprecated_avro::data_json));
      success = reader.read(static_categories_[cat.get_index()]);
    }
    catch (const std::exception& e) {
      RMF_THROW(Message(e.what()) << Component(static_path), IOException);
    }
    if (!success) {
      RMF_THROW(Message("Error reading from data file")
                    << Component(static_path),
                IOException);
    }
  } else {
    static_categories_[cat.get_index()].frame = -1;
  }
}

FrameID MultipleAvroFileReader::add_frame(std::string /*name*/,
                                          FrameType /*t*/) {
  RMF_THROW(Message("Trying to modify read-only file"), UsageException);
}
void MultipleAvroFileReader::add_child_frame(FrameID /*child_node*/) {
  RMF_THROW(Message("Trying to modify read-only file"), UsageException);
}
FrameIDs MultipleAvroFileReader::get_children(FrameID node) const {
  if (frame_children_.find(node.get_index()) != frame_children_.end()) {
    return FrameIDs(frame_children_.find(node.get_index())->second.begin(),
                    frame_children_.find(node.get_index())->second.end());
  } else
    return FrameIDs();
}

std::string MultipleAvroFileReader::get_loaded_frame_name() const {
  FrameID i = get_loaded_frame();
  if (frames_.find(i.get_index()) != frames_.end()) {
    return frames_.find(i.get_index())->second.name;
  } else {
    return "";
  }
}
FrameType MultipleAvroFileReader::get_loaded_frame_type() const {
  FrameID i = get_loaded_frame();
  if (frames_.find(i.get_index()) != frames_.end()) {
    return boost::lexical_cast<FrameType>(
        frames_.find(i.get_index())->second.type);
  } else {
    return FRAME;
  }
}
unsigned int MultipleAvroFileReader::get_number_of_frames() const {
  return number_of_frames_;
}

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
