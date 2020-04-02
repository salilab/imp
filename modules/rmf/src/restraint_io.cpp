/**
 *  \file IMP/rmf/restraint_io.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/restraint_io.h>
#include <IMP/rmf/simple_links.h>
#include <IMP/rmf/link_macros.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <RMF/decorator/physics.h>
#include <RMF/decorator/uncertainty.h>
#include <RMF/decorator/feature.h>
#include <IMP/core/RestraintsScoringFunction.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/Gaussian.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/isd/Scale.h>
#include <IMP/atom/Mass.h>
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
RMF::Ints get_node_ids(RMF::FileConstHandle fh, const C &ps,
                       Restraint *o=nullptr) {
  RMF::Ints ret;
  for (unsigned int i = 0; i < ps.size(); ++i) {
    RMF::NodeConstHandle nh = get_node_from_association(fh, ps[i]);
    if (nh != RMF::NodeConstHandle()) {
      ret.push_back(nh.get_id().get_index());
    } else if (!core::RigidBody::get_is_setup(ps[i])) {
      /* Don't complain about rigid bodies referenced by restraints, as
         these don't have their own nodes in RMF */
      if (o) {
        IMP_WARN("Particle " << Showable(ps[i]) << " referenced by restraint "
               << Showable(o) << " is not in the RMF." << std::endl);
      } else {
        IMP_WARN("Particle " << Showable(ps[i]) << " is not in the RMF."
                             << std::endl);
       }
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
  typedef std::pair<std::string, ParticleIndexes> ParticleIndexesData;
  typedef SimpleLoadLink<Restraint> P;
  RMF::decorator::ScoreFactory sf_;
  RMF::decorator::RepresentationFactory rf_;
  RMF::decorator::GaussianParticleFactory gaussian_factory_;
  RMF::decorator::IntermediateParticleFactory ipf_;
  RMF::decorator::ScaleFactory scalef_;
  RMF::decorator::ReferenceFrameFactory refframef_;
  RMF::FloatKey radius_key_;
  RMF::FloatKey mass_key_;
  RMF::Category imp_cat_;
  RMF::Category imp_restraint_cat_;
  RMF::Category imp_restraint_fn_cat_;
  RMF::FloatKey weight_key_;
  RMF::IntKeys iks_;
  RMF::StringKeys sks_;
  RMF::FloatKeys fks_;
  RMF::FloatsKeys fsks_;
  RMF::IntsKeys isks_;
  RMF::StringsKeys ssks_;
  RMF::StringKeys filenameks_;
  RMF::StringsKeys filenamesks_;

  void do_load_one(RMF::NodeConstHandle nh, Restraint *oi) {
    RMF::NodeConstHandles chs = nh.get_children();
    if (sf_.get_is(nh)) {
      RMF::decorator::ScoreConst d = sf_.get(nh);
      IMP_LOG_TERSE("Loading score " << d.get_score() << " into restraint"
                                     << oi->get_name() << std::endl);
      oi->set_last_score(d.get_score());
    } else {
      oi->set_last_score(0);
    }
    IMP_FOREACH(RMF::NodeConstHandle ch, chs) {
      if (ch.get_type() == RMF::ORGANIZATIONAL) {
        load_restraint_particles(ch);
      }
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
    std::vector<ParticleIndexesData> static_pis, dynamic_pis;
    IMP_FOREACH(RMF::NodeConstHandle ch, chs) {
      if (ch.get_type() == RMF::FEATURE) {
        childr.push_back(do_create(ch, m));
        add_link(childr.back(), ch);
      } else if (ch.get_type() == RMF::ORGANIZATIONAL) {
        create_restraint_particles(ch, m, static_pis, dynamic_pis);
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
      load_restraint_info(r, name, static_pis, dynamic_pis);
    }
    if (name.get_has_value(weight_key_)) {
      ret->set_weight(name.get_value(weight_key_));
    }
    return ret.release();
  }

  void load_restraint_particles(RMF::NodeConstHandle parent) {
    IMP_FOREACH(RMF::NodeConstHandle ch, parent.get_children()) {
      Particle *p = get_association<Particle>(ch);
      if (p) {
        load_particle(ch, p->get_model(), p->get_index());
      } else {
        IMP_WARN("No IMP particle for node " << ch.get_name() << std::endl);
      }
    }
  }

  void create_restraint_particles(
      RMF::NodeConstHandle parent, Model *m,
      std::vector<ParticleIndexesData> &static_pis,
      std::vector<ParticleIndexesData> &dynamic_pis) {
    ParticleIndexes pis;
    IMP_FOREACH(RMF::NodeConstHandle ch, parent.get_children()) {
      Pointer<Particle> pi = new IMP::Particle(m, ch.get_name());
      set_association(ch, pi.get(), true);
      pis.push_back(pi->get_index());
      setup_particle(ch, m, pi->get_index());
    }
    // todo: distinguish static and dynamic pis
    static_pis.push_back(ParticleIndexesData(parent.get_name(), pis));
  }

  void load_particle(RMF::NodeConstHandle n, Model *m, ParticleIndex p) {
    if (scalef_.get_is(n)) {
      RMF::decorator::ScaleConst rscale = scalef_.get(n);
      isd::Scale scale(m, p);
      scale.set_scale(rscale.get_scale());
      scale.set_upper(rscale.get_upper());
      scale.set_lower(rscale.get_lower());
    }
  }

  void setup_particle(RMF::NodeConstHandle n, Model *m, ParticleIndex p) {
    if (n.get_has_value(radius_key_)) {
      double r = n.get_value(radius_key_);
      m->add_attribute(core::XYZR::get_radius_key(), p, r);
    }
    if (n.get_has_value(mass_key_)) {
      atom::Mass::setup_particle(m, p, n.get_value(mass_key_));
    }
    if (scalef_.get_is(n)) {
      if (!isd::Scale::get_is_setup(m, p)) {
        isd::Scale::setup_particle(m, p);
      }
      RMF::decorator::ScaleConst rscale = scalef_.get(n);
      isd::Scale scale(m, p);
      scale.set_scale(rscale.get_scale());
      scale.set_upper(rscale.get_upper());
      scale.set_lower(rscale.get_lower());
    }
    if (ipf_.get_is(n)) {
      if (core::XYZ::get_is_setup(m, p)) {
        core::XYZ(m, p).set_coordinates(internal::get_coordinates(n, ipf_));
      } else {
        core::XYZ::setup_particle(m, p, internal::get_coordinates(n, ipf_));
      }
    }
    if (gaussian_factory_.get_is(n)) {
      if (!core::Gaussian::get_is_setup(m, p)) {
        core::Gaussian::setup_particle(m, p);
      }
      RMF::Vector3 v = gaussian_factory_.get(n).get_variances();
      core::Gaussian(m, p).set_variances(algebra::Vector3D(v));
    }
    if (refframef_.get_is(n)) {
      if (!core::RigidBody::get_is_setup(m, p)) {
        core::RigidBody::setup_particle(m, p, algebra::ReferenceFrame3D());
      }
      algebra::ReferenceFrame3D rf(internal::get_transformation(n, refframef_));
      core::RigidBody(m, p).set_reference_frame_lazy(rf);
    }
  }

  void load_restraint_info(RMFRestraint *r, RMF::NodeConstHandle nh,
                           std::vector<ParticleIndexesData> &static_pis,
                           std::vector<ParticleIndexesData> &dynamic_pis) {
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
    RMF_FOREACH(RMF::StringKey k, filenameks_) {
      if (!nh.get_value(k).get_is_null()) {
        r->get_info()->add_filename(fh.get_name(k),
                                    RMF::internal::get_absolute_path(
                                            fh.get_path(), nh.get_value(k)));
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
    RMF_FOREACH(RMF::IntsKey k, isks_) {
      if (!nh.get_value(k).get_is_null()) {
        // No automatic conversion from RMF::Ints to IMP::Ints
        RMF::Ints rvalue = nh.get_value(k);
        Ints value(rvalue.begin(), rvalue.end());
        r->get_info()->add_ints(fh.get_name(k), value);
      }
    }
    RMF_FOREACH(RMF::StringsKey k, ssks_) {
      if (!nh.get_value(k).get_is_null()) {
        // No automatic conversion from RMF::Strings to IMP::Strings
        RMF::Strings rvalue = nh.get_value(k);
        Strings value(rvalue.begin(), rvalue.end());
        r->get_info()->add_strings(fh.get_name(k), value);
      }
    }
    RMF_FOREACH(RMF::StringsKey k, filenamesks_) {
      if (!nh.get_value(k).get_is_null()) {
        RMF::Strings rvalue = nh.get_value(k);
        // Convert RMF relative paths to absolute
        Strings value;
        for (RMF::Strings::const_iterator it = rvalue.begin();
             it != rvalue.end(); ++it) {
          value.push_back(RMF::internal::get_absolute_path(fh.get_path(), *it));
        }
        r->get_info()->add_filenames(fh.get_name(k), value);
      }
    }

    for (unsigned i = 0; i < static_pis.size(); ++i) {
      r->get_info()->add_particle_indexes(static_pis[i].first,
                                          static_pis[i].second);
    }
    for (unsigned i = 0; i < dynamic_pis.size(); ++i) {
      r->get_info()->add_particle_indexes(dynamic_pis[i].first,
                                          dynamic_pis[i].second);
    }
  }

 public:
  RestraintLoadLink(RMF::FileConstHandle fh)
      : P("RestraintLoadLink%1%"),
        sf_(fh),
        rf_(fh),
        gaussian_factory_(fh),
        ipf_(fh),
        scalef_(fh),
        refframef_(fh),
        imp_cat_(fh.get_category("IMP")),
        imp_restraint_cat_(fh.get_category("IMP restraint")),
        imp_restraint_fn_cat_(fh.get_category("IMP restraint files")),
        weight_key_(fh.get_key<RMF::FloatTraits>(imp_cat_, "weight")) {
    iks_ = fh.get_keys<RMF::IntTraits>(imp_restraint_cat_);
    fks_ = fh.get_keys<RMF::FloatTraits>(imp_restraint_cat_);
    sks_ = fh.get_keys<RMF::StringTraits>(imp_restraint_cat_);
    fsks_ = fh.get_keys<RMF::FloatsTraits>(imp_restraint_cat_);
    isks_ = fh.get_keys<RMF::IntsTraits>(imp_restraint_cat_);
    ssks_ = fh.get_keys<RMF::StringsTraits>(imp_restraint_cat_);
    filenameks_ = fh.get_keys<RMF::StringTraits>(imp_restraint_fn_cat_);
    filenamesks_ = fh.get_keys<RMF::StringsTraits>(imp_restraint_fn_cat_);
    RMF::Category phy = fh.get_category("physics");
    radius_key_ = fh.get_key(phy, "radius", RMF::FloatTraits());
    mass_key_ = fh.get_key(phy, "mass", RMF::FloatTraits());
  }
  static const char *get_name() { return "restraint load"; }

  IMP_OBJECT_METHODS(RestraintLoadLink);
};

class RestraintSaveLink : public SimpleSaveLink<Restraint> {
  typedef SimpleSaveLink<Restraint> P;
  RMF::decorator::ScoreFactory sf_;
  RMF::decorator::RepresentationFactory rf_;
  RMF::decorator::ParticleFactory particle_factory_;
  RMF::decorator::GaussianParticleFactory gaussian_factory_;
  RMF::decorator::IntermediateParticleFactory ipf_;
  RMF::decorator::ScaleFactory scalef_;
  RMF::decorator::ReferenceFrameFactory refframef_;
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
      RMF::Ints nhs = get_node_ids(nh.get_file(), inputs, o);
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
    for (i = 0; i < ri->get_number_of_filename(); ++i) {
      RMF::StringKey key = fh.get_key<RMF::StringTraits>(
                             imp_restraint_fn_cat_, ri->get_filename_key(i));
      nh.set_frame_value(key, RMF::internal::get_relative_path(
                                 fh.get_path(), ri->get_filename_value(i)));
    }
    for (i = 0; i < ri->get_number_of_floats(); ++i) {
      RMF::FloatsKey key = fh.get_key<RMF::FloatsTraits>(
                             imp_restraint_cat_, ri->get_floats_key(i));
      // No automatic conversion from IMP::Floats to RMF::Floats
      Floats value = ri->get_floats_value(i);
      RMF::Floats rvalue(value.begin(), value.end());
      nh.set_frame_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_ints(); ++i) {
      RMF::IntsKey key = fh.get_key<RMF::IntsTraits>(
                             imp_restraint_cat_, ri->get_ints_key(i));
      // No automatic conversion from IMP::Ints to RMF::Ints
      Ints value = ri->get_ints_value(i);
      RMF::Ints rvalue(value.begin(), value.end());
      nh.set_frame_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_strings(); ++i) {
      RMF::StringsKey key = fh.get_key<RMF::StringsTraits>(
                             imp_restraint_cat_, ri->get_strings_key(i));
      // No automatic conversion from IMP::Strings to RMF::Strings
      Strings value = ri->get_strings_value(i);
      RMF::Strings rvalue(value.begin(), value.end());
      nh.set_frame_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_filenames(); ++i) {
      RMF::StringsKey key = fh.get_key<RMF::StringsTraits>(
                             imp_restraint_fn_cat_, ri->get_filenames_key(i));
      Strings value = ri->get_filenames_value(i);
      RMF::Strings rvalue;
      for (Strings::const_iterator it = value.begin();
           it != value.end(); ++it) {
        rvalue.push_back(RMF::internal::get_relative_path(fh.get_path(), *it));
      }
      nh.set_frame_value(key, rvalue);
    }

    save_restraint_child_particles(o, nh, ri);
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
    for (i = 0; i < ri->get_number_of_filename(); ++i) {
      RMF::StringKey key = fh.get_key<RMF::StringTraits>(
                             imp_restraint_fn_cat_, ri->get_filename_key(i));
      nh.set_static_value(key, RMF::internal::get_relative_path(
                                  fh.get_path(), ri->get_filename_value(i)));
    }
    for (i = 0; i < ri->get_number_of_floats(); ++i) {
      RMF::FloatsKey key = fh.get_key<RMF::FloatsTraits>(
                             imp_restraint_cat_, ri->get_floats_key(i));
      // No automatic conversion from IMP::Floats to RMF::Floats
      Floats value = ri->get_floats_value(i);
      RMF::Floats rvalue(value.begin(), value.end());
      nh.set_static_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_ints(); ++i) {
      RMF::IntsKey key = fh.get_key<RMF::IntsTraits>(
                             imp_restraint_cat_, ri->get_ints_key(i));
      // No automatic conversion from IMP::Ints to RMF::Ints
      Ints value = ri->get_ints_value(i);
      RMF::Ints rvalue(value.begin(), value.end());
      nh.set_static_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_strings(); ++i) {
      RMF::StringsKey key = fh.get_key<RMF::StringsTraits>(
                             imp_restraint_cat_, ri->get_strings_key(i));
      // No automatic conversion from IMP::Strings to RMF::Strings
      Strings value = ri->get_strings_value(i);
      RMF::Strings rvalue(value.begin(), value.end());
      nh.set_static_value(key, rvalue);
    }
    for (i = 0; i < ri->get_number_of_filenames(); ++i) {
      RMF::StringsKey key = fh.get_key<RMF::StringsTraits>(
                             imp_restraint_fn_cat_, ri->get_filenames_key(i));
      Strings value = ri->get_filenames_value(i);
      RMF::Strings rvalue;
      for (Strings::const_iterator it = value.begin();
           it != value.end(); ++it) {
        rvalue.push_back(RMF::internal::get_relative_path(fh.get_path(), *it));
      }
      nh.set_static_value(key, rvalue);
    }

    add_restraint_child_particles(o, nh, ri);

    // Need dynamic info too to get the set of particles
    Pointer<RestraintInfo> dri = o->get_dynamic_info();
    if (dri) {
      dri->set_was_used(true);
      add_restraint_child_particles(o, nh, dri);
    }
  }

  void add_restraint_child_particles(Restraint *o, RMF::NodeHandle nh,
                                     RestraintInfo *ri) {
    unsigned i;
    for (i = 0; i < ri->get_number_of_particle_indexes(); ++i) {
      RMF::NodeHandle n = nh.add_child(ri->get_particle_indexes_key(i),
                                       RMF::ORGANIZATIONAL);
      add_static_particles(n, IMP::get_particles(o->get_model(),
                                       ri->get_particle_indexes_value(i)));
    }
  }

  void save_restraint_child_particles(Restraint *o, RMF::NodeHandle nh,
                                      RestraintInfo *ri) {
    unsigned i;
    RMF::FileHandle file = nh.get_file();
    for (i = 0; i < ri->get_number_of_particle_indexes(); ++i) {
      save_dynamic_particles(file, IMP::get_particles(o->get_model(),
                                       ri->get_particle_indexes_value(i)));
    }
  }

  void save_dynamic_particles(RMF::FileHandle file, ParticlesTemp ps) {
    for (ParticlesTemp::iterator pit = ps.begin(); pit != ps.end(); ++pit) {
      Particle *p = *pit;
      RMF::NodeHandle nh = get_node_from_association(file, p);
      save_node(p->get_model(), p->get_index(), nh);
    }
  }

  void add_static_particles(RMF::NodeHandle parent, ParticlesTemp ps) {
    RMF::FileHandle file = parent.get_file();
    RMF::decorator::AliasFactory af(file);
    for (ParticlesTemp::iterator pit = ps.begin(); pit != ps.end(); ++pit) {
      Particle *p = *pit;
      std::string nicename = RMF::get_as_node_name(p->get_name());
      if (get_has_associated_node(file, p)) {
        RMF::NodeHandle c = parent.add_child(nicename, RMF::ALIAS);
        af.get(c).set_aliased(get_node_from_association(file, p));
      } else {
        RMF::NodeHandle c = parent.add_child(nicename, RMF::REPRESENTATION);
        setup_node(p->get_model(), p->get_index(), c);
        // add static attributes
        set_association(c, p, true);
      }
    }
  }

  void save_node(Model *m, ParticleIndex p, RMF::NodeHandle n) {
    if (isd::Scale::get_is_setup(m, p)) {
      isd::Scale scale(m, p);
      RMF::decorator::Scale rscale = scalef_.get(n);
      rscale.set_scale(scale.get_scale());
      rscale.set_upper(scale.get_upper());
      rscale.set_lower(scale.get_lower());
    }
  }

  void setup_node(Model *m, ParticleIndex p, RMF::NodeHandle n) {
    if (core::RigidBody::get_is_setup(m, p)) {
      internal::copy_to_static_reference_frame(
         core::RigidBody(m, p).get_reference_frame().get_transformation_to(),
         n, refframef_);
    }
    if (isd::Scale::get_is_setup(m, p)) {
      isd::Scale scale(m, p);
      RMF::decorator::Scale rscale = scalef_.get(n);
      rscale.set_scale(scale.get_scale());
      rscale.set_upper(scale.get_upper());
      rscale.set_lower(scale.get_lower());
    }
    if (core::XYZR::get_is_setup(m, p)) {
      core::XYZR d(m, p);
      internal::copy_to_static_particle(d.get_coordinates(), n, ipf_);
      ipf_.get(n).set_radius(d.get_radius());
    }
    if (atom::Mass::get_is_setup(m, p)) {
      atom::Mass d(m, p);
      particle_factory_.get(n).set_mass(d.get_mass());
    }
    if (core::Gaussian::get_is_setup(m, p)) {
      algebra::Vector3D var = core::Gaussian(m, p).get_variances();
      gaussian_factory_.get(n).set_variances(RMF::Vector3(var));
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
        particle_factory_(fh),
        gaussian_factory_(fh),
        ipf_(fh),
        scalef_(fh),
        refframef_(fh),
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
