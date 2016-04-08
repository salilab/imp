/**
 * \file IMP/atom/HelixRestraint.h
 * \brief Class to maintain helix shape (dihedrals + elastic network)
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_HELIX_RESTRAINT_H
#define IMPATOM_HELIX_RESTRAINT_H

#include <IMP/atom/atom_config.h>
#include <IMP/atom/DihedralSingletonScore.h>
#include <IMP/atom/angle_decorators.h>
#include <IMP/Pointer.h>
#include <IMP/Restraint.h>
#include <IMP/core/DistancePairScore.h>


IMPATOM_BEGIN_NAMESPACE

//! Restraint a set of residues to use ideal helix dihedrals and bonds
class IMPATOMEXPORT HelixRestraint : public Restraint {
  IMP::PointerMember<DihedralSingletonScore> dihedral_score_;
  IMP::PointerMember<core::HarmonicDistancePairScore> bond_ON_score_;
  ParticleIndexes dihedrals_;
  ParticleIndexPairs bonds_ON_;
 public:
  HelixRestraint(Residues rs);

  //! Count bonds
  int get_number_of_bonds(){return bonds_ON_.size();}

  //! Count dihedrals
  int get_number_of_dihedrals(){return dihedrals_.size();}

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(HelixRestraint);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_HELIX_RESTRAINT_H */
