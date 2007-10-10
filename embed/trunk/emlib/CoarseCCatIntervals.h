#ifndef _COARSECCATINTERVALS_H
#define _COARSECCATINTERVALS_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Javi
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT
  Cross correlation coefficient calculator.
  Stored CCC and derivate values and recompute then every each X steps.
*/

#include "CoarseCC.h"


class CoarseCCatIntervals : public CoarseCC {


  CoarseCCatIntervals(
		      const int &ncd);

public:


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
		 const DensityMap &em_map,
		 SampledDensityMap &model_map,
		 float **cdx,  float **cdy,  float **cdz,
		 float *dvx, float *dvy, float *dvz, 
		 const int &ncd,
		 float **radius,  float **wei,
		 float scalefac,
		 bool lderiv,
		 int &ierr,
		 unsigned long eval_interval );


protected:
  //  unsigned long eval_interval_;                  // the interval size before recalculating the CC score
  unsigned long calls_counter;          // counts the number of time the evalute function was called
  float stored_cc_ ;                        // Stored correlation value
  float *stored_dvx_,*stored_dvy_,*stored_dvz_; // Stored derivative terms

};


#endif //_CoarseCCatIntervals
