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
#include <avro/DataFile.hh>
#include <backend/avro/AllJSON.h>
#include <boost/shared_ptr.hpp>

namespace RMF {
namespace internal {

class MultipleAvroFileWriter: public MultipleAvroFileBase {
  bool file_dirty_;
  bool nodes_dirty_;
  bool frames_dirty_;
  vector<bool> static_categories_dirty_;

  struct CategoryData {
    boost::shared_ptr<avro::DataFileWriter<RMF_internal::Data > > writer;
    RMF_internal::Data data;
    bool dirty;
  };

  vector<CategoryData> categories_;

  RMF_internal::Data null_frame_data_;
  RMF_internal::Data null_static_frame_data_;
protected:
  const RMF_internal::Data &get_frame_data(Category cat,
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

  RMF_internal::Data &access_frame_data(Category cat,
                                        int      frame) {
    if (frame == ALL_FRAMES) {
      if (static_categories_.size() <= cat.get_id()) {
        RMF_internal::Data data;
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

  RMF_internal::Node &access_node(unsigned int node) {
    nodes_dirty_ = true;
    if (nodes_.size() <= node) {
      nodes_.resize(node + 1);
    }
    return nodes_[node];
  }

  RMF_internal::File &access_file() {
    file_dirty_ = true;
    return file_;
  }
  RMF_internal::Node& access_frame(int i) {
    frames_dirty_ = true;
    if (static_cast<int>(frames_.size()) <= i + 1) {
      // we are adding a new frame, commit old data
      commit();
      frames_.resize(i + 2);
    }
    return frames_[i + 1];
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

};

}   // namespace internal
} /* namespace RMF */


#endif /* RMF_INTERNAL_MULTIPLE_AVRO_FILE_WRITER_H */
