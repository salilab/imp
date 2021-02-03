/**
 * \file IMP/atom/HelixRestraint.cpp
 * \brief Class to maintain helix shape (dihedrals + elastic network)
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/HelixRestraint.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/core/Harmonic.h>

IMPATOM_BEGIN_NAMESPACE

HelixRestraint::HelixRestraint(Residues rs, bool ideal)
    : Restraint(rs[0]->get_model(), "HelixRestraint%1%") {
  //dihedral info
  //Float dih1,std1,dih2,std2,distON,kON;
  Float dih1  = -1.117010721276371; //mean = -64deg, std = 6deg
  Float std1  = 0.10471975511965977;
  Float dih2  = -0.715584993317675; //mean = -41deg, std = 8deg
  Float std2  = 0.13962634015954636;
  Float corr = -0.25;
  Float weight = 0.5;
  core::BinormalTerm bt;
  bt.set_means(std::make_pair(dih1,dih2));
  bt.set_standard_deviations(std::make_pair(std1,std2));
  bt.set_correlation(corr);
  bt.set_weight(weight);

  //bond info
  Float distON = 2.96;
  Float kON = core::Harmonic::get_k_from_standard_deviation(0.11);

  //will expand to more bonds and residue types
  bond_ON_score_ = new core::HarmonicDistancePairScore(distON,kON);

  if (rs.size()>=3){
    for(size_t nr=0;nr<rs.size()-2;nr++){
      core::XYZ a1(get_atom(rs[nr],AT_C));
      core::XYZ a2(get_atom(rs[nr+1],AT_N));
      core::XYZ a3(get_atom(rs[nr+1],AT_CA));
      core::XYZ a4(get_atom(rs[nr+1],AT_C));
      core::XYZ a5(get_atom(rs[nr+2],AT_N));
      IMP_NEW(core::MultipleBinormalRestraint,
              mbr,(get_model(),
                   ParticleIndexQuad(a1,a2,a3,a4),
                   ParticleIndexQuad(a2,a3,a4,a5)));
      mbr->add_term(bt);
      dihedral_rs_.push_back(mbr);
    }
  }

  if (rs.size()>=5){
    for(size_t nr=0;nr<rs.size()-4;nr++){
      bonds_ON_.push_back(ParticleIndexPair(
                                            get_atom(rs[nr],AT_O).get_particle_index(),
                                            get_atom(rs[nr+4],AT_N).get_particle_index()));
    }
  }
}

Float HelixRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const {
  Float score = 0.0;
  Model *m = get_model();
  for (IMP::Vector<IMP::PointerMember<
         core::MultipleBinormalRestraint> >::const_iterator td = dihedral_rs_.begin();
       td != dihedral_rs_.end(); ++td){
    score += (*td)->unprotected_evaluate(accum);
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
  for (IMP::Vector<IMP::PointerMember<
         core::MultipleBinormalRestraint> >::const_iterator td = dihedral_rs_.begin();
       td != dihedral_rs_.end(); ++td){
    ModelObjectsTemp bps = (*td)->get_inputs();
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
