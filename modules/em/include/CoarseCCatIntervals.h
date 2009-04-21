/**
 *  \file CoarseCCatIntervals.h
 *  \brief Cross correlation coefficient calculator.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_COARSE_CCAT_INTERVALS_H
#define IMPEM_COARSE_CCAT_INTERVALS_H

#include "config.h"
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
  /*!
  * Evaluates the value of the cross correlation term
  * but only at given intervals to save computation time.
  * Otherwise, the same value is returned\n
  * This function has an special behavior, as it
  * does not return the true cross correlation coefficient ccc,
  * but the value:\n scalefac*(1-ccc)\n
  * The reason why is to use this term as part of an scoring
  * function that is better the lower the term.
  * If you want the cross correlation coefficient,
  * use cross_correlation_coefficient() instead.
  \param[in] em_map         exp EM map
  \note correct RMSD and mean MUST be in the header!
  \param[in] model_map      an empty map that will contain the sampled
particles in the access_p
  \param[in] access_p       the particles data (location,radius, weight)
  \param[in] dvx            to contain the x partial derivatives
  \param[in] dvy            to contain the y partial derivatives
  \param[in] dvz            to contain the z partial derivatives
  \param[in] scalefac       scale factor to apply to the value of the cross
correlation term
  \param[in] lderiv          if true, the derivatives of the term are computed
  \param[in] eval_interval the number of times the function is going to return
the same value before computing the values again
  \return the value of the cross correlation term: scalefac * (1-CCC)
   */
  float evaluate(DensityMap &em_map, SampledDensityMap &model_map,
                 const ParticlesAccessPoint &access_p,
                 std::vector<float> &dvx, std::vector<float>&dvy,
                 std::vector<float>&dvz, float scalefac, bool lderiv,
                 unsigned long eval_interval);


protected:
  void allocate_derivatives_array(int ncd);
  //! Number of times the evaluation has been called.
  /** The evaluation is only performed the first time and when calls_counter
      reaches eval_interval. Otherwise the stored_cc_ value is returned */
  int calls_counter_;
  //! Stored correlation value
  float stored_cc_ ;
  //! Stored derivative terms
  float *stored_dvx_,*stored_dvy_,*stored_dvz_;
  // true when memory for the terms storing the derivatives has been assigned
  bool dv_memory_allocated_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_COARSE_CCAT_INTERVALS_H */
