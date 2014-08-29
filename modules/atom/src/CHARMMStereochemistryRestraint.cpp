/**
 * \file CHARMMStereochemistryRestraint.cpp
 * \brief Class to maintain CHARMM stereochemistry.
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/CHARMMStereochemistryRestraint.h>
#include <IMP/core/Harmonic.h>
#include <set>
IMPATOM_BEGIN_NAMESPACE


void CHARMMStereochemistryRestraint::init(Hierarchy h, CHARMMTopology *topology){
  bonds_ = topology->add_bonds(h);
  angles_ = topology->get_parameters()->create_angles(bonds_);
  dihedrals_ = topology->get_parameters()->create_dihedrals(bonds_);
  impropers_ = topology->add_impropers(h);

  bond_score_ = new BondSingletonScore(new core::Harmonic(0., 1.));
  angle_score_ = new AngleSingletonScore(new core::Harmonic(0., 1.));
  dihedral_score_ = new DihedralSingletonScore();
  improper_score_ = new ImproperSingletonScore(new core::Harmonic(0., 1.));

  full_bonds_ = bonds_;
  full_angles_ = angles_;
  full_dihedrals_ = dihedrals_;
  full_impropers_ = impropers_;
}


CHARMMStereochemistryRestraint::CHARMMStereochemistryRestraint(
    Hierarchy h, CHARMMTopology *topology)
    : kernel::Restraint(h->get_model(), "CHARMMStereochemistryRestraint%1%") {
  init(h,topology);
}

CHARMMStereochemistryRestraint::CHARMMStereochemistryRestraint(
    Hierarchy h, CHARMMTopology *topology, ParticlesTemp limit_to_these_particles)
  : kernel::Restraint(h->get_model(),
                      "CHARMMStereochemistryRestraint%1%") {
    init(h,topology);

  kernel::Particles tbonds, tangles, tdihedrals, timpropers;
  std::set<kernel::ParticleIndex> pset;
  for (size_t np=0;np<limit_to_these_particles.size();np++)
    pset.insert(limit_to_these_particles[np]->get_index());
  for (kernel::Particles::const_iterator tb = bonds_.begin(); tb != bonds_.end();
       ++tb) {
    Bond b(*tb);
    if (pset.count(b.get_bonded(0).get_particle_index()) &&
        pset.count(b.get_bonded(1).get_particle_index()))
      tbonds.push_back(*tb);
  }

  for (kernel::Particles::const_iterator ta = angles_.begin();
       ta != angles_.end(); ++ta) {
    Angle a(*ta);
    if (pset.count(a.get_particle(0)->get_index()) &&
        pset.count(a.get_particle(1)->get_index()) &&
        pset.count(a.get_particle(2)->get_index()))
      tangles.push_back(*ta);
  }

  for (kernel::Particles::const_iterator td = dihedrals_.begin();
       td != dihedrals_.end(); ++td) {
    Dihedral d(*td);
    if (pset.count(d.get_particle(0)->get_index()) &&
        pset.count(d.get_particle(1)->get_index()) &&
        pset.count(d.get_particle(2)->get_index()) &&
        pset.count(d.get_particle(3)->get_index()))
      tdihedrals.push_back(*td);
  }

  for (kernel::Particles::const_iterator ti = impropers_.begin();
       ti != impropers_.end(); ++ti) {
    Dihedral i(*ti);
    if (pset.count(i.get_particle(0)->get_index()) &&
        pset.count(i.get_particle(1)->get_index()) &&
        pset.count(i.get_particle(2)->get_index()) &&
        pset.count(i.get_particle(3)->get_index()))
      timpropers.push_back(*ti);
  }
  bonds_ = tbonds;
  angles_ = tangles;
  dihedrals_ = tdihedrals;
  impropers_ = timpropers;
}

double CHARMMStereochemistryRestraint::unprotected_evaluate(
    DerivativeAccumulator *accum) const {
  double score = 0.;

  for (kernel::Particles::const_iterator b = bonds_.begin(); b != bonds_.end();
       ++b) {
    score += bond_score_->evaluate(*b, accum);
  }
  for (kernel::Particles::const_iterator a = angles_.begin();
       a != angles_.end(); ++a) {
    score += angle_score_->evaluate(*a, accum);
  }
  for (kernel::Particles::const_iterator d = dihedrals_.begin();
       d != dihedrals_.end(); ++d) {
    score += dihedral_score_->evaluate(*d, accum);
  }
  for (kernel::Particles::const_iterator i = impropers_.begin();
       i != impropers_.end(); ++i) {
    score += improper_score_->evaluate(*i, accum);
  }
  return score;
}

ModelObjectsTemp CHARMMStereochemistryRestraint::do_get_inputs() const {
  kernel::ModelObjectsTemp ps;
  for (kernel::Particles::const_iterator b = bonds_.begin(); b != bonds_.end();
       ++b) {
    ps.push_back(*b);
    kernel::ParticlesTemp bps = bond_score_->get_input_particles(*b);
    ps.insert(ps.end(), bps.begin(), bps.end());
  }
  for (kernel::Particles::const_iterator a = angles_.begin();
       a != angles_.end(); ++a) {
    ps.push_back(*a);
    kernel::ParticlesTemp bps = angle_score_->get_input_particles(*a);
    ps.insert(ps.end(), bps.begin(), bps.end());
  }
  for (kernel::Particles::const_iterator d = dihedrals_.begin();
       d != dihedrals_.end(); ++d) {
    ps.push_back(*d);
    kernel::ParticlesTemp bps = dihedral_score_->get_input_particles(*d);
    ps.insert(ps.end(), bps.begin(), bps.end());
  }
  for (kernel::Particles::const_iterator i = impropers_.begin();
       i != impropers_.end(); ++i) {
    ps.push_back(*i);
    kernel::ParticlesTemp bps = improper_score_->get_input_particles(*i);
    ps.insert(ps.end(), bps.begin(), bps.end());
  }
  return ps;
}

StereochemistryPairFilter *CHARMMStereochemistryRestraint::get_pair_filter() {
  IMP_NEW(StereochemistryPairFilter, pf, ());
  pf->set_bonds(bonds_);
  pf->set_angles(angles_);
  pf->set_dihedrals(dihedrals_);
  return pf.release();
}

StereochemistryPairFilter *CHARMMStereochemistryRestraint::get_full_pair_filter() {
  IMP_NEW(StereochemistryPairFilter, pf, ());
  pf->set_bonds(full_bonds_);
  pf->set_angles(full_angles_);
  pf->set_dihedrals(full_dihedrals_);
  return pf.release();
}


IMPATOM_END_NAMESPACE
