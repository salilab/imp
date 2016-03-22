/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_AVRO_IO_H
#define RMF_AVRO_IO_H

#include "RMF/config.h"
#include "backend/IO.h"
#include "data_file.h"
#include "internal/shared_data_maps.h"
#include "traits.h"
#include "types.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/shared_data_ranges.h"
#include <avrocpp/api/Compiler.hh>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro2 {

template <class RW>
struct Avro2IO : public backends::IO {
  RW rw_;
  avro2::FileData file_data_;
  bool file_data_dirty_;
  avro2::FileDataChanges file_data_changes_;
  avro2::Frame frame_;
  void commit();
  unsigned int get_number_of_frames() const;

 public:
  template <class T>
  Avro2IO(T t);
  virtual void save_loaded_frame(internal::SharedData *shared_data)
      RMF_OVERRIDE;
  virtual void load_loaded_frame(internal::SharedData *shared_data)
      RMF_OVERRIDE;
  virtual void save_static_frame(internal::SharedData *shared_data)
      RMF_OVERRIDE;
  virtual void load_static_frame(internal::SharedData *shared_data)
      RMF_OVERRIDE;
  virtual void load_file(internal::SharedData *shared_data) RMF_OVERRIDE;
  virtual void save_file(const internal::SharedData *shared_data) RMF_OVERRIDE;
  virtual void load_hierarchy(internal::SharedData *shared_data) RMF_OVERRIDE;
  virtual void save_hierarchy(const internal::SharedData *shared_data)
      RMF_OVERRIDE;
  virtual void flush() RMF_OVERRIDE;
  virtual ~Avro2IO();
};

namespace {

template <class Traits, class Loader>
void load(internal::SharedData *shared_data, const KeyMaps<Traits> &keys,
          internal::TypeData<Traits> &data, Loader) {
  typedef std::pair<ID<Traits>, Category> KKP;
  RMF_FOREACH(KKP kp, keys.category) {
    shared_data->ensure_key(kp.second, kp.first,
                            keys.name.find(kp.first)->second, Traits());
  }
  if (!data.empty()) {
    RMF_TRACE("Found data for " << data.size() << " nodes.");
  }
  Loader::access_data(shared_data, Traits()).swap(data);
}

template <class Traits, class Loader>
bool save(KeyType key_type, internal::SharedData *shared_data,
          KeyMaps<Traits> &keys, std::vector<KeyInfo> *keys_changed,
          internal::TypeData<Traits> &data,
          internal::TypeData<Traits> *change_data, Loader) {
  bool ret = false;
  RMF_FOREACH(ID<Traits> k, shared_data->get_keys(Traits())) {
    if (keys.category.find(k) == keys.category.end()) {
      ret = true;
      std::string name = shared_data->get_name(k);
      keys.category[k] = shared_data->get_category(k);
      keys.name[k] = name;
      KeyInfo ki;
      ki.name = name;
      ki.category = shared_data->get_category(k);
      ki.id = k.get_index();
      ki.type = key_type;
      keys_changed->push_back(ki);
    } else {
      RMF_INTERNAL_CHECK(
          keys.name.find(k) != keys.name.end() &&
              keys.name.find(k)->second == shared_data->get_name(k),
          "Names don't match");
    }
  }

  if (change_data) {
    RMF_FOREACH(typename internal::TypeData<Traits>::const_reference kpd,
                Loader::get_data(shared_data, Traits())) {
      if (data.find(kpd.first) == data.end() && !kpd.second.empty()) {
        ret = true;
        change_data->operator[](kpd.first) = kpd.second;
        data[kpd.first] = kpd.second;
      } else {
        RMF_FOREACH(typename internal::KeyData<Traits>::const_reference npd,
                    kpd.second) {
          if (data[kpd.first].find(npd.first) == data[kpd.first].end()) {
            ret = true;
            change_data->operator[](kpd.first)[npd.first] = npd.second;
            data[kpd.first][npd.first] = npd.second;
          }
        }
      }
    }
  } else {
    std::swap(data, Loader::access_data(shared_data, Traits()));
  }

  return ret;
}

template <class Loader>
void load_all(const std::vector<std::pair<Category, std::string> > &categories,
              internal::SharedData *shared_data, const KeyData &keys,
              DataTypes &data, Loader) {
  typedef std::pair<Category, std::string> CP;
  RMF_FOREACH(CP cp, categories) {
    shared_data->ensure_category(cp.first, cp.second);
  }
  load(shared_data, keys.float_keys, data.float_data, Loader());
  load(shared_data, keys.int_keys, data.int_data, Loader());
  load(shared_data, keys.string_keys, data.string_data, Loader());
  load(shared_data, keys.floats_keys, data.floats_data, Loader());
  load(shared_data, keys.ints_keys, data.ints_data, Loader());
  load(shared_data, keys.strings_keys, data.strings_data, Loader());

  load(shared_data, keys.vector3_keys, data.vector3_data, Loader());
  load(shared_data, keys.vector4_keys, data.vector4_data, Loader());
  load(shared_data, keys.vector3s_keys, data.vector3s_data, Loader());
}

template <class Loader>
bool save_all(FileData &file_data, FileDataChanges &file_data_changes,
              internal::SharedData *shared_data, DataTypes &data,
              DataTypes *data_changes, Loader) {
  bool ret = false;
  Categories categories = shared_data->get_categories();
  for (unsigned int i = file_data.categories.size(); i < categories.size();
       ++i) {
    ret = true;
    file_data.categories.push_back(
        std::make_pair(categories[i], shared_data->get_name(categories[i])));
    file_data_changes.categories.push_back(file_data.categories.back());
  }

  ret = save(FLOAT, shared_data, file_data.keys.float_keys,
             &file_data_changes.keys, data.float_data,
             data_changes ? &data_changes->float_data : NULL, Loader()) ||
        ret;
  ret = save(INT, shared_data, file_data.keys.int_keys, &file_data_changes.keys,
             data.int_data, data_changes ? &data_changes->int_data : NULL,
             Loader()) ||
        ret;
  ret = save(STRING, shared_data, file_data.keys.string_keys,
             &file_data_changes.keys, data.string_data,
             data_changes ? &data_changes->string_data : NULL, Loader()) ||
        ret;
  ret = save(FLOATS, shared_data, file_data.keys.floats_keys,
             &file_data_changes.keys, data.floats_data,
             data_changes ? &data_changes->floats_data : NULL, Loader()) ||
        ret;
  ret = save(INTS, shared_data, file_data.keys.ints_keys,
             &file_data_changes.keys, data.ints_data,
             data_changes ? &data_changes->ints_data : NULL, Loader()) ||
        ret;
  ret = save(STRINGS, shared_data, file_data.keys.strings_keys,
             &file_data_changes.keys, data.strings_data,
             data_changes ? &data_changes->strings_data : NULL, Loader()) ||
        ret;

  ret = save(VECTOR3, shared_data, file_data.keys.vector3_keys,
             &file_data_changes.keys, data.vector3_data,
             data_changes ? &data_changes->vector3_data : NULL, Loader()) ||
        ret;
  ret = save(VECTOR4, shared_data, file_data.keys.vector4_keys,
             &file_data_changes.keys, data.vector4_data,
             data_changes ? &data_changes->vector4_data : NULL, Loader()) ||
        ret;
  ret = save(VECTOR3S, shared_data, file_data.keys.vector3s_keys,
             &file_data_changes.keys, data.vector3s_data,
             data_changes ? &data_changes->vector3s_data : NULL, Loader()) ||
        ret;

  return ret;
}
}

template <class RW>
void Avro2IO<RW>::commit() {
  if (file_data_dirty_) {
    rw_.write(file_data_changes_);
    file_data_dirty_ = false;
    file_data_changes_ = FileDataChanges();
  }
  if (frame_.id != FrameID()) {
    rw_.write(frame_);
    frame_.id = FrameID();
  }
}

// Woah
template <class RW>
template <class T>
Avro2IO<RW>::Avro2IO(T t)
    : rw_(t), file_data_dirty_(false) {}

template <class RW>
void Avro2IO<RW>::save_loaded_frame(internal::SharedData *shared_data) {
  if (frame_.id != FrameID()) {
    rw_.write(frame_);
  }
  FrameID id = shared_data->get_loaded_frame();
  frame_ = avro2::Frame();
  frame_.id = id;
  const internal::FrameData &fd = shared_data->get_frame_data(id);
  frame_.parents = FrameIDs(fd.parents.begin(), fd.parents.end());
  frame_.type = fd.type;
  frame_.name = fd.name;
  save_all(file_data_, file_data_changes_, shared_data, frame_.data, NULL,
           internal::LoadedValues());
}

template <class RW>
void Avro2IO<RW>::load_loaded_frame(internal::SharedData *shared_data) {
  FrameID id = shared_data->get_loaded_frame();
  rw_.load_frame(file_data_, frame_.id, id, frame_);
  load_all(file_data_.categories, shared_data, file_data_.keys, frame_.data,
           internal::LoadedValues());
}

template <class RW>
void Avro2IO<RW>::load_static_frame(internal::SharedData *shared_data) {
  load_all(file_data_.categories, shared_data, file_data_.keys, file_data_.data,
           internal::StaticValues());
}

template <class RW>
void Avro2IO<RW>::save_static_frame(internal::SharedData *shared_data) {
  bool changed =
      save_all(file_data_, file_data_changes_, shared_data, file_data_.data,
               &file_data_changes_.data, internal::StaticValues());
  file_data_dirty_ = changed || file_data_dirty_;
}

template <class RW>
void Avro2IO<RW>::load_file(internal::SharedData *shared_data) {
  // set producer and description
  // for some weird reason, mac os 10.8 clang needs this two step thing
  rw_.load_file_data(file_data_);
  RMF_INFO("Found " << get_number_of_frames() << " frames");
  shared_data->set_description(file_data_.description);
  shared_data->set_producer(file_data_.producer);
  typedef std::pair<FrameID, internal::FrameData> FDP;
  RMF_FOREACH(FDP fdp, file_data_.frames) {
    shared_data->add_frame_data(fdp.first, fdp.second.name, fdp.second.type);
    RMF_FOREACH(FrameID p, fdp.second.parents) {
      shared_data->add_child_frame(p, fdp.first);
    }
  }
  shared_data->set_file_type("rmf3");

  typedef std::pair<Category, std::string> CP;
  RMF_FOREACH(CP cp, file_data_.categories) {
    shared_data->ensure_category(cp.first, cp.second);
  }
}

template <class RW>
void Avro2IO<RW>::save_file(const internal::SharedData *shared_data) {
  if (shared_data->get_description() != file_data_.description) {
    file_data_changes_.description = shared_data->get_description();
    file_data_.description.set(shared_data->get_description());
    file_data_dirty_ = true;
  }
  if (shared_data->get_producer() != file_data_.producer) {
    file_data_changes_.producer = shared_data->get_producer();
    file_data_.producer.set(shared_data->get_producer());
    file_data_dirty_ = true;
  }
}

template <class RW>
void Avro2IO<RW>::load_hierarchy(internal::SharedData *shared_data) {
  using namespace std;
  swap(shared_data->access_node_hierarchy(), file_data_.nodes);
}

template <class RW>
void Avro2IO<RW>::save_hierarchy(const internal::SharedData *shared_data) {
  RMF_FOREACH(NodeID n, get_nodes(shared_data)) {
    HierarchyNode cur;
    cur.id = n;
    bool cur_dirty = false;
    if (file_data_.nodes.size() <= n.get_index()) {
      cur_dirty = true;
      file_data_.nodes.resize(n.get_index() + 1);
      file_data_.nodes[n.get_index()].type = shared_data->get_type(n);
      cur.type = shared_data->get_type(n);
    }
    std::string name = shared_data->get_name(n);
    if (file_data_.nodes[n.get_index()].name != name) {
      cur_dirty = true;
      file_data_.nodes[n.get_index()].name = name;
      cur.name = name;
    }
    if (shared_data->get_parents(n).size() !=
        file_data_.nodes[n.get_index()].parents.size()) {
      cur_dirty = true;
      NodeIDs cur_parents = shared_data->get_parents(n);
      NodeIDs new_parents(
          cur_parents.begin() + file_data_.nodes[n.get_index()].parents.size(),
          cur_parents.end());
      file_data_.nodes[n.get_index()].parents = cur_parents;
      cur.parents = new_parents;
    }
    if (cur_dirty) {
      file_data_changes_.nodes.push_back(cur);
      file_data_dirty_ = true;
    }
  }
}

template <class RW>
void Avro2IO<RW>::flush() {
  commit();
}

template <class RW>
unsigned int Avro2IO<RW>::get_number_of_frames() const {
  if (file_data_.max_id == FrameID())
    return 0;
  else
    return file_data_.max_id.get_index() + 1;
}

template <class RW>
Avro2IO<RW>::~Avro2IO() {
  commit();
}

}  // namespace avro2
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif  /* RMF_AVRO_IO_H */
