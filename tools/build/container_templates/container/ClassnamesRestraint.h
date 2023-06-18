/**
 *  \file IMP/container/ClassnamesRestraint.h
 *  \brief Apply a ClassnameScore to each Classname in a list.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_CLASSNAMES_RESTRAINT_H
#define IMPCONTAINER_CLASSNAMES_RESTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/internal/ContainerRestraint.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/ClassnameScore.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a ClassnameScore to each Classname in a list.
/** This restraint stores the used particle indexes in a PLURALINDEXTYPE.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used).

    Examples using various multiplicity containers:
    \include restrain_in_sphere.py
    \include nonbonded_interactions.py

    \see IMP::core::ClassnameRestraint
 */
class ClassnamesRestraint :
#if defined(SWIG) || defined(IMP_DOXYGEN)
    public Restraint
#else
    public IMP::internal::ContainerRestraint<ClassnameScore, ClassnameContainer>
#endif
    {
  typedef IMP::internal::ContainerRestraint<
      ClassnameScore, ClassnameContainer> P;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<P>(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(ClassnamesRestraint);

 public:
  //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
      \param[in] name The object name
   */
  ClassnamesRestraint(ClassnameScore *ss, ClassnameContainerAdaptor pc,
                      std::string name = "ClassnamesRestraint %1%")
      : P(ss, pc, name) {}

  ClassnamesRestraint() {}

#if defined(IMP_DOXYGEN) || defined(SWIG)
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  IMP::ModelObjectsTemp do_get_inputs() const;
  IMP_OBJECT_METHODS(ClassnamesRestraint)
#endif
};

IMP_OBJECTS(ClassnamesRestraint, ClassnamesRestraints);

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CLASSNAMES_RESTRAINT_H */
