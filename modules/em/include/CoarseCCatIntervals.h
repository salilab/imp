/**
 *  \file IMP/em/CoarseCCatIntervals.h
 *  \brief Cross correlation coefficient calculator.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_COARSE_CCAT_INTERVALS_H
#define IMPEM_COARSE_CCAT_INTERVALS_H

#include <IMP/em/em_config.h>
#include "CoarseCC.h"
#include <vector>

IMPEM_BEGIN_NAMESPACE

//! Cross correlation coefficient calculator.
/** Store CCC and derivative values and recompute then every X steps.
 */
class IMPEMEXPORT CoarseCCatIntervals : public CoarseCC
{

public:

  CoarseCCatIntervals(const int &ncd);

  CoarseCCatIntervals();

  //! Evaluate the cross-correlation value only every X calls to the function.
  /**
    Evaluates the value of the cross correlation term
    but only at given intervals to save computation time.
    Otherwise, the same value is returned.
    This function has an special behavior, as it
    does not return the true cross correlation coefficient ccc,
    but the value:  scalefac*(1-ccc)

    The reason why is to use this term as part of an scoring
    function that is better the lower the term.
    If you want the cross correlation coefficient,
    use cross_correlation_coefficient() instead.
  \param[in] em_map         exp EM map
  \note correct RMSD and mean MUST be in the header!
  \param[in] model_map      an empty map that will contain the sampled
particles in the access_p
  \param[in] scalefac       scale factor to apply to the value of the cross
correlation term
  \param[in] deriv          input derivatives (only the size is used)
  \param[in] lderiv          if true, the derivatives of the term are computed
  \param[in] eval_interval the number of times the function is going to return
the same value before computing the values again
  \return the value of the cross correlation term: scalefac * (1-CCC)
          and a vector of derivatives
   */
  std::pair<double,algebra::Vector3Ds> evaluate(
                 DensityMap *em_map, SampledDensityMap *model_map,
                 const algebra::Vector3Ds &deriv,
                 float scalefac, bool lderiv,
                 unsigned long eval_interval);

  IMP_OBJECT_METHODS(CoarseCCatIntervals);

protected:
  void allocate_derivatives_array(int ncd);
  //! Number of times the evaluation has been called.
  /** The evaluation is only performed the first time and when calls_counter
      reaches eval_interval. Otherwise the stored_cc_ value is returned */
  int calls_counter_;
  //! Stored correlation value
  float stored_cc_ ;
  //! Stored derivative terms
  double *stored_dvx_,*stored_dvy_,*stored_dvz_;
  // true when memory for the terms storing the derivatives has been assigned
  bool dv_memory_allocated_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_COARSE_CCAT_INTERVALS_H */
