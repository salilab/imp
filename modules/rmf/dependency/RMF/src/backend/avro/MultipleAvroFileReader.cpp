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

namespace RMF {
namespace internal {

namespace {
void clear_data(RMF_internal::Data &data,
                int                frame) {
  data = RMF_internal::Data();
  data.frame = frame;
}
}

void MultipleAvroFileReader::set_current_frame(int frame) {
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
          categories_[i].reader
          .reset( new avro::DataFileReader<RMF_internal::Data >(name.c_str(),
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
          clear_data(categories_[i].data, frame);
          break;
        } else {
          //std::cout << "Read frame " << categories_[i].data.frame << std::endl;
          //show(categories_[i].data, std::cout);
        }
        if (frame < categories_[i].data.frame) {
          //std::cout << "Missing frame looking for " << frame << std::endl;
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
    if (a->path().extension() == ".frames" || a->path().extension() == ".static") {
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



#define RMF_RELOAD(UCName, lcname)                                        \
  {                                                                       \
    bool success;                                                         \
    try {                                                                 \
      avro::DataFileReader<UCName> re(get_##lcname##_file_path().c_str(), \
                                      get_##UCName##_schema());           \
      success = re.read(lcname##_);                                       \
    } catch (const std::exception &e) {                                   \
      RMF_THROW(Message(e.what())                                         \
                << Component(get_##lcname##_file_path()), IOException);   \
    }                                                                     \
    if (!success) {                                                       \
      RMF_THROW(Message("Error parsing data")                             \
                << Component(get_##lcname##_file_path()), IOException);   \
    }                                                                     \
  }

void MultipleAvroFileReader::reload() {
  RMF_RELOAD(File,  file);
  RMF_RELOAD(Nodes, nodes);
  RMF_RELOAD(Nodes, frames);

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
      categories_[cat.get_id()].reader
      .reset(new avro::DataFileReader<RMF_internal::Data>(dynamic_path.c_str(),
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
      avro::DataFileReader<RMF_internal::Data> reader(static_path.c_str(),
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

}   // namespace internal
} /* namespace RMF */
