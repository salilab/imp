/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_MULTIPLE_AVRO_FILE_WRITER_H
#define RMF_INTERNAL_MULTIPLE_AVRO_FILE_WRITER_H

#include "avrocpp/api/DataFile.hh"
#include "AllJSON.h"
#include "FrameJSON.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "MultipleAvroFileBase.h"
#include "RMF/ID.h"
#include "RMF/config.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"
#include "AvroKeysAndCategories.h"

namespace internal_avro {
template <typename T>
class DataFileWriter;
}  // namespace internal_avro

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

class MultipleAvroFileWriter : public MultipleAvroFileBase {
  bool file_dirty_;
  bool nodes_dirty_;
  bool frames_dirty_;
  std::vector<bool> static_categories_dirty_;

  struct CategoryData {
    boost::shared_ptr<internal_avro::DataFileWriter<RMF_avro_backend::Data> >
        writer;
    RMF_avro_backend::Data data;
    bool dirty;
  };

  std::vector<CategoryData> categories_;

  RMF_avro_backend::Data null_frame_data_;
  RMF_avro_backend::Data null_static_frame_data_;
  boost::shared_ptr<internal_avro::DataFileWriter<RMF_avro_backend::Frame> >
      frame_writer_;
  RMF_avro_backend::Frame frame_;

 protected:
  const RMF_avro_backend::Data& get_frame_data(Category cat,
                                               FrameID frame) const {
    if (frame == ALL_FRAMES) {
      if (static_categories_.size() > cat.get_index()) {
        return static_categories_[cat.get_index()];
      } else {
        return null_static_data_;
      }
    } else {
      if (categories_.size() > cat.get_index()) {
        return categories_[cat.get_index()].data;
      } else {
        return null_data_;
      }
    }
  }

  RMF_avro_backend::Data& access_frame_data(Category cat, FrameID frame) {
    if (frame == ALL_FRAMES) {
      if (static_categories_.size() <= cat.get_index()) {
        RMF_avro_backend::Data data;
        data.frame = -1;
        static_categories_.resize(cat.get_index() + 1, data);
        static_categories_dirty_.resize(cat.get_index() + 1, false);
      }
      static_categories_dirty_[cat.get_index()] = true;
      return static_categories_[cat.get_index()];
    } else {
      while (categories_.size() <= cat.get_index()) {
        categories_.push_back(CategoryData());
        categories_.back().dirty = false;
        categories_.back().data.frame = frame.get_index();
      }
      categories_[cat.get_index()].dirty = true;
      return categories_[cat.get_index()].data;
    }
  }

  RMF_avro_backend::Node& access_node(NodeID node) {
    nodes_dirty_ = true;
    if (nodes_.size() <= static_cast<unsigned int>(node.get_index())) {
      nodes_.resize(node.get_index() + 1);
    }
    return nodes_[node.get_index()];
  }

  RMF_avro_backend::File& access_file() {
    file_dirty_ = true;
    return file_;
  }
  void commit();

 public:
  void flush() {}
  void reload() {
    RMF_THROW(Message("Can't reload writable file"), UsageException);
  }

  MultipleAvroFileWriter(std::string path, bool create, bool read_only);
  ~MultipleAvroFileWriter();

  void set_loaded_frame(FrameID frame);

  FrameID add_frame(std::string name, FrameType t);
  void add_child_frame(FrameID child_node);
  FrameIDs get_children(FrameID node) const;
  using AvroKeysAndCategories::get_name;
  std::string get_loaded_frame_name() const;
  FrameType get_loaded_frame_type() const;
  unsigned int get_number_of_frames() const;
};

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_MULTIPLE_AVRO_FILE_WRITER_H */
