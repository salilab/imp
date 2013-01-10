/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SINGLE_AVRO_FILE_BASE_H
#define RMF_INTERNAL_SINGLE_AVRO_FILE_BASE_H

#include <RMF/config.h>
#include "AvroKeysAndCategories.h"
#include <backend/avro/AllJSON.h>
namespace RMF {
namespace internal {

/* Later have laze and non-lazy frame loading so we can skip check on most
   fetches.

   split into two classes, one for creating and one for reading (with a common base)

   push get_all into base classes, then we can specialize it for the reader and
   do everything nicely with changing on set_current_frame

   merge all the static data (file, nodes, frames, base structure) into one record
   share that. Only have the dynamic stuff be different between backends

   can build cache on node keys later.
 */
class MultipleAvroFileBase: public AvroKeysAndCategories {
protected:
  typedef RMF_internal::File File;
  File file_;
  typedef RMF::vector<RMF_internal::Node> Nodes;
  Nodes nodes_;
  typedef RMF::vector<RMF_internal::Node > Frames;
  Frames frames_;
  typedef vector<RMF_internal::Data> StaticData;
  StaticData static_categories_;

  RMF_internal::Node null_frame_data_;

  RMF_internal::Data null_data_;
  RMF_internal::Data null_static_data_;



  const RMF_internal::Node &get_node(unsigned int node) const {
    return nodes_[node];
  }

  const RMF::vector<RMF_internal::Node> &get_nodes_data() const {
    return nodes_;
  }

  const RMF_internal::File &get_file() const {
    return file_;
  }

  const RMF_internal::Node& get_frame(int i) const {
    if (i + 1 >= static_cast<int>(frames_.size())) {
      return null_frame_data_;
    }
    return frames_[i + 1];
  }

  const std::vector<RMF_internal::Node>& get_frames() const {
    return frames_;
  }

  const RMF_internal::Data& get_static_data(Category cat) const {
    return static_categories_[cat.get_id()];
  }

  void initialize_node_keys() {
    clear_node_keys();
    for (unsigned int i = 0; i < nodes_.size(); ++i) {
      add_node_key();
    }
  }

  std::string get_category_dynamic_file_path(Category cat) const;
  std::string get_category_static_file_path(Category cat) const;
  std::string get_file_file_path() const;
  std::string get_nodes_file_path() const;
  std::string get_static_file_path() const;
  std::string get_frames_file_path() const;

  void set_current_frame(int frame) {
    null_data_.frame = frame;
    AvroKeysAndCategories::set_current_frame(frame);
  }

  MultipleAvroFileBase(std::string path);
};

}   // namespace internal
} /* namespace RMF */

#endif /* RMF_INTERNAL_MULTIPLE_AVRO_FILE_H */
