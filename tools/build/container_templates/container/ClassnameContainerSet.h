/**
 *  \file IMP/container/ClassnameContainerSet.h
 *  \brief Store a set of ClassnameContainers
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLASSNAME_CONTAINER_SET_H
#define IMPCONTAINER_CLASSNAME_CONTAINER_SET_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/container_macros.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/scoped.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Stores a set of ClassnameContainers
/** The input sets must be disjoint. This can change if there is
    demand for it.

    \usesconstraint
*/
class IMPCONTAINEREXPORT ClassnameContainerSet : public ClassnameContainer {
  Ints versions_;
  static ClassnameContainerSet *get_set(ClassnameContainer *c) {
    return dynamic_cast<ClassnameContainerSet *>(c);
  }
  virtual std::size_t do_get_contents_hash() const IMP_OVERRIDE;

 public:
  //! Construct and empty set
  ClassnameContainerSet(Model *m,
                        std::string name = "ClassnameContainerSet %1%");

  ClassnameContainerSet(const ClassnameContainersTemp &pc,
                        std::string name = "ClassnameContainerSet %1%");

  /** \brief apply modifier sm to all classname containers */
  IMP_IMPLEMENT(void do_apply(const ClassnameModifier *sm) const IMP_OVERRIDE);

  template <class M>
  void apply_generic(const M *m) const {
    apply(m);
  }

  ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ClassnameContainerSet);

  /** @name Methods to control the nested container

      This container merges a set of nested containers. To add
      or remove nested containers, use the methods below.
  */
  /**@{*/
  IMP_LIST_ACTION(public, ClassnameContainer, ClassnameContainers,
                  CLASSFUNCTIONNAME_container, CLASSFUNCTIONNAME_containers,
                  ClassnameContainer *, ClassnameContainers,
  {
    obj->set_was_used(true);
    versions_.clear();
  },
                  {}, );
/**@}*/

#ifndef IMP_DOXYGEN
  PLURALINDEXTYPE get_indexes() const IMP_OVERRIDE;
  PLURALINDEXTYPE get_range_indexes() const IMP_OVERRIDE;
  ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
#endif

 protected:
  //! Insert the contents of the container into output
  //! instead of output's current content
  virtual void get_indexes_in_place(PLURALINDEXTYPE& output)
    const IMP_OVERRIDE;
};

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_CLASSNAME_CONTAINER_SET_H */
