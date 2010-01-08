/**
 *  \file Sampler.h     \brief Base class for all samplers.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_SAMPLER_H
#define IMP_SAMPLER_H

#include "config.h"
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
  double max_score_;
  typedef std::map<Restraint*, double> Maxes;
  Maxes max_scores_;
 public:
  Sampler(Model *m, std::string name="Sampler %1%");

  virtual ConfigurationSet *sample() const =0;

  /** \name Filtering
     The set of returned configurations can be filtered on a variety
     of criteria.
      @{
   */
  //! Set the maximum allowable score for the whole model
  void set_maximum_score(double s) {max_score_=s;}
  //! Set the maximum allowable score for a restraint
  void set_maximum_score(Restraint *r, double s) {
    max_scores_[r]=s;
  }
  double get_maximum_score() const {return max_score_;}
  /** @} */

  Model *get_model() const {return model_;}

  // for the vtable
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Sampler);
 protected:
  /** The Sampler can contain a number of filters which limit
      the set of configurations which are saved. The sampler
      should check that a state passes the filters before adding
      it to the returned ConfigurationSet.
  */
  bool get_is_good_configuration() const;
};



IMP_END_NAMESPACE

#endif  /* IMP_SAMPLER_H */
