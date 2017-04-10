/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_MULTIPLE_AVRO_FILE_BASE_H
#define RMF_INTERNAL_MULTIPLE_AVRO_FILE_BASE_H

#include "AllJSON.h"
#include <string>
#include <vector>

#include "AvroKeysAndCategories.h"
#include "RMF/ID.h"
#include "RMF/config.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

class MultipleAvroFileBase : public AvroKeysAndCategories {
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

  const RMF_avro_backend::Node& get_node(NodeID node) const {
    return nodes_[node.get_index()];
  }

  const std::vector<RMF_avro_backend::Node>& get_nodes_data() const {
    return nodes_;
  }

  const RMF_avro_backend::File& get_file() const { return file_; }

  const RMF_avro_backend::Data& get_static_data(Category cat) const {
    return static_categories_[cat.get_index()];
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

 public:
  std::string get_file_type() const { return "Multiple avro version 1"; }

  void set_loaded_frame(FrameID frame);

  unsigned int get_number_of_nodes() const { return nodes_.size(); }

  MultipleAvroFileBase(std::string path);
};

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_MULTIPLE_AVRO_FILE_BASE_H */
