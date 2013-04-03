/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SINGLE_AVRO_FILE_WRITER_H
#define RMF_INTERNAL_SINGLE_AVRO_FILE_WRITER_H

#include <RMF/config.h>
#include <RMF/internal/SharedData.h>
#include <RMF/infrastructure_macros.h>
#include "MultipleAvroFileBase.h"
#include <RMF/exceptions.h>
#include <backend/avro/AvroCpp/api/DataFile.hh>
#include <backend/avro/AllJSON.h>
#include <backend/avro/FrameJSON.h>
#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

class MultipleAvroFileWriter: public MultipleAvroFileBase {
  bool file_dirty_;
  bool nodes_dirty_;
  bool frames_dirty_;
  std::vector<bool> static_categories_dirty_;

  struct CategoryData {
    boost::shared_ptr<rmf_avro::DataFileWriter<RMF_avro_backend::Data > > writer;
    RMF_avro_backend::Data data;
    bool dirty;
  };

  std::vector<CategoryData> categories_;

  RMF_avro_backend::Data null_frame_data_;
  RMF_avro_backend::Data null_static_frame_data_;
  boost::shared_ptr<rmf_avro::DataFileWriter<RMF_avro_backend::Frame> > frame_writer_;
  RMF_avro_backend::Frame frame_;
protected:
  const RMF_avro_backend::Data &get_frame_data(Category cat,
                                           int      frame) const {
    if (frame == ALL_FRAMES) {
      if (static_categories_.size() > cat.get_id()) {
        return static_categories_[cat.get_id()];
      } else {
        return null_static_data_;
      }
    } else {
      if (categories_.size() > cat.get_id()) {
        return categories_[cat.get_id()].data;
      } else {
        return null_data_;
      }
    }
  }

  RMF_avro_backend::Data &access_frame_data(Category cat,
                                        int      frame) {
    if (frame == ALL_FRAMES) {
      if (static_categories_.size() <= cat.get_id()) {
        RMF_avro_backend::Data data;
        data.frame = ALL_FRAMES;
        static_categories_.resize(cat.get_id() + 1, data);
        static_categories_dirty_.resize(cat.get_id() + 1, false);
      }
      static_categories_dirty_[cat.get_id()] = true;
      return static_categories_[cat.get_id()];
    } else {
      while (categories_.size() <= cat.get_id()) {
        categories_.push_back(CategoryData());
        categories_.back().dirty = false;
        categories_.back().data.frame = frame;
      }
      categories_[cat.get_id()].dirty = true;
      return categories_[cat.get_id()].data;
    }
  }

  RMF_avro_backend::Node &access_node(unsigned int node) {
    nodes_dirty_ = true;
    if (nodes_.size() <= node) {
      nodes_.resize(node + 1);
    }
    return nodes_[node];
  }

  RMF_avro_backend::File &access_file() {
    file_dirty_ = true;
    return file_;
 }
  void commit();
public:
  void flush() {
  }
  void reload() {
    RMF_THROW(Message("Can't reload writable file"), UsageException);
  }

  MultipleAvroFileWriter(std::string path,
                         bool create, bool read_only);
  virtual ~MultipleAvroFileWriter();

  void set_current_frame(int frame);


  int add_child_frame(int node, std::string name, int t);
  void add_child_frame(int node, int child_node);
  Ints get_children_frame(int node) const ;

  std::string get_frame_name(int i) const;
  unsigned int get_number_of_frames() const;
};

}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_MULTIPLE_AVRO_FILE_WRITER_H */
