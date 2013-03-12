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

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

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
  typedef RMF_avro_backend::File File;
  File file_;
  typedef std::vector<RMF_avro_backend::Node> Nodes;
  Nodes nodes_;
  typedef std::vector<RMF_avro_backend::Data> StaticData;
  StaticData static_categories_;

  RMF_avro_backend::Node null_frame_data_;

  RMF_avro_backend::Data null_data_;
  RMF_avro_backend::Data null_static_data_;



  const RMF_avro_backend::Node &get_node(unsigned int node) const {
    return nodes_[node];
  }

  const std::vector<RMF_avro_backend::Node> &get_nodes_data() const {
    return nodes_;
  }

  const RMF_avro_backend::File &get_file() const {
    return file_;
  }

  const RMF_avro_backend::Data& get_static_data(Category cat) const {
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

  void set_current_frame(int frame);

  MultipleAvroFileBase(std::string path);
};

}   // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_MULTIPLE_AVRO_FILE_H */
