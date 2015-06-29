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
  mutable Pointer<ScoringFunction> cache_;
  PointerMember<ScoringFunction> sf_;

 public:
  Sampler(Model *m, std::string name = "Sampler %1%");
  ConfigurationSet *create_sample() const;

  ScoringFunction *get_scoring_function() const {
    if (sf_) {
      return sf_;
    } else if (cache_) {
      return cache_;
    } else {
/* Don't warn about deprecated model scoring function every time someone
   includes Sampler.h */
IMP_HELPER_MACRO_PUSH_WARNINGS
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 5)
IMP_GCC_PRAGMA(diagnostic ignored "-Wdeprecated-declarations")
#endif
      return cache_ = get_model()->create_model_scoring_function();
IMP_HELPER_MACRO_POP_WARNINGS
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
