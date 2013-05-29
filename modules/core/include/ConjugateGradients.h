/**
 *  \file IMP/core/ConjugateGradients.h
 *  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CONJUGATE_GRADIENTS_H
#define IMPCORE_CONJUGATE_GRADIENTS_H

#include <IMP/core/core_config.h>

#include <IMP/Optimizer.h>
#include <IMP/optimizer_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! Simple conjugate gradients optimizer.
/** Algorithm is as per Shanno and Phua, ACM Transactions On Mathematical
    Software 6 (December 1980), 618-622

    Conjugate gradients optimization is sensitive to the scales of the
    derivatives of the various attributes being optimized. By default,
    the scales are estimated from the range of values found for the attribute
    upon initialization. These estimates can be viewed either by calling
    Model::get_range(my_float_key) or by turning on TERSE logging and looking
    at logged messages. If this estimate does not accurately reflect the
    scale, then you can use Model::set_range to set a more accurate range
    for the parameters.
*/
class IMPCOREEXPORT ConjugateGradients : public Optimizer {
 public:
  ConjugateGradients(Model *m = nullptr);

  //! Set the threshold for the minimum gradient
  void set_gradient_threshold(Float t) { threshold_ = t; }

#ifndef IMP_DOXYGEN
  void set_threshold(Float t) { set_gradient_threshold(t); }
#endif

  //! Limit how far anything can change each time step
  void set_max_change(Float t) { max_change_ = t; }

  IMP_OPTIMIZER(ConjugateGradients);

 private:

  typedef double NT;

  // Handle optimization failing badly
  void failure();

  NT get_score(base::Vector<FloatIndex> float_indices, base::Vector<NT> &x,
               base::Vector<NT> &dscore);
  bool line_search(base::Vector<NT> &x, base::Vector<NT> &dx, NT &alpha,
                   const base::Vector<FloatIndex> &float_indices, int &ifun,
                   NT &f, NT &dg, NT &dg1, int max_steps,
                   const base::Vector<NT> &search,
                   const base::Vector<NT> &estimate);
  Float threshold_;
  Float max_change_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONJUGATE_GRADIENTS_H */
