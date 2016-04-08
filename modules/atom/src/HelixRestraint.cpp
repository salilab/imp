/**
 * \file IMP/atom/HelixRestraint.cpp
 * \brief Class to maintain helix shape (dihedrals + elastic network)
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/HelixRestraint.h>
#include <IMP/core/internal/dihedral_helpers.h>

IMPATOM_BEGIN_NAMESPACE

HelixRestraint::HelixRestraint(Residues rs)
    : Restraint(rs[0]->get_model(), "HelixRestraint%1%") {
  //will expand to more bonds and residue types
  bond_ON_score_ = new core::HarmonicDistancePairScore(2.96, 0.11);
  dihedral_score_ = new DihedralSingletonScore();
  for(size_t nr=0;nr<rs.size()-2;nr++){
    core::XYZ a1(get_atom(rs[nr],AT_C));
    core::XYZ a2(get_atom(rs[nr+1],AT_N));
    core::XYZ a3(get_atom(rs[nr+1],AT_CA));
    core::XYZ a4(get_atom(rs[nr+1],AT_C));
    core::XYZ a5(get_atom(rs[nr+2],AT_N));
    IMP_NEW(Particle,dp1,(get_model()));
    IMP_NEW(Particle,dp2,(get_model()));
    Dihedral d1 = Dihedral::setup_particle(dp1,a1,a2,a3,a4);
    d1.set_ideal(-1.117010721276371); //-64deg
    d1.set_stiffness(6.0);
    Dihedral d2 = Dihedral::setup_particle(dp2,a2,a3,a4,a5);
    d2.set_ideal(-0.715584993317675); //-41 deg
    d2.set_stiffness(8.0);
    dihedrals_.push_back(dp1->get_index());
    dihedrals_.push_back(dp2->get_index());
  }

  for(size_t nr=0;nr<rs.size()-4;nr++){
    bonds_ON_.push_back(ParticleIndexPair(
                         get_atom(rs[nr],AT_O).get_particle_index(),
                         get_atom(rs[nr+4],AT_N).get_particle_index()));
  }
}

Float HelixRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const {
  Float score = 0.0;
  Model *m = get_model();
  for (ParticleIndexes::const_iterator td = dihedrals_.begin();
       td != dihedrals_.end(); ++td) {
    score += dihedral_score_->evaluate_index(m, *td, accum);
  }
  for (ParticleIndexPairs::const_iterator tb = bonds_ON_.begin();
       tb != bonds_ON_.end(); ++tb) {
    score += bond_ON_score_->evaluate_index(m, *tb, accum);
  }
  return score;
}

ModelObjectsTemp HelixRestraint::do_get_inputs() const {
  ModelObjectsTemp ps;
  Model *m = get_model();
  for (ParticleIndexes::const_iterator d = dihedrals_.begin();
       d != dihedrals_.end(); ++d) {
    ps.push_back(m->get_particle(*d));
    ModelObjectsTemp bps = dihedral_score_->get_inputs(m,
                                                       ParticleIndexes(1, *d));
    ps.insert(ps.end(), bps.begin(), bps.end());
  }
  for (ParticleIndexPairs::const_iterator tb = bonds_ON_.begin();
       tb != bonds_ON_.end(); ++tb) {
    ps.push_back(m->get_particle((*tb)[0]));
    ps.push_back(m->get_particle((*tb)[1]));
  }

  return ps;
}
IMPATOM_END_NAMESPACE
