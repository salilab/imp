/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/particle_io.h>
#include <RMF/FileHandle.h>
#include <RMF/NodeHandle.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>

IMPRMF_BEGIN_NAMESPACE

namespace {
  class ParticleLoadLink: public SimpleLoadLink<Particle> {
    typedef SimpleLoadLink<Particle> P;
    RMF::Category cat_;
    Pointer<Model> m_;
    compatibility::map<RMF::FloatKey, FloatKey> float_;
    compatibility::map<RMF::IntKey, IntKey> int_;
    compatibility::map<RMF::StringKey, StringKey> string_;

    template <class IK, class RK>
    void load_keys(RMF::FileConstHandle fh,
                   RMF::Category cat,
                   compatibility::map<RK, IK> &map) {
      typedef typename RK::TypeTraits Traits;
      RMF::vector<RK> ks= fh.get_keys<Traits, 1>(cat);
      for (unsigned int i=0; i< ks.size(); ++i) {
        IK ik(fh.get_name(ks[i]));
        map[ks[i]]= ik;
        IMP_LOG(TERSE, "Found "
                << ks[i]
                << " with " << ik
                << std::endl);
      }
      for (typename compatibility::map<RK, IK>::const_iterator
             it= map.begin(); it != map.end(); ++it) {
        IMP_LOG(TERSE, "Added key assoc "
                << fh.get_name(it->first)
                << " with " << it->second
                << std::endl);
      }
    }
    template <class IK, class RK>
    void load_one(Particle *o,
                  RMF::NodeConstHandle nh,
                  const compatibility::map<RK, IK> &map,
                  unsigned int frame) {
      /*RMF::show_hierarchy_with_values(nh,
        frame);*/
      for (typename compatibility::map<RK, IK>::const_iterator
             it= map.begin(); it != map.end(); ++it) {
        /*std::cout << "Checking for " << it->second
                  << " " << it->first.get_is_per_frame()
                  << std::endl;*/
        if (nh.get_has_value(it->first, frame)) {
          IK ik= it->second;
          /*std::cout << "Value for " << it->first
            << " to " << it->second << std::endl;*/
          if (o->has_attribute(ik)) {
            o->set_value(ik,
                         nh.get_value(it->first, frame));
          } else {
            o->add_attribute(ik,
                             nh.get_value(it->first, frame));
          }
        } else {
          if (o->has_attribute(it->second)) {
            o->remove_attribute(it->second);
          }
          //std::cout << "No value for " << it->first << std::endl;
        }
      }
    }
    void do_load_one( RMF::NodeConstHandle nh,
                      Particle *o,
                      unsigned int frame) {
      load_one(o, nh, float_, frame);
      load_one(o, nh, int_, frame);
      load_one(o, nh, string_, frame);
    }
    bool get_is(RMF::NodeConstHandle nh) const {
      return nh.get_type()==RMF::CUSTOM;
    }
    Particle* do_create(RMF::NodeConstHandle name) {
      return new Particle(m_, name.get_name());
    }
  public:
    ParticleLoadLink(RMF::FileConstHandle fh, Model *m):
      P("ParticleLoadLink%1%") {
      cat_=fh.get_category<1>("IMP");
      m_=m;
      if (cat_ != RMF::Category()) {
        load_keys(fh, cat_, float_);
        load_keys(fh, cat_, int_);
        load_keys(fh, cat_, string_);
      }
    }
    IMP_OBJECT_INLINE(ParticleLoadLink,IMP_UNUSED(out),);
  };

  class ParticleSaveLink: public SimpleSaveLink<Particle> {
    typedef SimpleSaveLink<Particle> P;
    RMF::Category cat_;
    compatibility::map<FloatKey, RMF::FloatKey> float_;
    compatibility::map<IntKey, RMF::IntKey> int_;
    compatibility::map<StringKey, RMF::StringKey> string_;
    template <class IK, class RK>
    void save_one(Particle *o,
                  const base::Vector<IK> &ks,
                  RMF::NodeHandle nh,
                  compatibility::map<IK, RK> &map,
                  unsigned int frame) {
      for (unsigned int i=0; i< ks.size(); ++i) {
        if (map.find(ks[i]) == map.end()) {
          map[ks[i]]
            = RMF::get_key_always<typename RK::TypeTraits>(nh.get_file(),
                                                           cat_,
                                                           ks[i].get_string(),
                                                           true);
        }
        nh.set_value(map.find(ks[i])->second, o->get_value(ks[i]), frame);
      }
    }

    void do_save_one(Particle *o,
                     RMF::NodeHandle nh,
                     unsigned int frame) {
      save_one(o, o->get_float_keys(), nh, float_, frame);
      save_one(o, o->get_int_keys(), nh, int_, frame);
      save_one(o, o->get_string_keys(), nh, string_, frame);
    }
    RMF::NodeType get_type(Particle*) const {
      return RMF::CUSTOM;
    }
  public:
    ParticleSaveLink(RMF::FileHandle fh): P("ParticleSaveLink%1%"){
      cat_=RMF::get_category_always<1>(fh, "IMP");
    }
    IMP_OBJECT_INLINE(ParticleSaveLink,IMP_UNUSED(out),);
  };
}


IMP_DEFINE_LINKERS(Particle, particle, particles,
                   Particle*, ParticlesTemp,
                   Particle*, ParticlesTemp,
                   (RMF::FileHandle fh),
                   (RMF::FileConstHandle fh,
                    Model *m), (fh), (fh, m),
                   (fh, IMP::internal::get_model(hs)));

IMPRMF_END_NAMESPACE
