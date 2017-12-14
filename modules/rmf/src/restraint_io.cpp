/**
 *  \file IMP/rmf/restraint_io.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>
#include <RMF/decorator/physics.h>
#include <RMF/decorator/feature.h>
#include <IMP/core/RestraintsScoringFunction.h>
#include <IMP/input_output.h>
#include <IMP/ConstVector.h>
#include <IMP/WeakPointer.h>
#include <boost/shared_array.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

IMPRMF_BEGIN_NAMESPACE
namespace {
class RMFRestraint;
IMP_OBJECTS(RMFRestraint, RMFRestraints);
/** A dummy restraint object to represent restraints loaded from
    an RMF file.*/
class IMPRMFEXPORT RMFRestraint : public Restraint {
  ParticlesTemp ps_;
  PointerMember<RestraintInfo> info_;

 public:
#ifndef IMP_DOXYGEN
  RMFRestraint(Model *m, std::string name);
  void set_particles(const ParticlesTemp &ps) { ps_ = ps; }

  // Note that we don't make a distinction here between dynamic and static info
  RestraintInfo *get_info() {
    if (!info_) {
      info_ = new RestraintInfo();
    }
    return info_;
  }
  RestraintInfo *get_dynamic_info() const IMP_OVERRIDE { return info_; }
  RestraintInfo *get_static_info() const IMP_OVERRIDE { return info_; }

#endif
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;
  Restraints do_create_current_decomposition() const;
  IMP_OBJECT_METHODS(RMFRestraint);
};

double RMFRestraint::unprotected_evaluate(DerivativeAccumulator *)
    const {
  set_was_used(true);
  return get_last_score();
}

ModelObjectsTemp RMFRestraint::do_get_inputs() const { return ps_; }

Restraints RMFRestraint::do_create_current_decomposition() const {
  set_was_used(true);
  if (get_last_score() != 0) {
    const Restraint *rp = this;
    return Restraints(1, const_cast<Restraint *>(rp));
  } else {
    return Restraints();
  }
}

RMFRestraint::RMFRestraint(Model *m, std::string name)
    : Restraint(m, name) {}

class Subset : public ConstVector<WeakPointer<Particle>, Particle *> {
  typedef ConstVector<WeakPointer<Particle>, Particle *> P;
  static ParticlesTemp get_sorted(ParticlesTemp ps) {
    std::sort(ps.begin(), ps.end());
    ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
    return ps;
  }

 public:
  /** Construct a subset from a non-empty list of particles.
   */
  explicit Subset(const ParticlesTemp &ps) : P(get_sorted(ps)) {}
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
RMF::Ints get_node_ids(RMF::FileConstHandle fh, const C &ps) {
  RMF::Ints ret;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    RMF::NodeConstHandle nh = get_node_from_association(fh, ps[i]);
    if (nh != RMF::NodeConstHandle()) {
      ret.push_back(nh.get_id().get_index());
    } else {
      IMP_WARN("Particle " << Showable(ps[i]) << " is not in the RMF."
                           << std::endl);
    }
  }
  return ret;
}

struct RestraintSaveData {
  // must not be a handle so as not to keep things alive
  boost::unordered_map<Subset, RMF::NodeID> map_;
};

RMF::NodeHandle get_node(Subset s, RestraintSaveData &d,
                         RMF::decorator::RepresentationFactory sf,
                         RMF::NodeHandle parent) {
  if (d.map_.find(s) == d.map_.end()) {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (boost::unordered_map<Subset, RMF::NodeID>::const_iterator it =
               d.map_.begin();
           it != d.map_.end(); ++it) {
        IMP_INTERNAL_CHECK(it->first != s, "Found!!!!");
      }
    }
    RMF::NodeHandle n = parent.add_child(s.get_name(), RMF::FEATURE);
    d.map_[s] = n.get_id();
    IMP_INTERNAL_CHECK(d.map_.find(s) != d.map_.end(), "Not found");
    RMF::decorator::Representation csd = sf.get(n);
    RMF::Ints nodes = get_node_ids(parent.get_file(), s);
    csd.set_static_representation(nodes);
  }
  return parent.get_file().get_node(d.map_.find(s)->second);
}

// get_particles
//
class RestraintLoadLink : public SimpleLoadLink<Restraint> {
  typedef SimpleLoadLink<Restraint> P;
  RMF::decorator::ScoreFactory sf_;
  RMF::decorator::RepresentationFactory rf_;
  RMF::Category imp_cat_;
  RMF::Category imp_restraint_cat_;
  RMF::Category imp_restraint_fn_cat_;
  RMF::FloatKey weight_key_;
  RMF::IntKeys iks_;
  RMF::StringKeys sks_;
  RMF::FloatKeys fks_;
  RMF::FloatsKeys fsks_;
  RMF::StringsKeys filenamesks_;

  void do_load_one(RMF::NodeConstHandle nh, Restraint *oi) {
    if (sf_.get_is(nh)) {
      RMF::decorator::ScoreConst d = sf_.get(nh);
      IMP_LOG_TERSE("Loading score " << d.get_score() << " into restraint"
                                     << oi->get_name() << std::endl);
      oi->set_last_score(d.get_score());
    } else {
      oi->set_last_score(0);
    }
  }
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type() == RMF::FEATURE;
  }
  using P::do_create;
  Restraint *do_create(RMF::NodeConstHandle name, Model *m) {
    RMF::NodeConstHandles chs = name.get_children();
    Restraints childr;
    ParticlesTemp inputs;
    IMP_FOREACH(RMF::NodeConstHandle ch, chs) {
      if (ch.get_type() == RMF::FEATURE) {
        childr.push_back(do_create(ch, m));
        add_link(childr.back(), ch);
      }
    }
    if (rf_.get_is(name)) {
      IMP_FOREACH(RMF::NodeConstHandle an, rf_.get(name).get_representation()) {
        IMP_LOG_TERSE("Found alias child to " << an.get_name() << " of type "
                                              << an.get_type() << std::endl);
        Particle *p = get_association<Particle>(an);
        if (p) {
          inputs.push_back(p);
        } else {
          Restraint *r = get_association<Restraint>(an);
          if (r) {
            childr.push_back(do_create(an, m));
            add_link(r, an);
          } else {
            IMP_WARN("No IMP particle or restraint for node " << an.get_name()
                                                              << std::endl);
          }
        }
      }
    }
    Pointer<Restraint> ret;
    if (!childr.empty()) {
      ret = new RestraintSet(childr, 1.0, name.get_name());
    } else {
      IMP_NEW(RMFRestraint, r, (m, name.get_name()));
      ret = r;
      r->set_particles(inputs);
      load_restraint_info(r, name);
    }
    if (name.get_has_value(weight_key_)) {
      ret->set_weight(name.get_value(weight_key_));
    }
    return ret.release();
  }

  void load_restraint_info(RMFRestraint *r, RMF::NodeConstHandle nh) {
    RMF::FileConstHandle fh = nh.get_file();
    RMF_FOREACH(RMF::IntKey k, iks_) {
      if (!nh.get_value(k).get_is_null()) {
        r->get_info()->add_int(fh.get_name(k), nh.get_value(k));
      }
    }
    RMF_FOREACH(RMF::FloatKey k, fks_) {
      if (!nh.get_value(k).get_is_null()) {
        r->get_info()->add_float(fh.get_name(k), nh.get_value(k));
      }
    }
    RMF_FOREACH(RMF::StringKey k, sks_) {
      if (!nh.get_value(k).get_is_null()) {
        r->get_info()->add_string(fh.get_name(k), nh.get_value(k));
      }
    }
    RMF_FOREACH(RMF::FloatsKey k, fsks_) {
      if (!nh.get_value(k).get_is_null()) {
        // No automatic conversion from RMF::Floats to IMP::Floats
        RMF::Floats rvalue = nh.get_value(k);
        Floats value(rvalue.begin(), rvalue.end());
        r->get_info()->add_floats(fh.get_name(k), value);
      }
    }
    RMF_FOREACH(RMF::StringsKey k, filenamesks_) {
      if (!nh.get_value(k).get_is_null()) {
        RMF::Strings rvalue = nh.get_value(k);
        // Convert RMF relative paths to absolute
        Strings value;
        for (RMF::Strings::const_iterator it = rvalue.begin();
             it != rvalue.end(); ++it) {
          value.push_back(RMF::internal::get_absolute_path(*it, fh.get_path()));
        }
        r->get_info()->add_filenames(fh.get_name(k), value);
      }
    }
  }

 public:
  RestraintLoadLink(RMF::FileConstHandle fh)
      : P("RestraintLoadLink%1%"),
        sf_(fh),
        rf_(fh),
        imp_cat_(fh.get_category("IMP")),
        imp_restraint_cat_(fh.get_category("IMP restraint")),
        imp_restraint_fn_cat_(fh.get_category("IMP restraint files")),
        weight_key_(fh.get_key<RMF::FloatTraits>(imp_cat_, "weight")) {
    iks_ = fh.get_keys<RMF::IntTraits>(imp_restraint_cat_);
    fks_ = fh.get_keys<RMF::FloatTraits>(imp_restraint_cat_);
    sks_ = fh.get_keys<RMF::StringTraits>(imp_restraint_cat_);
    fsks_ = fh.get_keys<RMF::FloatsTraits>(imp_restraint_cat_);
    filenamesks_ = fh.get_keys<RMF::StringsTraits>(imp_restraint_fn_cat_);
  }
  static const char *get_name() { return "restraint load"; }

  IMP_OBJECT_METHODS(RestraintLoadLink);
};

class RestraintSaveLink : public SimpleSaveLink<Restraint> {
  typedef SimpleSaveLink<Restraint> P;
  RMF::decorator::ScoreFactory sf_;
  RMF::decorator::RepresentationFactory rf_;
  RMF::Category imp_cat_;
  RMF::Category imp_restraint_cat_;
  RMF::Category imp_restraint_fn_cat_;
  RMF::FloatKey weight_key_;
  boost::unordered_map<Restraint *, RestraintSaveData> data_;
  Restraints all_;
  PointerMember<core::RestraintsScoringFunction> rsf_;
  unsigned int max_terms_;
  boost::unordered_set<Restraint *> no_terms_;

  void do_add(Restraint *r, RMF::NodeHandle nh) {
    // handle restraints being in multiple sets
    all_.push_back(r);
    rsf_ = new core::RestraintsScoringFunction(all_);
    nh.set_static_value(weight_key_, r->get_weight());
    save_static_info(r, nh);
    // sf_.get(nh).set_static_score(0.0);
    add_link(r, nh);
    RestraintSet *rs = dynamic_cast<RestraintSet *>(r);
    if (rs) {
      for (unsigned int i = 0; i < rs->get_number_of_restraints(); ++i) {
        Restraint *rc = rs->get_restraint(i);
        if (get_has_associated_node(nh.get_file(), rc)) {
          RMF::NodeHandle an = get_node_from_association(nh.get_file(), rc);
          nh.add_child(an);
        } else {
          RMF::NodeHandle c =
              nh.add_child(RMF::get_as_node_name(rc->get_name()), RMF::FEATURE);
          do_add(rc, c);
        }
      }
    }
  }
  void do_save_one(Restraint *o, RMF::NodeHandle nh) {
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE("Saving restraint info for " << o->get_name() << std::endl);
    RestraintSaveData &d = data_[o];
    if (!rf_.get_is(nh)) {
      RMF::decorator::Representation sdnf = rf_.get(nh);
      // be lazy about it
      ParticlesTemp inputs =
          get_input_particles(o->get_inputs());
      std::sort(inputs.begin(), inputs.end());
      inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
      RMF::Ints nhs = get_node_ids(nh.get_file(), inputs);
      sdnf.set_static_representation(nhs);
    }
    save_dynamic_info(o, nh);

    RMF::decorator::Score sd = sf_.get(nh);
    double score = o->get_last_score();
    // only set score if it is valid
    if (score != 0) {
      IMP_LOG_TERSE("Saving score" << std::endl);
      sd.set_frame_score(score);
      Subset os(get_input_particles(o->get_inputs()));
      if (no_terms_.find(o) != no_terms_.end()) {
        // too big, do nothing
      } else if (!dynamic_cast<RestraintSet *>(o)) {
        // required to set last score
        Pointer<Restraint> rd = o->create_current_decomposition();
        // set all child scores to 0 for this frame, we will over
        // right below
        /*RMF::NodeHandles chs = nh.get_children();
        for (unsigned int i = 0; i < chs.size(); ++i) {
          if (chs[i].get_type() == RMF::FEATURE) {
            RMF::Score s = sf_.get(chs[i]);
            s.set_frame_score(0);
          }
          }*/
        if (rd && rd != o) {
          rd->set_was_used(true);
          if (Subset(get_input_particles(rd->get_inputs())) != os) {
            RestraintsTemp rs =
                get_restraints(RestraintsTemp(1, rd));
            if (rs.size() > max_terms_) {
              no_terms_.insert(o);
              // delete old children
            } else {
              IMP_FOREACH(Restraint * r, rs) {
                Subset s(get_input_particles(r->get_inputs()));
                double score = r->get_last_score();
                r->set_was_used(true);
                if (score != 0) {
                  IMP_LOG_VERBOSE("Saving subscore for " << r->get_name()
                                                         << " of " << score
                                                         << std::endl);
                  RMF::NodeHandle nnh = get_node(s, d, rf_, nh);
                  RMF::decorator::Score csd = sf_.get(nnh);
                  csd.set_frame_score(score);
                  // csd.set_representation(get_node_ids(nh.get_file(), s));
                }
              }
            }
          }
        }
      }
    }
  }

  // Save any info from Restraint::get_dynamic_info()
  void save_dynamic_info(Restraint *o, RMF::NodeHandle nh) {
    Pointer<RestraintInfo> ri = o->get_dynamic_info();
    if (!ri) return;

    ri->set_was_used(true);
    unsigned i;
    RMF::FileHandle fh = nh.get_file();
    for (i = 0; i < ri->get_number_of_int(); ++i) {
      RMF::IntKey key = fh.get_key<RMF::IntTraits>(
                             imp_restraint_cat_, ri->get_int_key(i));
      nh.set_frame_value(key, ri->get_int_value(i));
    }
    for (i = 0; i < ri->get_number_of_float(); ++i) {
      RMF::FloatKey key = fh.get_key<RMF::FloatTraits>(
                             imp_restraint_cat_, ri->get_float_key(i));
      nh.set_frame_value(key, ri->get_float_value(i));
    }
    for (i = 0; i < ri->get_number_of_string(); ++i) {
      RMF::StringKey key = fh.get_key<RMF::StringTraits>(
                             imp_restraint_cat_, ri->get_string_key(i));
      nh.set_frame_value(key, ri->get_string_value(i));
    }
    for (i = 0; i < ri->get_number_of_floats(); ++i) {
      RMF::FloatsKey key = fh.get_key<RMF::FloatsTraits>(
                             imp_restraint_cat_, ri->get_floats_key(i));
      // No automatic conversion from IMP::Floats to RMF::Floats
      Floats value = ri->get_floats_value(i);
      RMF::Floats rvalue(value.begin(), value.end());
      nh.set_frame_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_filenames(); ++i) {
      RMF::StringsKey key = fh.get_key<RMF::StringsTraits>(
                             imp_restraint_fn_cat_, ri->get_filenames_key(i));
      Strings value = ri->get_filenames_value(i);
      RMF::Strings rvalue;
      for (Strings::const_iterator it = value.begin();
           it != value.end(); ++it) {
        rvalue.push_back(RMF::internal::get_relative_path(*it, fh.get_path()));
      }
      nh.set_frame_value(key, rvalue);
    }
  }

  // Save any info from Restraint::get_static_info()
  void save_static_info(Restraint *o, RMF::NodeHandle nh) {
    Pointer<RestraintInfo> ri = o->get_static_info();
    if (!ri) return;

    ri->set_was_used(true);
    unsigned i;
    RMF::FileHandle fh = nh.get_file();
    for (i = 0; i < ri->get_number_of_int(); ++i) {
      RMF::IntKey key = fh.get_key<RMF::IntTraits>(
                             imp_restraint_cat_, ri->get_int_key(i));
      nh.set_static_value(key, ri->get_int_value(i));
    }
    for (i = 0; i < ri->get_number_of_float(); ++i) {
      RMF::FloatKey key = fh.get_key<RMF::FloatTraits>(
                             imp_restraint_cat_, ri->get_float_key(i));
      nh.set_static_value(key, ri->get_float_value(i));
    }
    for (i = 0; i < ri->get_number_of_string(); ++i) {
      RMF::StringKey key = fh.get_key<RMF::StringTraits>(
                             imp_restraint_cat_, ri->get_string_key(i));
      nh.set_static_value(key, ri->get_string_value(i));
    }
    for (i = 0; i < ri->get_number_of_floats(); ++i) {
      RMF::FloatsKey key = fh.get_key<RMF::FloatsTraits>(
                             imp_restraint_cat_, ri->get_floats_key(i));
      // No automatic conversion from IMP::Floats to RMF::Floats
      Floats value = ri->get_floats_value(i);
      RMF::Floats rvalue(value.begin(), value.end());
      nh.set_static_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_filenames(); ++i) {
      RMF::StringsKey key = fh.get_key<RMF::StringsTraits>(
                             imp_restraint_fn_cat_, ri->get_filenames_key(i));
      Strings value = ri->get_filenames_value(i);
      RMF::Strings rvalue;
      for (Strings::const_iterator it = value.begin();
           it != value.end(); ++it) {
        rvalue.push_back(RMF::internal::get_relative_path(*it, fh.get_path()));
      }
      nh.set_static_value(key, rvalue);
    }
  }

  void do_save(RMF::FileHandle fh) {
    rsf_->evaluate(false);
    P::do_save(fh);
  }
  RMF::NodeType get_type(Restraint *) const { return RMF::FEATURE; }

 public:
  RestraintSaveLink(RMF::FileHandle fh)
      : P("RestraintSaveLink%1%"),
        sf_(fh),
        rf_(fh),
        imp_cat_(fh.get_category("IMP")),
        imp_restraint_cat_(fh.get_category("IMP restraint")),
        imp_restraint_fn_cat_(fh.get_category("IMP restraint files")),
        weight_key_(fh.get_key<RMF::FloatTraits>(imp_cat_, "weight")),
        max_terms_(100) {}
  void set_maximum_number_of_terms(unsigned int n) { max_terms_ = n; }
  static const char *get_name() { return "restraint save"; }
  IMP_OBJECT_METHODS(RestraintSaveLink);
};
}

IMP_DEFINE_LINKERS(Restraint, restraint, restraints, Restraint *,
                   Restraints,
                   (RMF::FileConstHandle fh, Model *m), (fh, m));

void set_maximum_number_of_terms(RMF::FileHandle fh, unsigned int num) {
  RestraintSaveLink *hsl = internal::get_save_link<RestraintSaveLink>(fh);
  hsl->set_maximum_number_of_terms(num);
}

void add_restraints_as_bonds(RMF::FileHandle fh, const Restraints &rs) {
  RMF::decorator::BondFactory bf(fh);
  Restraints decomp;

  IMP_FOREACH(Restraint * r, rs) {
    Pointer<Restraint> rd = r->create_decomposition();
    if (rd == r) {
      decomp.push_back(rd);
    } else {
      rd->set_was_used(true);
      decomp += get_restraints(RestraintsTemp(1, rd));
    }
  }
  RMF::NodeHandle bdr =
      fh.get_root_node().add_child("restraint bonds", RMF::ORGANIZATIONAL);
  IMP_FOREACH(Restraint * bd, decomp) {
    Subset s(get_input_particles(bd->get_inputs()));
    bd->set_was_used(bd);
    RMF::NodeHandles inputs;
    IMP_FOREACH(Particle * cur,
                get_input_particles(bd->get_inputs())) {
      RMF::NodeHandle n = get_node_from_association(fh, cur);
      if (n != RMF::NodeHandle()) {
        inputs.push_back(n);
      }
    }
    IMP_USAGE_CHECK(inputs.size() == 2,
                    "Decomposed restraint does not have two inputs in file: "
                        << bd->get_name() << " has " << inputs.size()
                        << std::endl);
    RMF::NodeHandle nh = bdr.add_child(bd->get_name(), RMF::BOND);
    bf.get(nh).set_bonded_0(inputs[0].get_id().get_index());
    bf.get(nh).set_bonded_1(inputs[1].get_id().get_index());
  }
}

IMPRMF_END_NAMESPACE
