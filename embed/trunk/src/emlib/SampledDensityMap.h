#ifndef _SAMPLEDDENSITYMAP_H
#define _SAMPLEDDENSITYMAP_H


#include <vector>
#include "DensityMap.h"
#include "DensityHeader.h"
#include "def.h"
#include "ParticlesAccessPoint.h"
#include "KernelParameters.h"
#include "exp.h"

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
  }
  
  /** 
      The size of the map is determined by the header and the data is allocated.
  */
  SampledDensityMap(const DensityHeader &header_);

  //! Generatea a sampled density map from the particles.

  /*      INPUT

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





  //!Sampling beads on an EM grid
  /**  
    /param[in]access_p        access point to the particles ( locations, radius, weight)
  */
  void resample(
		const ParticlesAccessPoint &access_p);
		


  void calc_sampling_bounding_box  (const float &x,const float &y,const float &z,
				    const float &kdist,
				    int &iminx,int &iminy, int &iminz,
				    int &imaxx,int &imaxy, int &imaxz) const;
  KernelParameters *get_kernel_params()  { return &kernel_params;}


protected:

  /**
     Initialize needed parameters for sampling
     convention: all parameters that start with r correspond to resolution.
     e.g. rsig: sigma (in Gaussian Fct.) for resolution 
  */

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
  KernelParameters kernel_params;
};


#endif //_SAMPLEDDENSITYMAP_H
