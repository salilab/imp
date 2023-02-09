/**
 *  \file IMP/ModelObject.h
 *  \brief Base class for objects in a Model that depend on other objects.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_MODEL_OBJECT_H
#define IMPKERNEL_MODEL_OBJECT_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include <IMP/ref_counted_macros.h>
#include <IMP/utility_macros.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/base_object.hpp>

IMPKERNEL_BEGIN_NAMESPACE

class Model;

//! Base class for objects in a Model that depend on other objects.
/** These objects are associated with a particular Model
    and have a callback that is called whenever the dependencies
    in the model change. This allows them to update internal state
    when that occurs.
 */
class IMPKERNELEXPORT ModelObject : public Object {
  friend class Model;
  WeakPointer<Model> model_;

#ifndef SWIG
  friend class boost::serialization::access;

  template<class Archive> void save(Archive &ar, const unsigned int) const {
    ar << boost::serialization::base_object<Object>(*this);
    uint32_t model_id = get_model_id();
    ar << model_id;
  }

  template<class Archive> void load(Archive &ar, const unsigned int) {
    uint32_t model_id;
    ar >> boost::serialization::base_object<Object>(*this);
    ar >> model_id;
    set_model_from_id(model_id);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  void set_model_from_id(uint32_t model_id);
  uint32_t get_model_id() const;
#endif

  // for cleanup
  void set_model(Model *m);

 public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void validate_inputs() const;
  void validate_outputs() const;
#endif

  ModelObject(Model *m, std::string name);
  ModelObject();
  ~ModelObject();

  Model *get_model() const { return model_; }
  /** get_has_dependencies() must be true. */
  ModelObjectsTemp get_inputs() const;
  /** get_has_dependencies() must be true. */
  ModelObjectsTemp get_outputs() const;
  //! Get the interacting sets induced by this ModelObject.
  /** That is, the particles in each ModelObjectsTemp in the list have some
      sort of computed relation with one another and none with
      disjoint other sets in the list. */
  ModelObjectsTemps get_interactions() const;

  //! Return whether this object has dependencies computed
  bool get_has_dependencies() const;

  //! Either invalidate the dependencies or ensure they are correct.
  void set_has_dependencies(bool tf);

  //! Compute the required score states.
  void set_has_required_score_states(bool tf);

  //! Return whether score states are computed.
  bool get_has_required_score_states() const;

  //! Get the score states that are ancestors of this in the dependency graph.
  const ScoreStatesTemp &get_required_score_states() const;

 protected:
  // virtual void do_destroy() override {set_has_dependencies(false);}
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
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_MODEL_OBJECT_H */
