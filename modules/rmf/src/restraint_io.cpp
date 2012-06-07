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
namespace {
class RMFRestraint;
IMP_OBJECTS(RMFRestraint, RMFRestraints);
/** A dummy restraint object to represent restraints loaded from
    an RMF file.*/
class IMPRMFEXPORT RMFRestraint: public Restraint {
  ParticlesTemp ps_;
public:
#ifndef IMP_DOXYGEN
  RMFRestraint(Model *m, std::string name);
  void set_particles(const ParticlesTemp &ps) {ps_=ps;}
#endif
  IMP_RESTRAINT(RMFRestraint);
  Restraints do_create_current_decomposition() const;
};

double RMFRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  set_was_used(true);
  return get_last_score();
}
ParticlesTemp RMFRestraint::get_input_particles() const {
  return ps_;
}
ContainersTemp RMFRestraint::get_input_containers() const {
  return ContainersTemp();
}
void RMFRestraint::do_show(std::ostream &) const {
}
Restraints RMFRestraint::do_create_current_decomposition() const {
  set_was_used(true);
  if (get_last_score() != 0) {
    const Restraint *rp= this;
    return Restraints(1, const_cast<Restraint*>(rp));
  } else {
    return Restraints();
  }
}
RMFRestraint::RMFRestraint(Model *m, std::string name): Restraint(m, name){}

  class Subset: public base::ConstArray<base::WeakPointer<Particle>,
                                        Particle*> {
    typedef base::ConstArray<base::WeakPointer<Particle>, Particle* > P;
    static ParticlesTemp get_sorted(ParticlesTemp ps) {
      std::sort(ps.begin(), ps.end());
      ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
      return ps;
    }
  public:
    Subset(){}
    /** Construct a subset from a non-empty list of particles.
     */
    explicit Subset(const ParticlesTemp &ps): P(get_sorted(ps)) {
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
    // must not be a handle so as not to keep things alive
    compatibility::map<Subset, RMF::NodeID> map_;
  };

  RMF::NodeHandle get_node(Subset s, RestraintSaveData &d,
                           RMF::ScoreFactory sf,
                           RMF::NodeHandle parent) {
    if (d.map_.find(s) == d.map_.end()) {
      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        for (compatibility::map<Subset, RMF::NodeID>::const_iterator it
               = d.map_.begin(); it != d.map_.end(); ++it) {
          IMP_INTERNAL_CHECK(it->first != s,
                             "Found!!!!");
        }
      }
      RMF::NodeHandle n= parent.add_child("term", RMF::FEATURE);
      d.map_[s]=n.get_id();
      IMP_INTERNAL_CHECK(d.map_.find(s) != d.map_.end(),
                         "Not found");
      RMF::Score csd= sf.get(n, 0);
      csd.set_representation(get_node_ids(parent.get_file(), s));
    }
    return parent.get_file().get_node_from_id(d.map_.find(s)->second);
  }

  // get_particles
  //
  class RestraintLoadLink: public SimpleLoadLink<Restraint> {
    typedef SimpleLoadLink<Restraint> P;
    RMF::ScoreConstFactory sf_;
    Model *m_;
    RMF::Category imp_cat_;
    RMF::FloatKey weight_key_;

    void do_load_one( RMF::NodeConstHandle nh,
                      Restraint *oi,
                      unsigned int frame) {
      if (sf_.get_is(nh, frame)) {
        RMF::ScoreConst d= sf_.get(nh, frame);
        oi->set_last_score(d.get_score());
      } else {
        oi->set_last_score(0);
      }
    }
    bool get_is(RMF::NodeConstHandle nh) const {
      return nh.get_type()==RMF::FEATURE;
    }
    Restraint* do_create(RMF::NodeConstHandle name) {
      RMF::NodeConstHandles chs= name.get_children();
      Restraints childr;
      for (unsigned int i=0; i < chs.size(); ++i) {
        if (chs[i].get_type() == RMF::FEATURE) {
          childr.push_back(do_create(chs[i]));
          add_link(childr.back(), chs[i]);
        }
      }
      base::Pointer<Restraint> ret;
      if (!childr.empty()) {
        ret= new RestraintSet(childr, 1.0, name.get_name());
      } else {
        ret= new RMFRestraint(m_, name.get_name());
      }
      if (name.get_has_value(weight_key_)) {
        ret->set_weight(name.get_value(weight_key_));
      }
      return ret.release();
    }
  public:
    RestraintLoadLink(RMF::FileConstHandle fh, Model *m):
        P("RestraintLoadLink%1%"), sf_(fh), m_(m),
        imp_cat_(fh.get_category<1>("IMP")),
        weight_key_(fh.get_key<RMF::FloatTraits>(imp_cat_,
                                                 "weight", false)){
    }
    IMP_OBJECT_INLINE(RestraintLoadLink,IMP_UNUSED(out),);
  };


  class RestraintSaveLink: public SimpleSaveLink<Restraint> {
    typedef SimpleSaveLink<Restraint> P;
    RMF::ScoreFactory sf_;
    RMF::Category imp_cat_;
    RMF::FloatKey weight_key_;
    compatibility::map<Restraint*, RestraintSaveData> data_;

    void do_add(Restraint* r, RMF::NodeHandle nh) {
      nh.set_value(weight_key_, r->get_weight());
      RestraintSet* rs= dynamic_cast<RestraintSet*>(r);
      if (rs) {
        for (unsigned int i=0; i< rs->get_number_of_restraints(); ++i) {
          Restraint *rc= rs->get_restraint(i);
          RMF::NodeHandle c= nh.add_child(rc->get_name(), RMF::FEATURE);
          add_link(rc, c);
          do_add(rc, c);
        }
      }
    }
    void do_save_one(Restraint *o,
                     RMF::NodeHandle nh,
                     unsigned int frame) {
      RestraintSaveData &d= data_[o];
      {
        RMF::Score sdnf= sf_.get(nh);
        if (sdnf.get_representation().empty()) {
          // be lazy about it
          ParticlesTemp inputs=o->get_input_particles();
          std::sort(inputs.begin(), inputs.end());
          inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
          RMF::NodeConstHandles nhs=get_node_ids(nh.get_file(),
                                                 inputs);
          sdnf.set_representation(nhs);
          IMP_INTERNAL_CHECK(sdnf.get_representation().size()
                             == nhs.size(), "Get and set values don't match");
        }
      }
      RMF::Score sd= sf_.get(nh, frame);
      double score=o->get_last_score();
      // only set score if it is valid
      if (score < std::numeric_limits<double>::max()) {
        sd.set_score(score);
        if (!dynamic_cast<RestraintSet*>(o)) {
          // required to set last score
          base::Pointer<Restraint> rd= o->create_current_decomposition();
          // set all child scores to 0 for this frame, we will over
          // right below
          RMF::NodeHandles chs= nh.get_children();
          for (unsigned int i=0; i< chs.size(); ++i) {
            if (chs[i].get_type()== RMF::FEATURE) {
              RMF::Score s= sf_.get(chs[i], frame);
              s.set_score(0);
            }
          }
          if (rd && rd != o) {
            rd->set_was_used(true);
            RestraintsTemp rs= IMP::get_restraints(RestraintsTemp(1,rd));
            for (unsigned int i=0; i< rs.size(); ++i) {
              Subset s(rs[i]->get_input_particles());
              double score= rs[i]->get_last_score();
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
    }
    RMF::NodeType get_type(Restraint*) const {
      return RMF::FEATURE;
    }
  public:
    RestraintSaveLink(RMF::FileHandle fh):
        P("RestraintSaveLink%1%"),
        sf_(fh),
        imp_cat_(RMF::get_category_always<1>(fh, "IMP")),
        weight_key_(RMF::get_key_always<RMF::FloatTraits>(fh, imp_cat_,
                                                          "weight",
                                                          false)) {
    }
    IMP_OBJECT_INLINE(RestraintSaveLink,IMP_UNUSED(out),);
  };


}

IMP_DEFINE_LINKERS(Restraint, restraint, restraints,
                   Restraint*, Restraints,
                   Restraint*, RestraintsTemp,
                   (RMF::FileHandle fh),
                   (RMF::FileConstHandle fh,
                    Model *m), (fh), (fh, m),
                   (fh, IMP::internal::get_model(hs)));



IMPRMF_END_NAMESPACE
