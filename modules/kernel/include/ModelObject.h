/**
 *  \file IMP/kernel/ModelObject.h    \brief Single variable function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_MODEL_OBJECT_H
#define IMPKERNEL_MODEL_OBJECT_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include <IMP/base/ref_counted_macros.h>
#include <IMP/base/tracking.h>
#include <IMP/base/utility_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

/** These objects are associated with a particular Model
    and have a callback that is called whenever the dependencies
    in the model change. This allows them to update internal state
    when that occurs.
 */
class IMPKERNELEXPORT ModelObject :
    public base::TrackedObject<ModelObject, Model> {
  typedef base::TrackedObject<ModelObject, Model> Tracked;

  friend class Model;

  bool has_dependencies_;
  ScoreStatesTemp required_score_states_;

 public:
#if !defined(IMP_DOXYGEN)
  void set_has_dependencies(bool t, const ScoreStatesTemp &ss);
#endif

  ModelObject(Model *m, std::string name);

  Model *get_model() const { return Tracked::get_tracker(); }
  /** Get any Particle, Container or other ModelObjects read by
      this during evaluation. If you read everything in a container,
  you can just return that container. */
  ModelObjectsTemp get_inputs() const;
  /** Get any Particle, Container or other ModelObjects changed by
      this during evaluation. This is only useful for ScoreStates,
      at the moment.*/
  ModelObjectsTemp get_outputs() const;
  /** Get the interacting sets induce by this ModelObject. That is,
      the particles in each ModelObjectsTemp in the list have some
      sort of computed relation with one another and none with
      disjoint other sets in the list.*/
  ModelObjectsTemps get_interactions() const;

  //! Return whether this object has dependencies computed
  bool get_has_dependencies() const { return has_dependencies_; }

  /** Either invalidate the dependncies or ensure they are correct.*/
  void set_has_dependencies(bool tf);

  /** Get the score states that are ancestors of this in the dependency graph.
   */
  const ScoreStatesTemp &get_required_score_states() {
    set_has_dependencies(true);
    return required_score_states_;
  }

 protected:
  /** Called when set_has_dependencies() is called.*/
  virtual void do_set_has_dependencies(bool) {}
  /** Override if this reads other objects during evaluate.*/
  virtual ModelObjectsTemp do_get_inputs() const = 0;
  /** Override if this writes other objects during evaluate.*/
  virtual ModelObjectsTemp do_get_outputs() const = 0;
  /** Override if this if not all inputs interact with all outputs. This is
      rarely something you want to do.*/
  virtual ModelObjectsTemps do_get_interactions() const;

  IMP_REF_COUNTED_DESTRUCTOR(ModelObject);


  /** \deprecated_at{2.1} Use the constructor that takes a Model. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  ModelObject(std::string name);
  /** \deprecated_at{2.1} Use the constructor that takes a Model. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  virtual void set_model(Model *m);
  /** \deprecated_at{2.1} Should always be true. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  bool get_is_part_of_model() const;
  /** \deprecated_at{2.1} As it should always be part of a model. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  virtual void do_set_model(Model *) {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_MODEL_OBJECT_H */
