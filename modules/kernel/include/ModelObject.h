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
#include <IMP/base/utility_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

class Model;

/** These objects are associated with a particular Model
    and have a callback that is called whenever the dependencies
    in the model change. This allows them to update internal state
    when that occurs.
 */
class IMPKERNELEXPORT ModelObject: public base::Object {
  friend class Model;
  base::WeakPointer<Model> model_;
 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void validate_inputs() const;
  void validate_outputs() const;
#endif

  ModelObject(kernel::Model *m, std::string name);
  ~ModelObject();

  Model *get_model() const { return model_; }
  /** get_has_dependencies() must be true. */
  ModelObjectsTemp get_inputs() const;
  /** get_has_dependencies() must be true. */
  ModelObjectsTemp get_outputs() const;
  /** Get the interacting sets induce by this ModelObject. That is,
      the particles in each ModelObjectsTemp in the list have some
      sort of computed relation with one another and none with
      disjoint other sets in the list.*/
  ModelObjectsTemps get_interactions() const;

  //! Return whether this object has dependencies computed
  bool get_has_dependencies() const;

  /** Either invalidate the dependncies or ensure they are correct.*/
  void set_has_dependencies(bool tf);

  /** Compute the required score states. */
  void set_has_required_score_states(bool tf);

  /** Return whether score states are computed.*/
  bool get_has_required_score_states() const;

  /** Get the score states that are ancestors of this in the dependency graph.
   */
  const ScoreStatesTemp &get_required_score_states() const;
 protected:
  //virtual void do_destroy() IMP_OVERRIDE {set_has_dependencies(false);}
  /** Called when set_has_required_score_states() is called.*/
  virtual void handle_set_has_required_score_states(bool) {}
  /** Get any Particle, Container or other ModelObjects read by
      this during evaluation. If you read everything in a container,
      you can just return that container. */
  virtual ModelObjectsTemp do_get_inputs() const = 0;
  /** Get any Particle, Container or other ModelObjects changed by
      this during evaluation. This is only useful for ScoreStates,
      at the moment.*/
  virtual ModelObjectsTemp do_get_outputs() const = 0;
  /** Override if this if not all inputs interact with all outputs. This is
      rarely something you want to do.*/
  virtual ModelObjectsTemps do_get_interactions() const;

 public:
  /** \deprecated_at{2.1} Use the constructor that takes a Model. */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  ModelObject(std::string name);
  /** \deprecated_at{2.1} Use the constructor that takes a Model. */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  virtual void set_model(kernel::Model *m);
  /** \deprecated_at{2.1} Should always be true. */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  bool get_is_part_of_model() const;
  /** \deprecated_at{2.1} As it should always be part of a model. */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  virtual void do_set_model(kernel::Model *) {}
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_MODEL_OBJECT_H */
