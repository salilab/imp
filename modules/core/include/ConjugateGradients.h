/**
 *  \file IMP/core/ConjugateGradients.h
 *  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CONJUGATE_GRADIENTS_H
#define IMPCORE_CONJUGATE_GRADIENTS_H

#include <IMP/core/core_config.h>

#include <IMP/AttributeOptimizer.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Simple conjugate gradients optimizer.
/** Algorithm is as per Shanno and Phua, ACM Transactions On Mathematical
    Software 6 (December 1980), 618-622

    Conjugate gradients optimization is sensitive to the scales of the
    derivatives of the various attributes being optimized. By default,
    the scales are estimated from the range of values found for the attribute
    upon initialization. These estimates can be viewed either by calling
    Model::get_range(my_float_key) or by turning on TERSE logging and
    looking at logged messages. If this estimate does not accurately reflect
    the scale, then you can use Model::set_range to set a more accurate
    range for the parameters.

    \note Currently, rigid bodies are not handled (and will not be moved
          by this optimizer).
*/
class IMPCOREEXPORT ConjugateGradients : public AttributeOptimizer {
 public:
  ConjugateGradients(Model *m,
                     std::string name = "ConjugateGradients%1%");
  ConjugateGradients() {}

  //! Set the threshold for the minimum gradient
  void set_gradient_threshold(Float t) { threshold_ = t; }

  //! Limit how far anything can change each time step
  void set_max_change(Float t) { max_change_ = t; }

  virtual Float do_optimize(unsigned int max_steps) override;
  IMP_OBJECT_METHODS(ConjugateGradients);

 private:
  typedef double NT;

  // Handle optimization failing badly
  void failure();

  NT get_score(Vector<FloatIndex> float_indices, Vector<NT> &x,
               Vector<NT> &dscore);
  bool line_search(Vector<NT> &x, Vector<NT> &dx, NT &alpha,
                   const Vector<FloatIndex> &float_indices, int &ifun,
                   NT &f, NT &dg, NT &dg1, int max_steps,
                   const Vector<NT> &search,
                   const Vector<NT> &estimate);
  Float threshold_;
  Float max_change_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<AttributeOptimizer>(this), threshold_, max_change_);
  }
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONJUGATE_GRADIENTS_H */
