/**
 *  \file ConjugateGradients.h    \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_CONJUGATE_GRADIENTS_H
#define IMPCORE_CONJUGATE_GRADIENTS_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Optimizer.h>

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
class IMPCOREEXPORT ConjugateGradients : public Optimizer
{
public:
  ConjugateGradients();
  virtual ~ConjugateGradients();

  IMP_OPTIMIZER(internal::version_info)

  //! Set the threshold for the minimum gradient
  void set_threshold(Float t){ threshold_=t;}

  //! Limit how far anything can change each time step
  void set_max_change(Float t) { max_change_ = t; }
private:

  typedef double NT;

  // Handle optimization failing badly
  void failure();

  NT get_score(std::vector<FloatIndex> float_indices,
               std::vector<NT> &x, std::vector<NT> &dscore);
  bool line_search(std::vector<NT> &x, std::vector<NT> &dx,
                   NT &alpha, const std::vector<FloatIndex> &float_indices,
                   int &ifun, NT &f, NT &dg, NT &dg1,
                   int max_steps, const std::vector<NT> &search,
                   const std::vector<NT> &estimate);
  Float threshold_;
  Float max_change_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CONJUGATE_GRADIENTS_H */
