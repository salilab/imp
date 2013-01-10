/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SINGLE_AVRO_FILE_H
#define RMF_INTERNAL_SINGLE_AVRO_FILE_H

#include <RMF/config.h>
#include <RMF/internal/SharedData.h>
#include <RMF/infrastructure_macros.h>
#include "AvroKeysAndCategories.h"
#include <backend/avro/AllJSON.h>

namespace RMF {
namespace internal {

class SingleAvroFile: public AvroKeysAndCategories {
  RMF_internal::All all_;
  bool dirty_;


  std::string *buffer_;
  bool write_to_buffer_;

  RMF_internal::Data null_frame_data_;
  RMF_internal::Data null_static_frame_data_;

  RMF_internal::Node null_frame_real_data_;

  // begin specific data
protected:
  const RMF_internal::Data &get_frame_data(Category cat,
                                           int      frame) const {
    std::string category = get_category_name(cat);
    std::map<std::string, std::vector<RMF_internal::Data > >::const_iterator
      it = all_.category.find(category);
    if (it == all_.category.end()) {
      return frame == ALL_FRAMES ? null_static_frame_data_ : null_frame_data_;
    }
    if (it->second.size() <= (frame + 1)) {
      return frame == ALL_FRAMES ? null_static_frame_data_ : null_frame_data_;
    } else {
      return it->second[frame + 1];
    }
  }

  RMF_internal::Data &access_frame_data(Category cat,
                                        int      frame) {
    std::string category = get_category_name(cat);
    dirty_ = true;
    while (all_.category[category].size() <= (frame + 1)) {
      int cur = all_.category[category].size() - 1;
      all_.category[category].push_back(RMF_internal::Data());
      all_.category[category].back().frame = cur;
    }
    return all_.category[category][frame + 1];
  }

  const RMF_internal::Node &get_node(unsigned int node) const {
    return all_.nodes[node];
  }

  const std::vector<RMF_internal::Node> &get_nodes_data() const {
    return all_.nodes;
  }

  RMF_internal::Node &access_node(unsigned int node) {
    dirty_ = true;
    if (all_.nodes.size() <= node) {
      all_.nodes.resize(node + 1);
    }
    return all_.nodes[node];
  }

  const RMF_internal::File &get_file() const {
    return all_.file;
  }

  RMF_internal::File &access_file() {
    dirty_ = true;
    return all_.file;
  }

  const RMF_internal::Node& get_frame(int i) const {
    if ( i + 1 >= static_cast<int>(all_.frames.size())) {
      return null_frame_real_data_;
    }
    return all_.frames[i + 1];
  }

  const std::vector< RMF_internal::Node>& get_frames() const {
    return all_.frames;
  }

  RMF_internal::Node& access_frame(int i) {
    dirty_ = true;
    if (static_cast<int>(all_.frames.size()) <= i + 1) {
      RMF_internal::Node def;
      def.type = "frame";
      all_.frames.resize(i + 2, def);
    }
    return all_.frames[i + 1];
  }

  void initialize_categories();
  void initialize_node_keys();
public:

  void set_current_frame(int frame) {
    null_frame_data_.frame = frame;
    AvroKeysAndCategories::set_current_frame(frame);
  }

  void flush();
  void reload();

  SingleAvroFile(std::string path, bool create,
                 bool read_only);
  SingleAvroFile(std::string &buffer, bool create,
                 bool read_only, bool);
  ~SingleAvroFile() {
    flush();
  }

};

}   // namespace internal
} /* namespace RMF */


#include "AvroSharedData.impl.h"

#endif /* RMF_INTERNAL_SINGLE_AVRO_FILE_H */
