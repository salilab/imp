/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/internal/imp_operations.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>
#include <RMF/decorators.h>
#include <IMP/scoped.h>
#include <IMP/base/ConstArray.h>
#include <IMP/base/WeakPointer.h>
#include <boost/shared_array.hpp>
IMPRMF_BEGIN_NAMESPACE

double RMFRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  set_was_used(true);
  return get_last_score();
}
ParticlesTemp RMFRestraint::get_input_particles() const {
  return ps_;
}
void RMFRestraint::set_decomposition(const RMFRestraints &d) {
  decomp_=d;
  for (unsigned int i=0; i< decomp_.size(); ++i) {
    decomp_[i]->set_was_used(true);
  }
}
ContainersTemp RMFRestraint::get_input_containers() const {
  return ContainersTemp();
}
void RMFRestraint::do_show(std::ostream &) const {
}
Restraints RMFRestraint::do_create_current_decomposition() const {
  set_was_used(true);
  return decomp_;
}
RMFRestraint::RMFRestraint(Model *m, std::string name): Restraint(m, name){}
namespace {
  class Subset: public base::ConstArray<base::WeakPointer<Particle>,
                                        Particle*> {
    typedef base::ConstArray<base::WeakPointer<Particle>, Particle* > P;
    static const ParticlesTemp &get_sorted(ParticlesTemp &ps) {
      std::sort(ps.begin(), ps.end());
      ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
      return ps;
    }
  public:
    Subset(){}
    /** Construct a subset from a non-empty list of particles.
     */
    explicit Subset(ParticlesTemp ps): P(get_sorted(ps)) {
    }
    Model *get_model() const {
      return operator[](0)->get_model();
    }
    std::string get_name() const;
    bool get_contains(const Subset &o) const {
      return std::includes(begin(), end(), o.begin(), o.end());
    }
  };

  IMP_VALUES(Subset, Subsets);
  IMP_SWAP(Subset);


  ParticlesTemp get_particles(RMF::FileConstHandle ,
                              const RMF::NodeConstHandles &ids) {
    ParticlesTemp ret;
    for (unsigned int i=0; i< ids.size(); ++i) {
      Particle *p= get_association<Particle>(ids[i]);
      IMP_USAGE_CHECK(p, "Must associate particles first");
      ret.push_back(p);
    }
    return ret;
  }

  template <class C>
  RMF::NodeConstHandles get_node_ids(RMF::FileConstHandle fh,
                                     const C &ps) {
    RMF::NodeConstHandles ret;
    for (unsigned int i=0; i< ps.size(); ++i) {
      RMF::NodeConstHandle nh=get_node_from_association(fh, ps[i]);
      if (nh != RMF::NodeConstHandle()) {
        ret.push_back(nh);
      } else {
        IMP_WARN("Particle " << Showable(ps[i])
                 << " is not in the RMF." << std::endl);
      }
    }
    return ret;
  }

  struct RestraintSaveData {
    compatibility::map<Subset, RMF::NodeID> map_;
  };

  RMF::NodeHandle get_node(Subset s, RestraintSaveData &d,
                           RMF::ScoreFactory sf,
                           RMF::NodeHandle parent) {
    if (d.map_.find(s) == d.map_.end()) {
      RMF::NodeHandle n= parent.add_child("subset", RMF::FEATURE);
      d.map_[s]=n.get_id();
      RMF::Score csd= sf.get(n, 0);
      csd.set_representation(get_node_ids(parent.get_file(), s));
    }
    return parent.get_file().get_node_from_id(d.map_.find(s)->second);
  }

  // get_particles
  //
  class RestraintLoadLink: public SimpleLoadLink<RMFRestraint> {
    typedef SimpleLoadLink<RMFRestraint> P;
    RMF::ScoreConstFactory sf_;
    Model *m_;

    void do_load_one( RMF::NodeConstHandle nh,
                      RMFRestraint *o,
                      unsigned int frame) {
      RMF::ScoreConst d= sf_.get(nh, frame);
      o->set_score(d.get_score());
      o->set_particles(get_particles(nh.get_file(),
                                     d.get_representation()));
      RMF::NodeConstHandles ch= nh.get_children();
      RMFRestraints subs;
      for (unsigned int i=0; i< ch.size(); ++i) {
        if (sf_.get_is(ch[i], frame)) {
          RMF::ScoreConst sd= sf_.get(ch[i], frame);
          IMP_NEW(RMFRestraint, s, (m_, ch[i].get_name()));
          subs.push_back(s);
          s->set_score(sd.get_score());
          s->set_particles(get_particles(nh.get_file(),
                                         sd.get_representation()));
        }
      }
      o->set_decomposition(subs);
   }
    bool get_is(RMF::NodeConstHandle nh) const {
      return nh.get_type()==RMF::FEATURE;
    }
    RMFRestraint* do_create(RMF::NodeConstHandle name) {
      return new RMFRestraint(m_, name.get_name());
    }
  public:
    RestraintLoadLink(RMF::FileConstHandle fh, Model *m):
        P("RestraintLoadLink%1%"), sf_(fh), m_(m) {
    }
    IMP_OBJECT_INLINE(RestraintLoadLink,IMP_UNUSED(out),);
  };


  class RestraintSaveLink: public SimpleSaveLink<Restraint> {
    typedef SimpleSaveLink<Restraint> P;
    RMF::ScoreFactory sf_;
    compatibility::map<Restraint*, RestraintSaveData> data_;


    void do_save_one(Restraint *o,
                     RMF::NodeHandle nh,
                     unsigned int frame) {
      RestraintSaveData &d= data_[o];
      RMF::Score sd= sf_.get(nh, frame);
      double score=o->get_last_score();
     if (sd.get_representation().empty()) {
       sd.set_representation(get_node_ids(nh.get_file(),
                                           o->get_input_particles()));
      }
      // only set score if it is valid
      if (score < std::numeric_limits<double>::max()) {
        sd.set_score(score);

        base::Pointer<Restraint> rd= o->create_current_decomposition();
        if (rd && rd != o) {
          rd->set_was_used(true);
          rd->unprotected_evaluate(nullptr);
          RestraintsTemp rs= IMP::get_restraints(RestraintsTemp(1,rd));
          for (unsigned int i=0; i< rs.size(); ++i) {
            Subset s(rs[i]->get_input_particles());
            double score= rs[i]->unprotected_evaluate(nullptr);
            rs[i]->set_was_used(true);
            if (score != 0) {
              RMF::NodeHandle nnh= get_node(s, d, sf_, nh);
              RMF::Score csd= sf_.get(nnh, frame);
              csd.set_score(score);
              //csd.set_representation(get_node_ids(nh.get_file(), s));
            }
          }
        }
      }
    }
    RMF::NodeType get_type(Restraint*) const {
      return RMF::FEATURE;
    }
  public:
    RestraintSaveLink(RMF::FileHandle fh): P("RestraintSaveLink%1%"),
                                           sf_(fh) {
    }
    IMP_OBJECT_INLINE(RestraintSaveLink,IMP_UNUSED(out),);
  };


}

IMP_DEFINE_LINKERS(Restraint, restraint, restraints,
                   RMFRestraint*, RMFRestraints,
                   Restraint*, RestraintsTemp,
                   (RMF::FileHandle fh),
                   (RMF::FileConstHandle fh,
                    Model *m), (fh), (fh, m),
                   (fh, IMP::internal::get_model(hs)));



IMPRMF_END_NAMESPACE
