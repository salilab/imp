/**
 *  \file IMP/kernel/ModelObject.h    \brief Single variable function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_MODEL_OBJECT_H
#define IMPKERNEL_MODEL_OBJECT_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "dependency_graph.h"
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
#if defined(IMP_DOXYGEN) || defined(SWIG)
    public base::Object
#else
    public base::TrackedObject<ModelObject, Model>
#endif
{
  typedef  base::TrackedObject<ModelObject, Model> Tracked;

  friend class Model;
public:
  ModelObject(Model *m, std::string name);
#ifndef IMP_DOXYGEN
  ModelObject(std::string name);
  /** Virtual so that RestraintSet can override it.*/
  virtual void set_model(Model *m);
  bool get_is_part_of_model() const {
    return Tracked::get_is_tracked();
  }
#endif
  Model *get_model() const {
     return Tracked::get_tracker();
  }
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
protected:
#ifndef SWIG
  // too hard to make swig handle this
  /** This method is called when the dependencies in the model have changed
      and model evaluate is called (or Model::ensure_dependencies()).
      The object can use this to update anything
      that is needed for efficient computation.*/
  virtual void do_update_dependencies(){
    // swig is being braindead and not matching this function successfully
  }
#endif
  /** The model calls this method when dependencies have changed. It in
      turn calls do_update_dependencies().*/
  virtual void update_dependencies();
  /** Override if this reads other objects during evaluate.*/
  virtual ModelObjectsTemp do_get_inputs() const =0;
  /** Override if this writes other objects during evaluate.*/
  virtual ModelObjectsTemp do_get_outputs() const =0;
  /** Override if this if not all inputs interact with all outputs. This is
      rarely something you want to do.*/
  virtual ModelObjectsTemps do_get_interactions() const;

  IMP_REF_COUNTED_DESTRUCTOR(ModelObject);
};

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_MODEL_OBJECT_H */
