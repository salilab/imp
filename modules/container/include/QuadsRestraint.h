/**
 *  \file IMP/container/QuadsRestraint.h
 *  \brief Apply a QuadScore to each Quad in a list.
 *
 *  WARNING This file was generated from NAMEsRestraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_QUADS_RESTRAINT_H
#define IMPCONTAINER_QUADS_RESTRAINT_H

#include <IMP/container/container_config.h>

#include <IMP/internal/InternalQuadsRestraint.h>

#include <iostream>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a QuadScore to each Quad in a list.
/** This restraint stores the used particles in a ParticleQuadsTemp.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListQuadContainer is used and the
    {add_, set_, clear_}particle_quad{s} methodas can be used.

    Examples using various multiplicity containers:
    \pythonexample{restrain_in_sphere}
    \pythonexample{nonbonded_interactions}

    \see IMP::core::QuadRestraint
 */
class IMPCONTAINEREXPORT QuadsRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
public Restraint
#else
public IMP::kernel::internal::InternalQuadsRestraint
#endif
{
  typedef IMP::kernel::internal::InternalQuadsRestraint P;
public:

 //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
      \param[in] name The object name
   */
  QuadsRestraint(QuadScore *ss,
                      QuadContainerAdaptor pc,
                      std::string name="QuadsRestraint %1%"):
      P(ss, pc, name)
      {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
 double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
 IMP::ModelObjectsTemp do_get_inputs() const;
 IMP_OBJECT_METHODS(QuadsRestraint)
#endif
};

IMP_OBJECTS(QuadsRestraint,QuadsRestraints);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_QUADS_RESTRAINT_H */
