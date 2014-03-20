/**
 * \file IMP/atom/CHARMMStereochemistryRestraint.h
 * \brief Class to maintain CHARMM stereochemistry.
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_STEREOCHEMISTRY_RESTRAINT_H
#define IMPATOM_CHARMM_STEREOCHEMISTRY_RESTRAINT_H

#include <IMP/atom/atom_config.h>
#include <IMP/base/Pointer.h>
#include <IMP/kernel/Restraint.h>
#include "StereochemistryPairFilter.h"
#include "Hierarchy.h"
#include "charmm_segment_topology.h"
#include "BondSingletonScore.h"
#include "AngleSingletonScore.h"
#include "DihedralSingletonScore.h"
#include "ImproperSingletonScore.h"

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
class IMPATOMEXPORT CHARMMStereochemistryRestraint : public kernel::Restraint {
  kernel::Particles bonds_, angles_, dihedrals_, impropers_;
  IMP::base::PointerMember<BondSingletonScore> bond_score_;
  IMP::base::PointerMember<AngleSingletonScore> angle_score_;
  IMP::base::PointerMember<DihedralSingletonScore> dihedral_score_;
  IMP::base::PointerMember<ImproperSingletonScore> improper_score_;

 public:
  CHARMMStereochemistryRestraint(Hierarchy h, CHARMMTopology *topology);

  //! Get a PairFilter that excludes all stereochemical pairs.
  /** \return a StereochemistryPairFilter that excludes all 1-2 (bond),
              1-3 (angle) and 1-4 (dihedral) pairs.
   */
  StereochemistryPairFilter *get_pair_filter();

  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(CHARMMStereochemistryRestraint);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARMM_STEREOCHEMISTRY_RESTRAINT_H */
