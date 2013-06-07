/**
 *  \file IMP/gsl/Simplex.h
 *  \brief The simplex algorithm from GSL.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPGSL_SIMPLEX_H
#define IMPGSL_SIMPLEX_H

#include <IMP/gsl/gsl_config.h>

#include "GSLOptimizer.h"

IMPGSL_BEGIN_NAMESPACE

//! A simplex optimizer taken from GSL
/** Simplex works by modifying a simplex in the space of the optimized
    attributes.  The algorithm may not behave well when using score
    states, such as those involved in rigid bodies, which
    significantly change the values of the attributes. Then, again, it
    may work just fine. But be aware that it is not understood.

    The main advantage of Simplex is that it is a local optimizer that
    does not require derivatives.
 */
class IMPGSLEXPORT Simplex : public GSLOptimizer {
  double min_length_;
  double max_length_;

 public:
  Simplex(Model *m = nullptr);

  /** \name Parameters

      The parameters are:

      - minumum_size: the optimization stops when the size of the
      simplex falls below this. The size is defined as the average
      distance from the centroid to the simplex vertices. (Default 0.1)

      - initial_length: the length of the initial sizes of the
      simplex. Make sure that this covers the optimal solution, given
      the starting configuration. (Default 1)

      \note Both quantities are relative to rescaled attribues and so should be
      numbers between 0 and 1.
    @{
  */

  void set_initial_length(double length) {
    IMP_USAGE_CHECK(length > 0 && length <= 4,
                    "The initial length is relative to the rescaled attributes"
                        << " and so should not be much larger than 1.");
    max_length_ = length;
  }

  void set_minimum_size(double d) {
    IMP_USAGE_CHECK(d > 0 && d <= 4,
                    "The minimum size is relative to the rescaled attributes"
                        << " and so should not be much larger than 1 "
                        << "(and must be non-zero).");
    min_length_ = d;
  }
  /** @} */
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(Simplex);
};

IMPGSL_END_NAMESPACE

#endif /* IMPGSL_SIMPLEX_H */
