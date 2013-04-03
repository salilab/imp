/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "avro_schemas.h"
#include "MultipleAvroFileReader.h"
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <stdexcept>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

namespace {
void clear_data(RMF_avro_backend::Data &data,
                int                frame) {
  data = RMF_avro_backend::Data();
  data.frame = frame;
}
}

void MultipleAvroFileReader::set_current_frame(int frame) {
  if (frame != ALL_FRAMES) {
    RMF_TRACE(get_avro_logger(), "Loading frame " << frame);
  } else {
    RMF_TRACE(get_avro_logger(), "Loading static data");
  }
  if (frame != ALL_FRAMES) {
    null_data_.frame = frame;
    for (unsigned int i = 0; i < categories_.size(); ++i) {
      if (!categories_[i].reader) {
        clear_data(categories_[i].data, frame);
      }
      if (frame < categories_[i].data.frame) {
        RMF_INTERNAL_CHECK(categories_[i].reader,
                           "No old reader found");
        std::string name = get_category_dynamic_file_path(Category(i));
        try {
          RMF_TRACE(get_avro_logger(),
                    "Opening category file for "
                    << get_category_name(Category(i)));
          categories_[i].reader.reset();
          categories_[i].reader
          .reset( new rmf_avro::DataFileReader<RMF_avro_backend::Data >(name.c_str(),
                                                                get_Data_schema()));
        } catch (const std::exception &e) {
          RMF_THROW(Message(e.what()) << Component(name), IOException);
        }
        bool success = categories_[i].reader->read(categories_[i].data);
        if (!success) {
          RMF_THROW(Message("Unable to read data from input"), IOException);
        }
      }
      while (frame > categories_[i].data.frame) {
        if (!categories_[i].reader->read(categories_[i].data)) {
          //std::cout << "Out of data looking for " << frame << std::endl;
          RMF_TRACE(get_avro_logger(),
                    "Out of data for category "
                    << get_category_name(Category(i)));
          clear_data(categories_[i].data, frame);
          break;
        } else {
          RMF_TRACE(get_avro_logger(),
                    "Loaded category "
                    << get_category_name(Category(i)));
        }
        if (frame < categories_[i].data.frame) {
          RMF_TRACE(get_avro_logger(),
                    "Missing frame for category "
                    << get_category_name(Category(i)));
          clear_data(categories_[i].data, frame);
          break;
        }
      }
    }
  }
  MultipleAvroFileBase::set_current_frame(frame);
}

MultipleAvroFileReader::MultipleAvroFileReader(std::string path,
                                               bool create, bool read_only):
  MultipleAvroFileBase(path) {
  RMF_INTERNAL_CHECK(!create,   "Can't create a file for reading");
  RMF_INTERNAL_CHECK(read_only, "RMF2 files open for reading must be read-only");
  reload();
}

template <class It>
std::vector<std::string> get_categories_from_disk(It a, It b) {
  std::vector<std::string> ret;
  for (; a != b; ++a) {
    if (a->path().extension() == ".frames"
        || a->path().extension() == ".static") {
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
  //std::cout << "Searching in " << path << std::endl;
  std::vector<std::string> categories
    = get_categories_from_disk(boost::filesystem::directory_iterator(path),
                               boost::filesystem::directory_iterator());
  categories_.clear();
  for (unsigned int i = 0; i < categories.size(); ++i) {
    //std::cout << "initializing category " << categories[i] << std::endl;
    Category cat = get_category(categories[i]);
    add_category_data(cat);
  }
}



#define RMF_RELOAD(UCName, lcname)                                      \
  {                                                                     \
    bool success;                                                       \
    try {                                                               \
      RMF_TRACE(get_avro_logger(), "Opening " #lcname " data");         \
      rmf_avro::DataFileReader<UCName> re(get_##lcname##_file_path().c_str(), \
                                      get_##UCName##_schema());         \
      success = re.read(lcname##_);                                     \
    } catch (const std::exception &e) {                                 \
      RMF_THROW(Message(e.what())                                       \
                << Component(get_##lcname##_file_path()), IOException); \
    }                                                                   \
    if (!success) {                                                     \
      RMF_THROW(Message("Error parsing data")                           \
                << Component(get_##lcname##_file_path()), IOException); \
    }                                                                   \
  }

void MultipleAvroFileReader::reload() {
  RMF_RELOAD(File,  file);
  RMF_RELOAD(Nodes, nodes);
  if (file_.version  >= 2) {
    // In old RMF we used a monolithic frame file that we won't bother parsing
    try {
      rmf_avro::DataFileReader<RMF_avro_backend::Frame> re(get_frames_file_path().c_str(),
                                                       get_Frame_schema());
      do {
        RMF_avro_backend::Frame frame;
        if (! re.read(frame)) break;
        frames_[frame.index]=frame;
        number_of_frames_= frame.index+1;
        for (unsigned int i=0; i< frame.parents.size(); ++i) {
          frame_children_[frame.parents[i]].push_back(frame.index);
        }
      } while (true);
    } catch (const std::exception &e) {
      RMF_THROW(Message(e.what())
                << Component(get_frames_file_path()), IOException);
    }
  } else {
    RMF_WARN(get_avro_logger(), "Ignoring frames data in old rmf2 file.");
  }

  initialize_categories();
  initialize_node_keys();
  // dance to read the correct data in
  int current = get_current_frame();
  set_current_frame(ALL_FRAMES);
  set_current_frame(current);
}

void MultipleAvroFileReader::add_category_data(Category cat) {
  if (categories_.size() <= cat.get_id()) {
    categories_.resize(cat.get_id() + 1);
    static_categories_.resize(cat.get_id() + 1);
  }

  std::string dynamic_path = get_category_dynamic_file_path(cat);
  //std::cout << "Checking dynamic path " << dynamic_path << std::endl;
  if (boost::filesystem::exists(dynamic_path)) {
    //std::cout << "Dynamic data found" << std::endl;
    try {
      // make sure it is closed before reopening on windows
      categories_[cat.get_id()].reader.reset();
      categories_[cat.get_id()].reader
      .reset(new rmf_avro::DataFileReader<RMF_avro_backend::Data>(dynamic_path.c_str(),
                                                          get_Data_schema()));
    } catch (const std::exception &e) {
      RMF_THROW(Message(e.what()) << Component(dynamic_path), IOException);
    }
    bool success
      = categories_[cat.get_id()].reader->read(categories_[cat.get_id()].data);
    if (!success) {
      RMF_THROW(Message("Error reading from data file")
                << Component(dynamic_path), IOException);
    }
  } else {
    categories_[cat.get_id()].data.frame = 0;
  }

  std::string static_path = get_category_static_file_path(cat);
  //std::cout << "Checking static path " << static_path << std::endl;
  if (boost::filesystem::exists(static_path)) {
    //std::cout << "Static data found" << std::endl;
    bool success;
    try {
      rmf_avro::DataFileReader<RMF_avro_backend::Data> reader(static_path.c_str(),
                                                      get_Data_schema());
      success = reader.read(static_categories_[cat.get_id()]);
    } catch (const std::exception &e) {
      RMF_THROW(Message(e.what()) << Component(static_path), IOException);
    }
    if (!success) {
      RMF_THROW(Message("Error reading from data file")
                << Component(static_path), IOException);
    }
  } else {
    static_categories_[cat.get_id()].frame = ALL_FRAMES;
  }
}

  int MultipleAvroFileReader::add_child_frame(int /*node*/, std::string /*name*/,
                                              int /*t*/) {
    RMF_THROW(Message("Trying to modify read-only file"),
              UsageException);
  }
  void MultipleAvroFileReader::add_child_frame(int /*node*/, int /*child_node*/) {
    RMF_THROW(Message("Trying to modify read-only file"),
              UsageException);
  }
  Ints MultipleAvroFileReader::get_children_frame(int node) const {
    if (frame_children_.find(node) != frame_children_.end()) {
      return frame_children_.find(node)->second;
    } else return Ints();
  }

 std::string MultipleAvroFileReader::get_frame_name(int i) const {
    if (i==ALL_FRAMES) {
      return "static";
    } else {
      if (frames_.find(i) != frames_.end()) {
        return frames_.find(i)->second.name;
      } else {
        return "";
      }
    }
  }
  unsigned int MultipleAvroFileReader::get_number_of_frames() const {
    return number_of_frames_;
  }

}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
