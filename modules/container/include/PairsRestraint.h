/**
 *  \file IMP/container/PairsRestraint.h
 *  \brief Apply a PairScore to each Pair in a list.
 *
 *  WARNING This file was generated from NAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_PAIRS_RESTRAINT_H
#define IMPCONTAINER_PAIRS_RESTRAINT_H

#include <IMP/container/container_config.h>

#include <IMP/internal/InternalPairsRestraint.h>

#include <iostream>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a PairScore to each Pair in a list.
/** This restraint stores the used particles in a ParticlePairsTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListPairContainer is used and the
    {add_, set_, clear_}particle_pair{s} methodas can be used.

    Examples using various multiplicity containers:
    \pythonexample{restrain_in_sphere}
    \pythonexample{nonbonded_interactions}

    \see IMP::core::PairRestraint
 */
class IMPCONTAINEREXPORT PairsRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
public Restraint
#else
public IMP::internal::InternalPairsRestraint
#endif
{
  typedef IMP::internal::InternalPairsRestraint P;
public:

 //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
      \param[in] name The object name
   */
  PairsRestraint(PairScore *ss,
                      PairContainerAdaptor pc,
                      std::string name="PairsRestraint %1%"):
      P(ss, pc, name)
      {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
 double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
 IMP::ModelObjectsTemp do_get_inputs() const;
 IMP_OBJECT_METHODS(PairsRestraint)
#endif
};

IMP_OBJECTS(PairsRestraint,PairsRestraints);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_PAIRS_RESTRAINT_H */
