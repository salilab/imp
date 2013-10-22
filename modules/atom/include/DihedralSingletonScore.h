/**
 *  \file IMP/atom/DihedralSingletonScore.h
 *  \brief A score on a dihedral angle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_DIHEDRAL_SINGLETON_SCORE_H
#define IMPATOM_DIHEDRAL_SINGLETON_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/SingletonScore.h>
#include <IMP/singleton_macros.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the dihedral angle.
/** This scores the dihedral using information stored in its Dihedral
    decorator. The form of the score is \f[
        \frac{1}{2} s|s| (1.0 + \cos(\chi m - \chi_i))
    \f] where \f$s\f$ is the stiffness, \f$m\f$ the multiplicity, \f$\chi_i\f$
    the ideal value of the dihedral, and \f$\chi\f$ the actual value.

    \note This score matches the CHARMM definition, but may not match
          other forcefields. The stiffness can be negative, which corresponds
          to CHARMM forcefield entries with negative force constants.

    \see CHARMMParameters::create_dihedrals(), Dihedral.
 */
class IMPATOMEXPORT DihedralSingletonScore : public SingletonScore {
 public:
  DihedralSingletonScore();
  virtual double evaluate_index(kernel::Model *m, kernel::ParticleIndex p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_SINGLETON_SCORE_METHODS(DihedralSingletonScore);
  IMP_OBJECT_METHODS(DihedralSingletonScore);
  ;
};

IMP_OBJECTS(DihedralSingletonScore, DihedralSingletonScores);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DIHEDRAL_SINGLETON_SCORE_H */
