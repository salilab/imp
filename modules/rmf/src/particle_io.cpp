/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/particle_io.h>
#include <RMF/FileHandle.h>
#include <RMF/NodeHandle.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>
#include <boost/unordered_map.hpp>

IMPRMF_BEGIN_NAMESPACE

namespace {
class ParticleLoadLink : public SimpleLoadLink<kernel::Particle> {
  typedef SimpleLoadLink<kernel::Particle> P;

  template <class IK, class RK>
  void load_keys(RMF::FileConstHandle fh, RMF::Category cat,
                 boost::unordered_map<RK, IK> &map) {
    typedef typename RK::Tag Traits;
    std::vector<RK> ks = fh.get_keys<Traits>(cat);
    for (unsigned int i = 0; i < ks.size(); ++i) {
      IK ik(fh.get_name(ks[i]));
      map[ks[i]] = ik;
      IMP_LOG_TERSE("Found " << ks[i] << " with " << ik << std::endl);
    }
    for (typename boost::unordered_map<RK, IK>::const_iterator it = map.begin();
         it != map.end(); ++it) {
      IMP_LOG_TERSE("Added key assoc " << fh.get_name(it->first) << " with "
                                       << it->second << std::endl);
    }
  }
  template <class IK, class RK>
  void load_one(kernel::Particle *o, RMF::NodeConstHandle nh,
                RMF::Category cat) {
    boost::unordered_map<RK, IK> map;
    load_keys(nh.get_file(), cat, map);
    /*RMF::show_hierarchy_with_values(nh,
      frame);*/
    for (typename boost::unordered_map<RK, IK>::const_iterator it = map.begin();
         it != map.end(); ++it) {
      if (nh.get_has_value(it->first)) {
        IK ik = it->second;
        if (o->has_attribute(ik)) {
          o->set_value(ik, nh.get_value(it->first));
        } else {
          o->add_attribute(ik, nh.get_value(it->first));
        }
      } else {
        if (o->has_attribute(it->second)) {
          o->remove_attribute(it->second);
        }
      }
    }
  }
  void do_load_one(RMF::NodeConstHandle nh, kernel::Particle *o) {
    RMF::Category cat = nh.get_file().get_category("IMP");
    load_one<IMP::FloatKey, RMF::FloatKey>(o, nh, cat);
    load_one<IMP::IntKey, RMF::IntKey>(o, nh, cat);
    load_one<IMP::StringKey, RMF::StringKey>(o, nh, cat);
  }
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type() == RMF::CUSTOM;
  }
  using P::do_create;
  kernel::Particle *do_create(RMF::NodeConstHandle name, kernel::Model *m) {
    return new kernel::Particle(m, name.get_name());
  }

 public:
  ParticleLoadLink(RMF::FileConstHandle) : P("ParticleLoadLink%1%") {}
  static const char *get_name() { return "particle load"; }
  IMP_OBJECT_METHODS(ParticleLoadLink);
};

class ParticleSaveLink : public SimpleSaveLink<kernel::Particle> {
  typedef SimpleSaveLink<kernel::Particle> P;
  RMF::Category cat_;
  boost::unordered_map<FloatKey, RMF::FloatKey> float_;
  boost::unordered_map<IntKey, RMF::IntKey> int_;
  boost::unordered_map<StringKey, RMF::StringKey> string_;
  template <class IK, class RK>
  void save_one(kernel::Particle *o, const base::Vector<IK> &ks,
                RMF::NodeHandle nh, boost::unordered_map<IK, RK> &map) {
    for (unsigned int i = 0; i < ks.size(); ++i) {
      if (map.find(ks[i]) == map.end()) {
        map[ks[i]] =
            nh.get_file().get_key<typename RK::Tag>(cat_, ks[i].get_string());
      }
      nh.set_value(map.find(ks[i])->second, o->get_value(ks[i]));
    }
  }

  void do_save_one(kernel::Particle *o, RMF::NodeHandle nh) {
    save_one(o, o->get_float_keys(), nh, float_);
    save_one(o, o->get_int_keys(), nh, int_);
    save_one(o, o->get_string_keys(), nh, string_);
  }
  RMF::NodeType get_type(kernel::Particle *) const { return RMF::CUSTOM; }

 public:
  ParticleSaveLink(RMF::FileHandle fh) : P("ParticleSaveLink%1%") {
    cat_ = fh.get_category("IMP");
  }
  static const char *get_name() { return "particle save"; }
  IMP_OBJECT_METHODS(ParticleSaveLink);
};
}

IMP_DEFINE_LINKERS(Particle, particle, particles, kernel::Particle *,
                   kernel::ParticlesTemp,
                   (RMF::FileConstHandle fh, kernel::Model *m), (fh, m));

IMPRMF_END_NAMESPACE
