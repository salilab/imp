/**
 *  \file IMP/container/ClassnamesRestraint.h
 *  \brief Apply a ClassnameScore to each Classname in a list.
 *
 *  BLURB
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_CLASSNAMES_RESTRAINT_H
#define IMPCONTAINER_CLASSNAMES_RESTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/kernel/internal/ContainerRestraint.h>
#include <IMP/kernel/ClassnameContainer.h>
#include <IMP/kernel/ClassnameScore.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a ClassnameScore to each Classname in a list.
/** This restraint stores the used particles in a PLURALVARIABLETYPE.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used).

    Examples using various multiplicity containers:
    \include restrain_in_sphere.py
    \include nonbonded_interactions.py

    \see IMP::core::ClassnameRestraint
 */
class ClassnamesRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
    public kernel::Restraint
#else
    public IMP::kernel::internal::ContainerRestraint<kernel::ClassnameScore,
                                                     kernel::ClassnameContainer>
#endif
    {
  typedef IMP::kernel::internal::ContainerRestraint<
      kernel::ClassnameScore, kernel::ClassnameContainer> P;

 public:
  //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
      \param[in] name The object name
   */
  ClassnamesRestraint(kernel::ClassnameScore *ss,
                      kernel::ClassnameContainerAdaptor pc,
                      std::string name = "ClassnamesRestraint %1%")
      : P(ss, pc, name) {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
  double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) const;
  IMP::kernel::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(ClassnamesRestraint)
#endif
};

IMP_OBJECTS(ClassnamesRestraint, ClassnamesRestraints);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CLASSNAMES_RESTRAINT_H */
