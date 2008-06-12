#ifndef _COARSECCATINTERVALS_H
#define _COARSECCATINTERVALS_H




#include "EM_config.h"
#include "CoarseCC.h"
#include <vector>

//! Cross correlation coefficient calculator.
/** Store CCC and derivative values and recompute then every X steps.
 */
class EMDLLEXPORT CoarseCCatIntervals : public CoarseCC
{

public:

  CoarseCCatIntervals(const int &ncd);

  CoarseCCatIntervals();




  /** \param[in] em_map         exp EM map
      \param[in] model_map      map for sampled model
      \param[in] cdx, cdy, cdz  coordinates of models
      \param[in] dvx, dvy, dvz  partial derivatives of score with respect
                 to model coordinates
      \param[in] ncd            number of coord
      \param[in] radius         radii of beads
      \param[in] wei            weights of beads
      \param[in] scalefac       scaling factor of EM-score
      \param[in] lderiv         derivatives ON/OFF
      \param[inout] ierr        error
      \param[in] calc           true - recalculate the CC , false - calc only
                 if end of interval

      \param[out] escore         EM score = scalefac * (1-CCC)
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
  int calls_counter;
  //! Stored correlation value
  float stored_cc_ ;
  //! Stored derivative terms
  float *stored_dvx_,*stored_dvy_,*stored_dvz_;
  // true when memory for the terms storing the derivatives has been assigned
  bool dv_memory_allocated;
};


#endif //_CoarseCCatIntervals
