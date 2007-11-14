#ifndef _COARSECCATINTERVALS_H
#define _COARSECCATINTERVALS_H




#include "CoarseCC.h"
#include <vector>

//!  Cross correlation coefficient calculator. Stored CCC and derivate values and recompute then every each X steps.
class CoarseCCatIntervals : public CoarseCC {

public:

  CoarseCCatIntervals(
		      const int &ncd);

  CoarseCCatIntervals();




  /*

    INPUT
    
    em_map         exp EM map
    model_map      map for sampled model
    cdx, cdy, cdz  coordinates of models
    dvx, dvy, dvz  partial derivatives of score with respect to model coordinates
    ncd            number of coord
    radius         radii of beads
    wei            weights of beads
    scalefac       scaling factor of EM-score
    lderiv         derivatives ON/OFF
    ierr           error
    calc           true - recalculate the CC , false - calc only if end of interval

    OUTPUT
    escore         EM score = scalefac * (1-CCC)
  */
  float evaluate(
		 DensityMap &em_map,
		 SampledDensityMap &model_map,
		 const ParticlesAccessPoint &access_p,
		 std::vector<float> &dvx,std::vector<float>&dvy,std::vector<float>&dvz,
		 float scalefac,
		 bool lderiv,
		 unsigned long eval_interval );


protected:
  void allocate_derivatives_array(int ncd);
  int calls_counter;          //Number of times the evaluation has been called. The evaluation is only performed the first time and when calls_counter reaches eval_interval. Otherwise the stored_cc_ value is returned
  float stored_cc_ ;                        // Stored correlation value
  float *stored_dvx_,*stored_dvy_,*stored_dvz_; // Stored derivative terms
  bool dv_memory_allocated; // This variable is true when memory for the terms storing the derivatives has been asigned
};


#endif //_CoarseCCatIntervals
