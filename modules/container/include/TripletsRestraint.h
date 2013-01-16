/**
 *  \file IMP/container/TripletsRestraint.h
 *  \brief Apply a TripletScore to each Triplet in a list.
 *
 *  WARNING This file was generated from NAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_TRIPLETS_RESTRAINT_H
#define IMPCONTAINER_TRIPLETS_RESTRAINT_H

#include <IMP/container/container_config.h>

#include <IMP/internal/InternalTripletsRestraint.h>

#include <iostream>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a TripletScore to each Triplet in a list.
/** This restraint stores the used particles in a ParticleTripletsTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListTripletContainer is used and the
    {add_, set_, clear_}particle_triplet{s} methodas can be used.

    Examples using various multiplicity containers:
    \pythonexample{restrain_in_sphere}
    \pythonexample{nonbonded_interactions}

    \see IMP::core::TripletRestraint
 */
class IMPCONTAINEREXPORT TripletsRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
public Restraint
#else
public IMP::internal::InternalTripletsRestraint
#endif
{
  typedef IMP::internal::InternalTripletsRestraint P;
public:

 //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
      \param[in] name The object name
   */
  TripletsRestraint(TripletScore *ss,
                      TripletContainerAdaptor pc,
                      std::string name="TripletsRestraint %1%"):
      P(ss, pc, name)
      {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
 double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
 IMP::ModelObjectsTemp do_get_inputs() const;
 IMP_OBJECT_METHODS(TripletsRestraint)
#endif
};

IMP_OBJECTS(TripletsRestraint,TripletsRestraints);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_TRIPLETS_RESTRAINT_H */
