/**
 *  \file IMP/Sampler.h     \brief Base class for all samplers.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SAMPLER_H
#define IMPKERNEL_SAMPLER_H

#include <IMP/kernel_config.h>
#include "Model.h"
#include <IMP/Pointer.h>
#include <IMP/Object.h>
#include "ConfigurationSet.h"
#include <IMP/deprecation_macros.h>
#include <IMP/ref_counted_macros.h>

IMPKERNEL_BEGIN_NAMESPACE

//! Base class for all samplers.
/** A sampler takes a Model and searches for good configurations,
    given the optimizable parameters and the scoring function in
    the Model and extra information that can be provided. Typically,
    the current configuration of the model is ignored.

    Typically a sampler works by using one or more Optimizer
    types to search for configurations which minimize the scoring
    function.
*/
class IMPKERNELEXPORT Sampler : public IMP::Object {
  PointerMember<Model> model_;
  PointerMember<ScoringFunction> sf_;

 public:
  Sampler(Model *m, std::string name = "Sampler %1%");
  ConfigurationSet *create_sample() const;

  //! Return the scoring function that is being used
  /** \throws ValueException if no scoring function was set
   */
  ScoringFunction *get_scoring_function() const {
    if (sf_) {
      return sf_;
    } else {
      IMP_THROW("No scoring function was set. "
                "Use Sampler::set_scoring_function() to set one.",
                ValueException);
    }
  }

  void set_scoring_function(ScoringFunctionAdaptor sf);

  Model *get_model() const { return model_; }

 protected:
  virtual ConfigurationSet *do_sample() const = 0;

  // for the vtable
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Sampler);
  //! Subclasses should override this method
};

IMP_OBJECTS(Sampler, Samplers);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SAMPLER_H */
