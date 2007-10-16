#ifndef _SAMPLEDDENSITYMAP_H
#define _SAMPLEDDENSITYMAP_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Friedrich Foerster
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT
  samples a given model on a grid 
*/

#include <vector>
#include "DensityMap.h"
#include "DensityHeader.h"
#include "def.h"
#include "ParticlesAccessPoint.h"

//an advantage of using const double over define is that it limits the use of pointers to effect the data:
//#define PI 3.141592653589793238462643383
//define could be manipulated with a const *int ptr declaration. 

class SampledDensityMap: public DensityMap 

{

public:


  /** 
      Creates a new density map for sampled map. 
      The header of the map is not determined and no data is being allocated
  */

  SampledDensityMap() {
    kernel_param_init = false;
  }
  /** 
      The size of the map is determined by the header and the data is allocated.*/
  SampledDensityMap(const DensityHeader &header_);
  /** 
      Generatea a sampled density map from the particles.

      IMPUT

      access_p : particles
      resolution   : half width the Gaussian
      voxel_size
      sig_cutoff   : Choose what should be the sigma cutoff for accurate sampling. It is used in two functions;
                     (i)  to determine the size of the grid dimensions
                     (ii) to determine the voxels around the coords participating in the sampling procedure.
      


   */
  SampledDensityMap(
		    const ParticlesAccessPoint &access_p,
		    float resolution,
		    float voxel_size,
		    int sig_cuttoff=3);



  /**Sampling beads on an EM grid
    
    INPUT
    access_p        access point to the particles ( locations, radius, weight)
    ierr            error
  */
  void resample(
		const ParticlesAccessPoint &access_p,
		int &ierr);


  // access functions
  float rsig() const { return rsig_;}
  float rsigsq() const {return rsigsq_;}
  float inv_rsigsq() const {return inv_rsigsq_;}
  float rnormfac() const {return rnormfac_;}
  float rkdist() const {return rkdist_;}
  float lim() const {return lim_;}
  float timessig() const {return timessig_;}



  void kernel_setup (
		 const float radii,
		 float &vsig,
		 float &vsigsq,
		 float &inv_sigsq,
		 float &sig,
		 float &kdist,
		 float &normfac) const;
		 
  void calc_sampling_bounding_box  (const float &x,const float &y,const float &z,
				    const float &kdist,
				    int &iminx,int &iminy, int &iminz,
				    int &imaxx,int &imaxy, int &imaxz) const;



protected:

  /**
     Initialize needed parameters for sampling
     convention: all parameters that start with r correspond to resolution.
     e.g. rsig: sigma (in Gaussian Fct.) for resolution 
  */
  void sampling_param_init();




  void calculate_particles_bounding_box(
					const ParticlesAccessPoint &access_p,
					std::vector<float> &lower_bound,
					std::vector<float> &upper_bound);




  void set_header(
		  const std::vector<float> &lower_bound,
		  const std::vector<float> &upper_bound,
		  float resolution,
		  float voxel_size,
		  int sig_offset);



  // bookkeeping functions
  int lower_voxel_shift(const float &loc, const float &kdist,const float &orig,int ndim) const {
    int imin;
    imin = (int)floor(((loc-kdist-orig) / header.Objectpixelsize));
    //bookkeeping
    if (imin < 0) 
      imin = 0;
    if (imin > ndim-1) 
      imin = ndim-1;
    return imin;
  }


  int upper_voxel_shift(const float &loc, const float &kdist,const float &orig,int ndim) const {
    int imax;
    imax = (int)floor(((loc+kdist-orig) / header.Objectpixelsize));
    //bookkeeping
    if (imax < 0) imax = 0;
    if (imax > ndim-1) imax = ndim-1;
    return imax;
  }


  //kernel handeling
  float sq2pi3_;
  bool kernel_param_init;
  float rsig_, rsigsq_,inv_rsigsq_,rnormfac_,rkdist_,lim_,timessig_;

};


#endif //_SAMPLEDDENSITYMAP_H
