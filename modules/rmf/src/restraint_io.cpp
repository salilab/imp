/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>
#include <RMF/decorators.h>
#include <IMP/core/RestraintsScoringFunction.h>
#include <IMP/scoped.h>
#include <IMP/base/ConstVector.h>
#include <IMP/base/WeakPointer.h>
#include <boost/shared_array.hpp>
IMPRMF_BEGIN_NAMESPACE
namespace {
class RMFRestraint;
IMP_OBJECTS(RMFRestraint, RMFRestraints);
/** A dummy restraint object to represent restraints loaded from
    an RMF file.*/
class IMPRMFEXPORT RMFRestraint : public kernel::Restraint {
  kernel::ParticlesTemp ps_;

 public:
#ifndef IMP_DOXYGEN
  RMFRestraint(kernel::Model *m, std::string name);
  void set_particles(const kernel::ParticlesTemp &ps) { ps_ = ps; }
#endif
  double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) const;
  kernel::ModelObjectsTemp do_get_inputs() const;
  kernel::Restraints do_create_current_decomposition() const;
  IMP_OBJECT_METHODS(RMFRestraint);
};

double RMFRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  set_was_used(true);
  return get_last_score();
}

ModelObjectsTemp RMFRestraint::do_get_inputs() const { return ps_; }

Restraints RMFRestraint::do_create_current_decomposition() const {
  set_was_used(true);
  if (get_last_score() != 0) {
    const kernel::Restraint *rp = this;
    return kernel::Restraints(1, const_cast<kernel::Restraint *>(rp));
  } else {
    return kernel::Restraints();
  }
}

RMFRestraint::RMFRestraint(kernel::Model *m, std::string name)
    : kernel::Restraint(m, name) {}

class Subset : public base::ConstVector<base::WeakPointer<kernel::Particle>,
                                        kernel::Particle *> {
  typedef base::ConstVector<base::WeakPointer<kernel::Particle>,
                            kernel::Particle *> P;
  static kernel::ParticlesTemp get_sorted(kernel::ParticlesTemp ps) {
    std::sort(ps.begin(), ps.end());
    ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
    return ps;
  }

 public:
  /** Construct a subset from a non-empty list of particles.
   */
  explicit Subset(const kernel::ParticlesTemp &ps) : P(get_sorted(ps)) {}
  std::string get_name() const {
    std::ostringstream oss;
    for (unsigned int i = 0; i < size(); ++i) {
      if (i > 0) oss << ", ";
      oss << "\'" << operator[](i)->get_name() << "\'";
    }
    return oss.str();
  }
};

IMP_VALUES(Subset, Subsets);

template <class C>
RMF::NodeConstHandles get_node_ids(RMF::FileConstHandle fh, const C &ps) {
  RMF::NodeConstHandles ret;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    RMF::NodeConstHandle nh = get_node_from_association(fh, ps[i]);
    if (nh != RMF::NodeConstHandle()) {
      ret.push_back(nh);
    } else {
      IMP_WARN("Particle " << Showable(ps[i]) << " is not in the RMF."
                           << std::endl);
    }
  }
  return ret;
}

struct RestraintSaveData {
  // must not be a handle so as not to keep things alive
  base::map<Subset, RMF::NodeID> map_;
};

RMF::NodeHandle get_node(Subset s, RestraintSaveData &d, RMF::ScoreFactory sf,
                         RMF::NodeHandle parent) {
  if (d.map_.find(s) == d.map_.end()) {
    RMF::SetCurrentFrame scf(parent.get_file(), RMF::ALL_FRAMES);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (base::map<Subset, RMF::NodeID>::const_iterator it = d.map_.begin();
           it != d.map_.end(); ++it) {
        IMP_INTERNAL_CHECK(it->first != s, "Found!!!!");
      }
    }
    RMF::NodeHandle n = parent.add_child(s.get_name(), RMF::FEATURE);
    d.map_[s] = n.get_id();
    IMP_INTERNAL_CHECK(d.map_.find(s) != d.map_.end(), "Not found");
    RMF::Score csd = sf.get(n);
    RMF::NodeConstHandles nodes = get_node_ids(parent.get_file(), s);
    csd.set_representation(nodes);
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      RMF::NodeHandles reps = csd.get_representation();
      IMP_INTERNAL_CHECK(reps.size() == nodes.size(),
                         "Representation not set right");
    }
  }
  return parent.get_file().get_node(d.map_.find(s)->second);
}

// get_particles
//
class RestraintLoadLink : public SimpleLoadLink<kernel::Restraint> {
  typedef SimpleLoadLink<kernel::Restraint> P;
  RMF::ScoreConstFactory sf_;
  RMF::AliasConstFactory af_;
  RMF::Category imp_cat_;
  RMF::FloatKey weight_key_;

  void do_load_one(RMF::NodeConstHandle nh, kernel::Restraint *oi) {
    if (sf_.get_is(nh)) {
      RMF::ScoreConst d = sf_.get(nh);
      oi->set_last_score(d.get_score());
    } else {
      oi->set_last_score(0);
    }
  }
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type() == RMF::FEATURE;
  }
  kernel::Restraint *do_create(RMF::NodeConstHandle name, kernel::Model *m) {
    RMF::NodeConstHandles chs = name.get_children();
    kernel::Restraints childr;
    kernel::ParticlesTemp inputs;
    for (unsigned int i = 0; i < chs.size(); ++i) {
      if (chs[i].get_type() == RMF::FEATURE) {
        childr.push_back(do_create(chs[i], m));
        add_link(childr.back(), chs[i]);
      } else if (af_.get_is(chs[i])) {
        RMF::NodeConstHandle an = af_.get(chs[i]).get_aliased();
        IMP_LOG_TERSE("Found alias child to " << an.get_name() << " of type "
                                              << an.get_type() << std::endl);
        kernel::Particle *p = get_association<kernel::Particle>(an);
        if (p) {
          inputs.push_back(p);
        } else {
          kernel::Restraint *r = get_association<kernel::Restraint>(an);
          if (r) {
            childr.push_back(do_create(an, m));
            add_link(r, an);
          } else {
            IMP_WARN("No IMP particle or restraint for node " << an.get_name()
                                                              << std::endl);
          }
        }
      } else {
        IMP_WARN("Not sure what to do with unknown child " << chs[i].get_name()
                                                           << std::endl);
      }
    }
    base::Pointer<kernel::Restraint> ret;
    if (!childr.empty()) {
      ret = new kernel::RestraintSet(childr, 1.0, name.get_name());
    } else {
      IMP_NEW(RMFRestraint, r, (m, name.get_name()));
      ret = r;
      r->set_particles(inputs);
    }
    if (name.get_has_value(weight_key_)) {
      ret->set_weight(name.get_value(weight_key_));
    }
    return ret.release();
  }

 public:
  RestraintLoadLink(RMF::FileConstHandle fh)
      : P("RestraintLoadLink%1%"),
        sf_(fh),
        af_(fh),
        imp_cat_(fh.get_category("IMP")),
        weight_key_(fh.get_key<RMF::FloatTraits>(imp_cat_, "weight")) {}
  static const char *get_name() {return "restraint load";}

  IMP_OBJECT_METHODS(RestraintLoadLink);
};

class RestraintSaveLink : public SimpleSaveLink<kernel::Restraint> {
  typedef SimpleSaveLink<kernel::Restraint> P;
  RMF::ScoreFactory sf_;
  RMF::AliasFactory af_;
  RMF::Category imp_cat_;
  RMF::FloatKey weight_key_;
  base::map<kernel::Restraint *, RestraintSaveData> data_;
  kernel::Restraints all_;
  base::PointerMember<core::RestraintsScoringFunction> rsf_;
  unsigned int max_terms_;
  base::set<kernel::Restraint *> no_terms_;

  void do_add(kernel::Restraint *r, RMF::NodeHandle nh) {
    // handle restraints being in multiple sets
    all_.push_back(r);
    rsf_ = new core::RestraintsScoringFunction(all_);
    nh.set_value(weight_key_, r->get_weight());
    add_link(r, nh);
    kernel::RestraintSet *rs = dynamic_cast<kernel::RestraintSet *>(r);
    if (rs) {
      for (unsigned int i = 0; i < rs->get_number_of_restraints(); ++i) {
        kernel::Restraint *rc = rs->get_restraint(i);
        if (get_has_associated_node(nh.get_file(), rc)) {
          RMF::NodeHandle an = get_node_from_association(nh.get_file(), rc);
          RMF::NodeHandle c =
              nh.add_child(RMF::get_as_node_name(rc->get_name()), RMF::ALIAS);
          RMF::Alias a = af_.get(c);
          a.set_aliased(an);
        } else {
          RMF::NodeHandle c =
              nh.add_child(RMF::get_as_node_name(rc->get_name()), RMF::FEATURE);
          do_add(rc, c);
        }
      }
    }
  }
  void do_save_one(kernel::Restraint *o, RMF::NodeHandle nh) {
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE("Saving restraint info for " << o->get_name() << std::endl);
    RestraintSaveData &d = data_[o];
    {
      RMF::Score sdnf = sf_.get(nh);
      if (sdnf.get_representation().empty()) {
        RMF::SetCurrentFrame scf(nh.get_file(), RMF::ALL_FRAMES);
        // be lazy about it
        kernel::ParticlesTemp inputs = get_input_particles(o->get_inputs());
        std::sort(inputs.begin(), inputs.end());
        inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
        RMF::NodeConstHandles nhs = get_node_ids(nh.get_file(), inputs);
        sdnf.set_representation(nhs);
        IMP_INTERNAL_CHECK(sdnf.get_representation().size() == nhs.size(),
                           "Get and set values don't match: "
                               << nhs << " vs " << sdnf.get_representation());
      }
    }
    RMF::Score sd = sf_.get(nh);
    double score = o->get_last_score();
    // only set score if it is valid
    if (score < std::numeric_limits<double>::max()) {
      IMP_LOG_TERSE("Saving score" << std::endl);
      sd.set_score(score);
      if (no_terms_.find(o) != no_terms_.end()) {
        // too big, do nothing
      } else if (!dynamic_cast<kernel::RestraintSet *>(o)) {
        // required to set last score
        base::Pointer<kernel::Restraint> rd = o->create_current_decomposition();
        // set all child scores to 0 for this frame, we will over
        // right below
        RMF::NodeHandles chs = nh.get_children();
        for (unsigned int i = 0; i < chs.size(); ++i) {
          if (chs[i].get_type() == RMF::FEATURE) {
            RMF::Score s = sf_.get(chs[i]);
            s.set_score(0);
          }
        }
        if (rd && rd != o) {
          rd->set_was_used(true);
          kernel::RestraintsTemp rs =
              IMP::get_restraints(kernel::RestraintsTemp(1, rd));
          if (rs.size() > max_terms_) {
            no_terms_.insert(o);
            // delete old children
          } else {
            for (unsigned int i = 0; i < rs.size(); ++i) {
              Subset s(get_input_particles(rs[i]->get_inputs()));
              double score = rs[i]->get_last_score();
              rs[i]->set_was_used(true);
              if (score != 0) {
                RMF::NodeHandle nnh = get_node(s, d, sf_, nh);
                RMF::Score csd = sf_.get(nnh);
                csd.set_score(score);
                // csd.set_representation(get_node_ids(nh.get_file(), s));
              }
            }
          }
        }
      }
    }
  }

  void do_save(RMF::FileHandle fh) {
    rsf_->evaluate(false);
    P::do_save(fh);
  }
  RMF::NodeType get_type(kernel::Restraint *) const { return RMF::FEATURE; }

 public:
  RestraintSaveLink(RMF::FileHandle fh)
      : P("RestraintSaveLink%1%"),
        sf_(fh),
        af_(fh),
        imp_cat_(fh.get_category("IMP")),
        weight_key_(fh.get_key<RMF::FloatTraits>(imp_cat_, "weight")),
        max_terms_(100) {}
  void set_maximum_number_of_terms(unsigned int n) { max_terms_ = n; }
  static const char *get_name() {return "restraint save";}
  IMP_OBJECT_METHODS(RestraintSaveLink);
};
}

IMP_DEFINE_LINKERS(Restraint, restraint, restraints, kernel::Restraint *,
                   kernel::Restraints,
                   (RMF::FileConstHandle fh, kernel::Model *m), (fh, m));

void set_maximum_number_of_terms(RMF::FileHandle fh, unsigned int num) {
  RestraintSaveLink *hsl = internal::get_save_link<RestraintSaveLink>(fh);
  hsl->set_maximum_number_of_terms(num);
}

IMPRMF_END_NAMESPACE
