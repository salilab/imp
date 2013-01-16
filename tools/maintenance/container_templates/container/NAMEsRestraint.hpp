/**
 *  \file IMP/container/CLASSNAMEsRestraint.h
 *  \brief Apply a CLASSNAMEScore to each CLASSNAME in a list.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_HEADERNAMES_RESTRAINT_H
#define IMPCONTAINER_HEADERNAMES_RESTRAINT_H

#include <IMP/container/container_config.h>

#include <IMP/internal/InternalCLASSNAMEsRestraint.h>

#include <iostream>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a CLASSNAMEScore to each CLASSNAME in a list.
/** This restraint stores the used particles in a PLURALVARIABLETYPE.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListCLASSNAMEContainer is used and the
    {add_, set_, clear_}FUNCTIONNAME{s} methodas can be used.

    Examples using various multiplicity containers:
    \pythonexample{restrain_in_sphere}
    \pythonexample{nonbonded_interactions}

    \see IMP::core::CLASSNAMERestraint
 */
class IMPCONTAINEREXPORT CLASSNAMEsRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
public Restraint
#else
public IMP::internal::InternalCLASSNAMEsRestraint
#endif
{
  typedef IMP::internal::InternalCLASSNAMEsRestraint P;
public:

 //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
      \param[in] name The object name
   */
  CLASSNAMEsRestraint(CLASSNAMEScore *ss,
                      CLASSNAMEContainerAdaptor pc,
                      std::string name="CLASSNAMEsRestraint %1%"):
      P(ss, pc, name)
      {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
 double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
 IMP::ModelObjectsTemp do_get_inputs() const;
 IMP_OBJECT_METHODS(CLASSNAMEsRestraint)
#endif
};

IMP_OBJECTS(CLASSNAMEsRestraint,CLASSNAMEsRestraints);

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_HEADERNAMES_RESTRAINT_H */
