#include "CoarseCCatIntervals.h"
	
CoarseCCatIntervals::CoarseCCatIntervals(const int &ncd){

  calls_counter=0;    
  stored_cc_ = 0.0;   

  // Allocate memmory for the derivative terms if not done yet
  stored_dvx_=new float [ncd]; 
  stored_dvy_=new float [ncd]; 
  stored_dvz_=new float [ncd]; 
  
  for(int i=0;i<ncd;i++) {
     stored_dvx_[i] = 0.0;
     stored_dvy_[i] = 0.0;
     stored_dvz_[i] = 0.0;
   }
}

float CoarseCCatIntervals::evaluate(
				     const DensityMap &data, 
				     SampledDensityMap &model_map,
				     const ParticlesAccessPoint &access_p,
				     std::vector<float> &dvx,std::vector<float>&dvy,std::vector<float>&dvz,
				     float scalefac,
				     bool lderiv,
				     int &ierr,		 
				     unsigned long eval_interval ){ // the interval size before recalculating the CC score


  // If the function requires to be evaluated
  if  (calls_counter%eval_interval ==0) {

    // we should sync the derivatives. store_dv holds the EM contirbution of the derivatives. 
    // The base evaluate function adds the EM contiorbution to the existing derivates.

    stored_cc_ = CoarseCC::evaluate(
			data,model_map,
			access_p,
			dvx,dvy,dvz,
			scalefac,
			lderiv);
	
    calls_counter = 1;

    if (lderiv) { // sync the derivatives. Now remove the additional contributions ( currently stored in store_dv from the new values
      for(int i=0;i<access_p.get_size();i++) {
	 stored_dvx_[i] = dvx[i];
	 stored_dvy_[i] = dvy[i];
	 stored_dvz_[i] = dvz[i];
       }
    }
  }
  // If the function does not require to evaluate, return the previously stored CCC and add the derivatives
  // to the general terms
  else {
    for(int i=0;i<access_p.get_size();i++) {
      dvx[i] = stored_dvx_[i];
      dvy[i] = stored_dvy_[i];
      dvz[i] = stored_dvz_[i];
      
    }
    ++calls_counter;
  }
  
  return stored_cc_;
}

