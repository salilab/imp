#include "KernelParameters.h"
KernelParameters::Parameters::Parameters(float radii_,float rsigsq_,float timessig_,
		       float sq2pi3_,float inv_rsigsq_, float rnormfac_, float rkdist_) {
  if (radii_ > EPS) { // to prevent calculation for particles with the same radius ( atoms)
    vsig = 1./(sqrt(2.*log(2.))) * radii_; // volume sigma
    vsigsq = vsig * vsig;
    inv_sigsq = rsigsq_ + vsigsq;
    sig = sqrt(inv_sigsq);
    kdist = timessig_ * sig;
    inv_sigsq = 1./inv_sigsq *.5;
    normfac = sq2pi3_ * 1. / powf(sig, 3.);
  }
  else {
    inv_sigsq = inv_rsigsq_;
    normfac = rnormfac_;
    kdist = rkdist_;
  }
}


void KernelParameters::init(float resolution_) {

  timessig=3.; // the number of sigmas used - 3 means that 99% of the density is considered.  
  sq2pi3 = 1. / sqrt(powf(2. * PI, 3));
  // convert resolution to sigma squared
  rsig = 1./(sqrt(2.*log(2.))) * resolution_ / 2.;
  rsigsq = rsig * rsig;
  inv_rsigsq = 1./rsigsq * .5;
  rnormfac = sq2pi3 * 1. / powf(rsig, 3.);
  rkdist   = timessig * rsig;
  lim = exp(-0.5 * powf(timessig - EPS, 2.));

}
