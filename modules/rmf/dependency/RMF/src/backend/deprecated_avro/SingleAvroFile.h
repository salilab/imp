/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SINGLE_AVRO_FILE_H
#define RMF_INTERNAL_SINGLE_AVRO_FILE_H

#include "AllJSON.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <utility>
#include <vector>

#include "AvroKeysAndCategories.h"
#include "RMF/ID.h"
#include "RMF/config.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {

class SingleAvroFile : public AvroKeysAndCategories {
  RMF_avro_backend::All all_;
  bool dirty_;
  bool text_;

  boost::shared_ptr<std::vector<char> > buffer_;

  RMF_avro_backend::Data null_frame_data_;
  RMF_avro_backend::Data null_static_frame_data_;

  RMF_avro_backend::Node null_frame_real_data_;

  // begin specific data
 protected:
  const RMF_avro_backend::Data& get_frame_data(Category cat,
                                               FrameID frame) const {
    std::string category = get_name(cat);
    std::map<std::string, std::vector<RMF_avro_backend::Data> >::const_iterator
        it = all_.category.find(category);
    if (it == all_.category.end()) {
      return frame == ALL_FRAMES ? null_static_frame_data_ : null_frame_data_;
    }
    unsigned int index;
    if (frame == ALL_FRAMES)
      index = 0;
    else
      index = frame.get_index() + 1;
    if (it->second.size() <= static_cast<unsigned int>(index)) {
      return frame == ALL_FRAMES ? null_static_frame_data_ : null_frame_data_;
    } else {
      return it->second[index];
    }
  }

  RMF_avro_backend::Data& access_frame_data(Category cat, FrameID frame) {
    std::string category = get_name(cat);
    dirty_ = true;
    unsigned int index;
    if (frame == ALL_FRAMES)
      index = 0;
    else
      index = frame.get_index() + 1;
    while (all_.category[category].size() <= static_cast<unsigned int>(index)) {
      int cur = all_.category[category].size() - 1;
      all_.category[category].push_back(RMF_avro_backend::Data());
      all_.category[category].back().frame = cur;
    }
    return all_.category[category][index];
  }

  const RMF_avro_backend::Node& get_node(NodeID node) const {
    return all_.nodes[node.get_index()];
  }

  const std::vector<RMF_avro_backend::Node>& get_nodes_data() const {
    return all_.nodes;
  }

  RMF_avro_backend::Node& access_node(NodeID node) {
    dirty_ = true;
    if (all_.nodes.size() <= static_cast<unsigned int>(node.get_index())) {
      all_.nodes.resize(node.get_index() + 1);
    }
    return all_.nodes[node.get_index()];
  }

  const RMF_avro_backend::File& get_file() const { return all_.file; }

  RMF_avro_backend::File& access_file() {
    dirty_ = true;
    return all_.file;
  }

  const RMF_avro_backend::Node& get_frame(FrameID i) const {
    if (i.get_index() + 1 >= all_.frames.size()) {
      return null_frame_real_data_;
    }
    return all_.frames[i.get_index() + 1];
  }

  const std::vector<RMF_avro_backend::Node>& get_frames() const {
    return all_.frames;
  }

  RMF_avro_backend::Node& access_frame(FrameID i) {
    dirty_ = true;
    unsigned int index;
    if (i == ALL_FRAMES) {
      index = 0;
    } else {
      index = i.get_index() + 1;
    }
    if (all_.frames.size() <= index) {
      RMF_avro_backend::Node def;
      def.type = "frame";
      all_.frames.resize(index + 1, def);
    }
    return all_.frames[index];
  }
  void initialize_frames();
  void initialize_categories();
  void initialize_node_keys();
  static bool get_is_text(std::string name) {
    return name[name.size() - 1] == 't';
  }

 public:
  unsigned int get_number_of_nodes() const { return all_.nodes.size(); }

  void set_loaded_frame(FrameID frame) {
    null_frame_data_.frame = frame.get_index();
    AvroKeysAndCategories::set_loaded_frame(frame);
  }

  void flush();
  void reload();

  SingleAvroFile(std::string path, bool create, bool read_only);
  SingleAvroFile(boost::shared_ptr<std::vector<char> > buffer, bool create,
                 bool read_only);
  SingleAvroFile();
  ~SingleAvroFile() { flush(); }
  FrameID add_frame(std::string name, FrameType t);
  void add_child_frame(FrameID child_node);
  FrameIDs get_children(FrameID node) const;
  using AvroKeysAndCategories::get_name;
  std::string get_loaded_frame_name() const;
  FrameType get_loaded_frame_type() const;
  unsigned int get_number_of_frames() const;
  std::string get_file_type() const {
    if (text_) {
      return "Single avro version 1 (text)";
    } else {
      return "Single avro version 1 (binary)";
    }
  }
  std::vector<char> get_buffer();
};

}  // namespace avro_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#include "AvroSharedData.impl.h"

#endif /* RMF_INTERNAL_SINGLE_AVRO_FILE_H */
