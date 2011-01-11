/**
 *  \file Sampler.h     \brief Base class for all samplers.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SAMPLER_H
#define IMP_SAMPLER_H

#include "kernel_config.h"
#include "Model.h"
#include "macros.h"
#include "Pointer.h"
#include "ConfigurationSet.h"

IMP_BEGIN_NAMESPACE

//! Base class for all samplers.
/** A sampler takes a Model and searches for good configurations,
    given the optimizeable parameters and the scoring function in
    the Model and extra information that can be provided.

    Typically a sampler works by using one or more Optimizer
    types to search for configurations which minimize the scoring
    function.
*/
class IMPEXPORT Sampler: public Object
{
  internal::OwnerPointer<Model> model_;
 public:
  Sampler(Model *m, std::string name="Sampler %1%");

  ConfigurationSet *get_sample() const;

#ifndef IMP_DOXYGEN
  void set_maximum_score(double s) {model_->set_maximum_score(s);}
  //! Set the maximum allowable score for a restraint
  void set_maximum_score(Restraint *r, double s) {
    model_->set_maximum_score(r,s);
  }
  double get_maximum_score() const {return model_->get_maximum_score();}
  double get_maximum_score(Restraint*r) const {
    return model_->get_maximum_score(r);
  }
#endif

  Model *get_model() const {return model_;}

  // for the vtable
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Sampler);
 protected:
  //! Subclasses should override this method
  virtual ConfigurationSet* do_sample() const=0;
};

IMP_OBJECTS(Sampler,Samplers);

IMP_END_NAMESPACE

#endif  /* IMP_SAMPLER_H */
