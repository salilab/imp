/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
    Pointer<Model> m_;

    template <class IK, class RK>
    void load_keys(RMF::FileConstHandle fh,
                   RMF::Category cat,
                   base::map<RK, IK> &map) {
      typedef typename RK::TypeTraits Traits;
      std::vector<RK> ks= fh.get_keys<Traits>(cat);
      for (unsigned int i=0; i< ks.size(); ++i) {
        IK ik(fh.get_name(ks[i]));
        map[ks[i]]= ik;
        IMP_LOG_TERSE( "Found "
                << ks[i]
                << " with " << ik
                << std::endl);
      }
      for (typename base::map<RK, IK>::const_iterator
             it= map.begin(); it != map.end(); ++it) {
        IMP_LOG_TERSE( "Added key assoc "
                << fh.get_name(it->first)
                << " with " << it->second
                << std::endl);
      }
    }
    template <class IK, class RK>
    void load_one(Particle *o,
                  RMF::NodeConstHandle nh,
                  RMF::Category cat) {
      base::map<RK, IK> map;
      load_keys(nh.get_file(), cat, map);
      /*RMF::show_hierarchy_with_values(nh,
        frame);*/
      for (typename base::map<RK, IK>::const_iterator
               it= map.begin(); it != map.end(); ++it) {
        if (nh.get_has_value(it->first)) {
          IK ik= it->second;
          if (o->has_attribute(ik)) {
            o->set_value(ik,
                         nh.get_value(it->first));
          } else {
            o->add_attribute(ik,
                             nh.get_value(it->first));
          }
        } else {
          if (o->has_attribute(it->second)) {
            o->remove_attribute(it->second);
          }
          std::cout << "No value for " << it->first << std::endl;
        }
      }
    }
    void do_load_one( RMF::NodeConstHandle nh,
                      Particle *o) {
      RMF::Category cat=nh.get_file().get_category("IMP");
      load_one<IMP::FloatKey, RMF::FloatKey>(o, nh, cat);
      load_one<IMP::IntKey, RMF::IntKey>(o, nh, cat);
      load_one<IMP::StringKey, RMF::StringKey>(o, nh, cat);
    }
    bool get_is(RMF::NodeConstHandle nh) const {
      return nh.get_type()==RMF::CUSTOM;
    }
    Particle* do_create(RMF::NodeConstHandle name) {
      return new Particle(m_, name.get_name());
    }
  public:
    ParticleLoadLink(RMF::FileConstHandle , Model *m):
      P("ParticleLoadLink%1%") {

      m_=m;
    }
    IMP_OBJECT_INLINE(ParticleLoadLink,IMP_UNUSED(out),);
  };

  class ParticleSaveLink: public SimpleSaveLink<Particle> {
    typedef SimpleSaveLink<Particle> P;
    RMF::Category cat_;
    base::map<FloatKey, RMF::FloatKey> float_;
    base::map<IntKey, RMF::IntKey> int_;
    base::map<StringKey, RMF::StringKey> string_;
    template <class IK, class RK>
    void save_one(Particle *o,
                  const base::Vector<IK> &ks,
                  RMF::NodeHandle nh,
                  base::map<IK, RK> &map) {
      for (unsigned int i=0; i< ks.size(); ++i) {
        if (map.find(ks[i]) == map.end()) {
          map[ks[i]]
              = nh.get_file().get_key<typename RK::TypeTraits>(cat_,
                                                            ks[i].get_string());
        }
        nh.set_value(map.find(ks[i])->second, o->get_value(ks[i]));
      }
    }

    void do_save_one(Particle *o,
                     RMF::NodeHandle nh) {
      save_one(o, o->get_float_keys(), nh, float_);
      save_one(o, o->get_int_keys(), nh, int_);
      save_one(o, o->get_string_keys(), nh, string_);
    }
    RMF::NodeType get_type(Particle*) const {
      return RMF::CUSTOM;
    }
  public:
    ParticleSaveLink(RMF::FileHandle fh): P("ParticleSaveLink%1%"){
      cat_=fh.get_category("IMP");
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
