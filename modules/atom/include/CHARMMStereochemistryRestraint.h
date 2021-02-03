/**
 * \file IMP/atom/CHARMMStereochemistryRestraint.h
 * \brief Class to maintain CHARMM stereochemistry.
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_STEREOCHEMISTRY_RESTRAINT_H
#define IMPATOM_CHARMM_STEREOCHEMISTRY_RESTRAINT_H

#include <IMP/atom/atom_config.h>
#include <IMP/Pointer.h>
#include <IMP/Restraint.h>
#include "StereochemistryPairFilter.h"
#include "Hierarchy.h"
#include "charmm_segment_topology.h"
#include "angle_decorators.h"
#include "bond_decorators.h"
#include "BondSingletonScore.h"
#include "AngleSingletonScore.h"
#include "DihedralSingletonScore.h"
#include "ImproperSingletonScore.h"
#include "Selection.h"

IMPATOM_BEGIN_NAMESPACE

//! Enforce CHARMM stereochemistry on the given Hierarchy.
/** It is assumed that the Hierarchy has already had CHARMM atom types
    assigned and conforms with the CHARMM topology information
    (for example, by calling CHARMMTopology::setup_hierarchy() first).

    \note This is a convenient high-level wrapper; the bonds, angles,
          dihedrals and impropers can also be created manually and
          evaluated using standard IMP building blocks - for example,
          angles can be created using CHARMMParameters::create_angles()
          and then evaluated using an AngleSingletonScore in combination
          with a container::SingletonsRestraint.
 */
class IMPATOMEXPORT CHARMMStereochemistryRestraint : public Restraint {
  Particles bonds_, angles_, dihedrals_, impropers_;
  Particles full_bonds_, full_angles_, full_dihedrals_, full_impropers_;
  IMP::PointerMember<BondSingletonScore> bond_score_;
  IMP::PointerMember<AngleSingletonScore> angle_score_;
  IMP::PointerMember<DihedralSingletonScore> dihedral_score_;
  IMP::PointerMember<ImproperSingletonScore> improper_score_;
#ifndef IMP_DOXYGEN
  void init(Hierarchy h, CHARMMTopology *topology);
#endif
 public:
  CHARMMStereochemistryRestraint(Hierarchy h, CHARMMTopology *topology);

  //! Initialize the restraint and limit to a set of particles
  /** Will only create restraints where every particle in the restraint
      is in the provided list.
  */
  CHARMMStereochemistryRestraint(Hierarchy h, CHARMMTopology *topology,
                                 ParticlesTemp limit_to_these_particles);

  //! Get a PairFilter that excludes all stereochemical pairs.
  /** \return a StereochemistryPairFilter that excludes all 1-2 (bond),
              1-3 (angle) and 1-4 (dihedral) pairs.
   */
  StereochemistryPairFilter *get_pair_filter();

  //! Get a PairFilter including everything from original topology
  /** \return a StereochemistryPairFilter that excludes all 1-2 (bond),
              1-3 (angle) and 1-4 (dihedral) pairs before limiting to selection.
   */
  StereochemistryPairFilter *get_full_pair_filter();

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(CHARMMStereochemistryRestraint);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARMM_STEREOCHEMISTRY_RESTRAINT_H */
