/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_BACKWARDS_IO_H
#define RMF_INTERNAL_BACKWARDS_IO_H

#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include "internal/clone_shared_data.h"
#include <algorithm>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "IO.h"
#include "KeyFilter.h"
#include "RMF/BufferConstHandle.h"
#include "RMF/BufferHandle.h"
#include "RMF/ID.h"
#include "RMF/Vector.h"
#include "RMF/config.h"
#include "RMF/enums.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/log.h"
#include "RMF/types.h"
#include "backend/IO.h"
#include "backend/backward_types.h"
#include "internal/clone_shared_data.h"
#include "internal/shared_data_maps.h"

RMF_ENABLE_WARNINGS namespace RMF {

  namespace backends {

  typedef RMF_LARGE_UNORDERED_MAP<std::string, boost::array<std::string, 3> >
      V3N;
  extern V3N vector_3_names_map;
  extern V3N vectors_3_names_map;
  typedef RMF_LARGE_UNORDERED_MAP<std::string, boost::array<std::string, 4> >
      V4N;
  extern V4N vector_4_names_map;

  inline V3N &get_vector_names_map(RMF_VECTOR<3>) { return vector_3_names_map; }
  inline V4N &get_vector_names_map(RMF_VECTOR<4>) { return vector_4_names_map; }
  inline V3N &get_vectors_names_map(RMF_VECTOR<3>) {
    return vectors_3_names_map;
  }

  template <class SD>
  struct BackwardsIO : public IO {
    boost::scoped_ptr<SD> sd_;
    std::string name_;

    template <class Traits, class SDC>
    ID<Traits> get_key_const(Category cat, std::string name, Traits,
                             SDC *sd) const {
      RMF_FOREACH(ID<Traits> k, sd->get_keys(cat, Traits())) {
        if (sd->get_name(k) == name) return k;
      }
      return ID<Traits>();
    }

    template <VectorDimension D, class Filter>
    void filter_vector(Filter &filter, Category cat) const {
      RMF_FOREACH(std::string key_name,
                  get_vector_names(cat, RMF_VECTOR<D>())) {
        RMF_FOREACH(std::string subkey_name,
                    get_vector_subkey_names(key_name, RMF_VECTOR<D>())) {
          filter.add_float_key(cat, subkey_name);
          RMF_TRACE("Filtering " << subkey_name);
        }
      }
    }

    template <VectorDimension D, class Filter>
    void filter_vectors(Filter &filter, Category cat) const {
      RMF_FOREACH(std::string key_name,
                  get_vectors_names(cat, RMF_VECTOR<D>())) {
        RMF_FOREACH(std::string subkey_name,
                    get_vectors_subkey_names(key_name, RMF_VECTOR<D>())) {
          filter.add_floats_key(cat, subkey_name);
          RMF_TRACE("Filtering " << subkey_name);
        }
      }
    }

    template <VectorDimension D>
    inline boost::array<std::string, D> get_vector_subkey_names(
        std::string key_name, RMF_VECTOR<D>) const {
      typename RMF_LARGE_UNORDERED_MAP<
          std::string, boost::array<std::string, D> >::const_iterator it =
          get_vector_names_map(RMF_VECTOR<D>()).find(key_name);
      if (it == get_vector_names_map(RMF_VECTOR<D>()).end()) {
        boost::array<std::string, D> ret;
        for (unsigned int i = 0; i < D; ++i) {
          std::ostringstream ossk;
          ossk << "_" << key_name << "_" << i;
          ret[i] = ossk.str();
        }
        return ret;
      } else {
        return it->second;
      }
    }

    template <VectorDimension D>
    inline boost::array<std::string, D> get_vectors_subkey_names(
        std::string key_name, RMF_VECTOR<D>) const {
      typename RMF_LARGE_UNORDERED_MAP<
          std::string, boost::array<std::string, D> >::const_iterator it =
          get_vectors_names_map(RMF_VECTOR<D>()).find(key_name);
      if (it == get_vectors_names_map(RMF_VECTOR<D>()).end()) {
        boost::array<std::string, D> ret;
        for (unsigned int i = 0; i < D; ++i) {
          std::ostringstream ossk;
          ossk << "_" << key_name << "_" << i;
          ret[i] = ossk.str();
        }
        return ret;
      } else {
        return it->second;
      }
    }

    template <VectorDimension D>
    inline Strings get_vector_names(Category cat, RMF_VECTOR<D>) const {
      std::ostringstream oss;
      oss << "_vector" << D;
      Strings ret;
      StringsKey key =
          get_key_const(cat, oss.str(), StringsTraits(), sd_.get());
      if (key != StringsKey()) {
        ret = sd_->get_static_value(NodeID(0), key);
      }
      typedef std::pair<std::string, boost::array<std::string, D> > KP;
      RMF_FOREACH(KP kp, get_vector_names_map(RMF_VECTOR<D>())) {
        ret.push_back(kp.first);
      }
      std::sort(ret.begin(), ret.end());
      ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
      return ret;
    }

    template <VectorDimension D>
    inline Strings get_vectors_names(Category cat, RMF_VECTOR<D>) const {
      std::ostringstream oss;
      oss << "_vectors" << D;
      Strings ret;
      StringsKey key =
          get_key_const(cat, oss.str(), StringsTraits(), sd_.get());
      if (key != StringsKey()) {
        ret = sd_->get_static_value(NodeID(0), key);
      }
      typedef std::pair<std::string, boost::array<std::string, D> > KP;
      RMF_FOREACH(KP kp, get_vectors_names_map(RMF_VECTOR<D>())) {
        ret.push_back(kp.first);
      }
      std::sort(ret.begin(), ret.end());
      ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
      return ret;
    }

    template <VectorDimension D, class SDB, class H>
    void load_vector(SDB *sdb, Category category_b, H) {
      typedef ID<Traits<Vector<D> > > Key;
      typedef boost::tuple<Key, int> Data;
      RMF_LARGE_UNORDERED_MAP<FloatKey, Data> map;
      RMF_FOREACH(std::string key_name,
                  get_vector_names(category_b, RMF_VECTOR<D>())) {
        boost::array<std::string, D> subkey_names =
            get_vector_subkey_names(key_name, RMF_VECTOR<D>());
        for (unsigned int i = 0; i < D; ++i) {
          FloatKey cur_key =
              sdb->get_key(category_b, subkey_names[i], FloatTraits());
          map[cur_key].template get<0>() =
              sdb->get_key(category_b, key_name, Traits<Vector<D> >());
          map[cur_key].template get<1>() = i;
        }
      }
      if (map.empty()) return;
      typedef std::pair<FloatKey, Data> KP;
      RMF_FOREACH(KP kp, map) {
        RMF_FOREACH(NodeID n, internal::get_nodes(sdb)) {
          double v = H::get(sdb, n, kp.first);
          if (!FloatTraits::get_is_null_value(v)) {
            RMF_VECTOR<D> &old = H::access(sdb, n, kp.second.template get<0>());
            old[kp.second.template get<1>()] = v;
            H::unset(sdb, n, kp.first);
          }
        }
      }
    }
    template <VectorDimension D, class SDA, class SDB, class H>
    void save_vector(SDA *sda, Category category_a, SDB *sdb,
                     Category category_b, H) {
      typedef ID<Traits<Vector<D> > > VectorKey;
      std::vector<VectorKey> keys =
          sda->get_keys(category_a, Traits<Vector<D> >());
      typedef boost::array<ID<FloatTraits>, D> Data;
      RMF_LARGE_UNORDERED_MAP<VectorKey, Data> map;
      Strings key_names;
      RMF_FOREACH(VectorKey k, keys) {
        std::string name = sda->get_name(k);
        key_names.push_back(name);
        boost::array<std::string, D> subkey_names =
            get_vector_subkey_names(name, RMF_VECTOR<D>());
        for (unsigned int i = 0; i < D; ++i) {
          map[k][i] = sdb->get_key(category_b, subkey_names[i], FloatTraits());
        }
      }
      if (key_names.empty()) return;
      {
        std::ostringstream oss;
        oss << "_vector" << D;
        StringsKey k = sdb->get_key(category_b, oss.str(), StringsTraits());
        sdb->set_static_value(NodeID(0), k, key_names);
      }

      typedef std::pair<VectorKey, Data> KP;
      RMF_FOREACH(KP kp, map) {
        RMF_FOREACH(NodeID n, internal::get_nodes(sda)) {
          RMF_VECTOR<D> v = H::get(sda, n, kp.first);
          if (!Traits<Vector<D> >::get_is_null_value(v)) {
            for (unsigned int i = 0; i < D; ++i) {
              H::set(sdb, n, kp.second[i], v[i]);
            }
          }
        }
      }
    }

    template <class SDB, class H>
    void load_vectors(SDB *sdb, Category category_b, H) {
      typedef Vector3sKey Key;
      typedef boost::tuple<Key, int> Data;
      RMF_LARGE_UNORDERED_MAP<FloatsKey, Data> map;
      RMF_FOREACH(std::string key_name,
                  get_vectors_names(category_b, RMF_VECTOR<3>())) {
        boost::array<std::string, 3> subkey_names =
            get_vectors_subkey_names(key_name, RMF_VECTOR<3>());
        for (unsigned int i = 0; i < 3; ++i) {
          FloatsKey cur_key =
              sdb->get_key(category_b, subkey_names[i], FloatsTraits());
          map[cur_key].template get<0>() =
              sdb->get_key(category_b, key_name, Vector3sTraits());
          map[cur_key].template get<1>() = i;
        }
      }
      if (map.empty()) return;
      typedef std::pair<FloatsKey, Data> KP;
      RMF_FOREACH(KP kp, map) {
        RMF_FOREACH(NodeID n, internal::get_nodes(sdb)) {
          Floats v = H::get(sdb, n, kp.first);
          if (!v.empty()) {
            std::vector<RMF_VECTOR<3> > &old =
                H::access(sdb, n, kp.second.template get<0>());
            old.resize(v.size());
            for (unsigned int i = 0; i < v.size(); ++i) {
              old[i][kp.second.get<1>()] = v[i];
            }
            H::unset(sdb, n, kp.first);
          }
        }
      }
    }

    template <class SDA, class SDB, class H>
    void save_vectors(SDA *sda, Category category_a, SDB *sdb,
                      Category category_b, H) {
      typedef Vector3sKey VectorKey;
      std::vector<VectorKey> keys = sda->get_keys(category_a, Vector3sTraits());
      typedef boost::array<ID<FloatsTraits>, 3> Data;
      RMF_LARGE_UNORDERED_MAP<VectorKey, Data> map;
      Strings key_names;
      RMF_FOREACH(VectorKey k, keys) {
        std::string name = sda->get_name(k);
        key_names.push_back(name);
        boost::array<std::string, 3> subkey_names =
            get_vectors_subkey_names(name, RMF_VECTOR<3>());
        for (unsigned int i = 0; i < 3; ++i) {
          map[k][i] = sdb->get_key(category_b, subkey_names[i], FloatsTraits());
        }
      }
      if (key_names.empty()) return;
      {
        std::ostringstream oss;
        oss << "_vectors" << 3;
        StringsKey k = sdb->get_key(category_b, oss.str(), StringsTraits());
        sdb->set_static_value(NodeID(0), k, key_names);
      }

      typedef std::pair<VectorKey, Data> KP;
      RMF_FOREACH(KP kp, map) {
        RMF_FOREACH(NodeID n, internal::get_nodes(sda)) {
          std::vector<RMF_VECTOR<3> > v = H::get(sda, n, kp.first);
          if (!v.empty()) {
            for (unsigned int i = 0; i < 3; ++i) {
              Floats cur(v.size());
              for (unsigned int j = 0; j < v.size(); ++j) {
                cur[j] = v[j][i];
              }
              H::set(sdb, n, kp.second[i], cur);
            }
          }
        }
      }
    }

    template <class SDA>
    ID<backward_types::NodeIDTraits> get_alias_key(const SDA *a) {
      Category alias_cat;
      RMF_FOREACH(Category cur_cat, a->get_categories()) {
        if (a->get_name(cur_cat) == "alias") {
          alias_cat = cur_cat;
        }
      }
      if (alias_cat == Category()) return ID<backward_types::NodeIDTraits>();

      ID<backward_types::NodeIDTraits> alias_key;
      RMF_FOREACH(ID<backward_types::NodeIDTraits> nik_cur,
                  a->get_keys(alias_cat, backward_types::NodeIDTraits())) {
        if (a->get_name(nik_cur) == "aliased") {
          alias_key = nik_cur;
        }
      }
      return alias_key;
    }

    template <class SDA, class SDB>
    void load_restraints(const SDA *a, SDB *b) {
      ID<backward_types::NodeIDTraits> alias_key = get_alias_key(a);
      if (alias_key == ID<backward_types::NodeIDTraits>()) {
        RMF_INFO("No alias key found, skipping restraint processing");
        return;
      }

      Category feature_category = b->get_category("feature");
      IntsKey rep_key =
          b->get_key(feature_category, "representation", IntsTraits());

      RMF_FOREACH(NodeID n, internal::get_nodes(b)) {
        if (b->get_type(n) == FEATURE) {
          RMF_TRACE("Processing restraint node " << b->get_name(n));
          // make a copy as it can change
          NodeIDs chs = b->get_children(n);
          Ints val;
          RMF_FOREACH(NodeID ch, chs) {
            if (b->get_type(ch) == ALIAS) {
              RMF_TRACE("Found alias child " << b->get_name(ch));
              val.push_back(a->get_static_value(ch, alias_key).get_index());
              b->remove_child(n, ch);
            }
          }
          if (!val.empty()) {
            b->set_static_value(n, rep_key, val);
          }
        }
      }
    }

    template <class SDA, class SDB>
    void load_bonds(const SDA *a, SDB *b) {

      ID<backward_types::NodeIDTraits> alias_key = get_alias_key(a);
      if (alias_key == ID<backward_types::NodeIDTraits>()) return;

      Category bond_cat = b->get_category("bond");
      IntKey k0 = b->get_key(bond_cat, "bonded 0", IntTraits());
      IntKey k1 = b->get_key(bond_cat, "bonded 1", IntTraits());
      RMF_FOREACH(NodeID n, internal::get_nodes(a)) {
        if (a->get_type(n) == BOND) {
          NodeIDs ch = a->get_children(n);
          if (ch.size() == 2 && a->get_type(ch[0]) == ALIAS &&
              b->get_type(ch[1]) == ALIAS) {
            b->set_static_value(
                n, k0, a->get_static_value(ch[0], alias_key).get_index());
            b->set_static_value(
                n, k1, a->get_static_value(ch[1], alias_key).get_index());
            b->remove_child(n, ch[0]);
            b->remove_child(n, ch[1]);
          }
        }
      }
    }

    template <class H>
    void load_frame_category(Category category,
                             internal::SharedData *shared_data, H) {
      Category file_cat = sd_->get_category(shared_data->get_name(category));
      KeyFilter<SD> filter(sd_.get());
      if (shared_data->get_name(category) == "sequence") {
        filter.add_index_key(file_cat, "chain id");
      }
      /*if (shared_data->get_name(category) == "shape") {
        filter.add_float_key(file_cat, "rbg color blue");
      }
      filter_vector<3>(filter, file_cat);
      filter_vector<4>(filter, file_cat);
      filter_vectors<3>(filter, file_cat);*/
      internal::clone_values_type<IntTraits, IntTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<backward_types::IndexTraits, IntTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<backward_types::NodeIDTraits, IntTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<FloatTraits, FloatTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<StringTraits, StringTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<IntsTraits, IntsTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<backward_types::IndexesTraits, IntsTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<backward_types::NodeIDsTraits, IntsTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<FloatsTraits, FloatsTraits>(
          &filter, file_cat, shared_data, category, H());
      internal::clone_values_type<StringsTraits, StringsTraits>(
          &filter, file_cat, shared_data, category, H());

      if (shared_data->get_name(category) == "sequence") {
        backward_types::IndexKey cidk = get_key_const(
            file_cat, "chain id", backward_types::IndexTraits(), sd_.get());
        if (cidk != backward_types::IndexKey()) {
          StringKey cidsk =
              shared_data->get_key(category, "chain id", StringTraits());
          RMF_FOREACH(NodeID ni, internal::get_nodes(shared_data)) {
            int ci = H::get(sd_.get(), ni, cidk);
            if (!backward_types::IndexTraits::get_is_null_value(ci)) {
              H::set(shared_data, ni, cidsk, std::string(1, ci = 'A'));
            }
          }
        }
        StringKey rtk = get_key_const(category, "residue type", StringTraits(),
                                      shared_data);
        IntKey bk = get_key_const(category, "first residue index", IntTraits(),
                                  shared_data);
        IntKey ek = get_key_const(category, "last residue index", IntTraits(),
                                  shared_data);
        if (rtk != StringKey() && bk != IntKey() && ek != IntKey()) {
          IntKey rik =
              shared_data->get_key(category, "residue index", IntTraits());
          RMF_FOREACH(NodeID ni, internal::get_nodes(shared_data)) {
            std::string rt = H::get(shared_data, ni, rtk);
            if (!rt.empty()) {
              int b = H::get(shared_data, ni, bk);
              if (!backward_types::IndexTraits::get_is_null_value(b)) {
                H::set(shared_data, ni, rik, b);
                H::unset(shared_data, ni, bk);
                H::unset(shared_data, ni, ek);
              }
            }
          }
        }
      }
      if (shared_data->get_name(category) == "shape") {
        FloatKey bk = get_key_const(category, "rbg color blue", FloatTraits(),
                                    shared_data);
        FloatKey nbk =
            shared_data->get_key(category, "rgb color blue", FloatTraits());
        if (bk != FloatKey()) {
          RMF_FOREACH(NodeID ni, internal::get_nodes(shared_data)) {
            float v = H::get(shared_data, ni, bk);
            if (!FloatTraits::get_is_null_value(v)) {
              H::set(shared_data, ni, nbk, v);
              H::unset(shared_data, ni, bk);
            }
          }
        }
      }
      load_vector<3>(shared_data, category, H());
      load_vector<4>(shared_data, category, H());
      load_vectors(shared_data, category, H());
    }
    template <class H>
    void save_frame_category(Category category,
                             const internal::SharedData *shared_data, H) {
      Category file_cat = sd_->get_category(shared_data->get_name(category));
      internal::clone_values_type<IntTraits, IntTraits>(
          shared_data, category, sd_.get(), file_cat, H());
      internal::clone_values_type<FloatTraits, FloatTraits>(
          shared_data, category, sd_.get(), file_cat, H());
      internal::clone_values_type<StringTraits, StringTraits>(
          shared_data, category, sd_.get(), file_cat, H());
      internal::clone_values_type<IntsTraits, IntsTraits>(
          shared_data, category, sd_.get(), file_cat, H());
      internal::clone_values_type<FloatsTraits, FloatsTraits>(
          shared_data, category, sd_.get(), file_cat, H());
      internal::clone_values_type<StringsTraits, StringsTraits>(
          shared_data, category, sd_.get(), file_cat, H());

      save_vector<3>(shared_data, category, sd_.get(), file_cat, H());
      save_vector<4>(shared_data, category, sd_.get(), file_cat, H());
      save_vectors(shared_data, category, sd_.get(), file_cat, H());
    }

   public:
    BackwardsIO(std::string name, bool create, bool read_only)
        : sd_(new SD(name, create, read_only)), name_(name) {}
    BackwardsIO(BufferConstHandle buffer)
        : sd_(new SD(buffer)), name_("buffer") {}
    BackwardsIO(BufferHandle buffer) : sd_(new SD(buffer)), name_("buffer") {}
    virtual ~BackwardsIO() { flush(); }

   protected:
    virtual void load_static_frame(internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_FOREACH(Category category, shared_data->get_categories()) {
        load_frame_category(category, shared_data, internal::StaticValues());
      }

      load_bonds(sd_.get(), shared_data);
      load_restraints(sd_.get(), shared_data);
    }

    virtual void save_static_frame(internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_FOREACH(Category category, shared_data->get_categories()) {
        save_frame_category(category, shared_data, internal::StaticValues());
      }
    }

    virtual void load_file(internal::SharedData *shared_data) RMF_OVERRIDE {
      sd_->reload();
      RMF_INFO("Loading file");
      RMF::internal::clone_file(sd_.get(), shared_data);
      shared_data->set_file_type(sd_->get_file_type());
      RMF_FOREACH(FrameID id, internal::get_frames(sd_.get())) {
        shared_data->add_frame_data(id, "", FRAME);
      }
      RMF_TRACE("Found " << sd_->get_number_of_frames() << " frames.");
      RMF_FOREACH(Category c, sd_->get_categories()) {
        shared_data->get_category(sd_->get_name(c));
      }
    }

    virtual void save_file(const internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_INFO("Saving file");
      RMF::internal::clone_file(shared_data, sd_.get());
      flush();
    }

    virtual void save_loaded_frame(internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_INFO("Saving frame " << shared_data->get_loaded_frame());
      FrameID cur = shared_data->get_loaded_frame();
      RMF_USAGE_CHECK(cur.get_index() == sd_->get_number_of_frames(),
                      "Saving a frame that is not the next one");
      // ignore nesting relationships for now
      if (cur.get_index() >= sd_->get_number_of_frames()) {
        RMF_TRACE("Adding new frame for " << cur);
        FrameID nfid = sd_->add_frame(shared_data->get_frame_data(cur).name,
                                      shared_data->get_frame_data(cur).type);
        RMF_UNUSED(nfid);
        RMF_INTERNAL_CHECK(nfid == cur, "Number of frames don't match.");
      }
      sd_->set_loaded_frame(cur);
      RMF_FOREACH(Category category, shared_data->get_categories()) {
        save_frame_category(category, shared_data, internal::LoadedValues());
      }
    }

    virtual void load_loaded_frame(internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_INFO("Loading frame " << shared_data->get_loaded_frame());
      FrameID cur = shared_data->get_loaded_frame();
      sd_->set_loaded_frame(cur);
      RMF_FOREACH(Category category, sd_->get_categories()) {
        load_frame_category(category, shared_data, internal::LoadedValues());
      }
    }

    virtual void load_hierarchy(internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_INFO("Loading hierarchy");
      RMF::internal::clone_hierarchy(sd_.get(), shared_data);
    }

    virtual void save_hierarchy(const internal::SharedData *shared_data)
        RMF_OVERRIDE {
      RMF_INFO("Saving hierarchy");
      RMF::internal::clone_hierarchy(shared_data, sd_.get());
    }

    virtual void flush() RMF_OVERRIDE { sd_->flush(); }
  };

  }  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_BACKWARDS_IO_H */
