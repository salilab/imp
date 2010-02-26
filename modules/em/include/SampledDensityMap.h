/**
 *  \file SampledDensityMap.h
 *  \brief Sampled density map.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_SAMPLED_DENSITY_MAP_H
#define IMPEM_SAMPLED_DENSITY_MAP_H

#include "config.h"
#include "exp.h"
#include "DensityMap.h"
#include "DensityHeader.h"
#include "def.h"
#include "KernelParameters.h"
#include <vector>
#include "IMP/base_types.h"
#include "IMP/core/XYZR.h"
#include "IMP/atom/Mass.h"
IMPEM_BEGIN_NAMESPACE

// an advantage of using const double over define is that it limits the use
// of pointers to affect the data:
//#define PI 3.141592653589793238462643383
//define could be manipulated with a const *int ptr declaration.

class IMPEMEXPORT SampledDensityMap: public DensityMap
{

public:


  //! Creates a new density map for sampled map.
  /** The header of the map is not determined and no data is being allocated
   */
  SampledDensityMap() {
  }

  //! The size of the map is determined by the header and the data is allocated.
  SampledDensityMap(const DensityHeader &header);

  //! Generatea a sampled density map from the particles.
  /** /param[in] ps     particles with XYZ, radius and weight attributes
      /param[in] resolution   half width the Gaussian
      /param[in] voxel_size
      /param[in] sig_cutoff   Choose what should be the sigma cutoff for
                 accurate sampling. It is used in two functions;
                 (i)  to determine the size of the grid dimensions
                 (ii) to determine the voxels around the coords participating
                      in the sampling procedure.
   */
  SampledDensityMap(const Particles &ps, emreal resolution,
   emreal voxel_size,
   IMP::FloatKey radius_key=IMP::core::XYZR::get_default_radius_key(),
   IMP::FloatKey mass_key=IMP::atom::Mass::get_mass_key(),
   int sig_cuttoff=3);

  //! Resampling beads on an EM grid
  /**
  \note The density of a particle p centered at pl at position gl is:

            Z * exp((-0.5(pl-gl))/sigma)
         ---------------------------------
                  sqrt(2*pi*sigma)

    , such that Z is the weight of the particle and sigma is defined to be
      0.425 the resolution, to follow the 'full width at half maxima'
      criterion. For more details please refer to Topf et al, Structure, 2008.
   */
  virtual void resample();

  //!setting particles in case they were not set by the constructor
  void set_particles(IMP::Particles &ps,
     IMP::FloatKey radius_key = IMP::core::XYZR::get_default_radius_key(),
     IMP::FloatKey mass_key = IMP::atom::Mass::get_mass_key());
  void calc_sampling_bounding_box(const emreal &x,const emreal &y,
                                  const emreal &z,
                                  const emreal &kdist,
                                  int &iminx,int &iminy, int &iminz,
                                  int &imaxx,int &imaxy, int &imaxz) const;

  KernelParameters *get_kernel_params()  { return &kernel_params_;}


  inline const core::XYZRs & get_xyzr_particles() const {return xyzr_;}
 // would go away on§ce we have a XYZRW decorator and the next function as well
  inline const Particles & get_sampled_particles() const {return ps_;}
  inline FloatKey  get_weight_key() const {return weight_key_;}
  inline FloatKey  get_radius_key() const {return radius_key_;}


  IMP_REF_COUNTED_DESTRUCTOR(SampledDensityMap)
protected:
  //! Calculate the parameters of the particles bounding box
  /** \param[in]  ps     particles with XYZ, radius and weight attributes
      \param[out] bb           the particles bounding box
   */
  IMP::algebra::BoundingBoxD<3>
     calculate_particles_bounding_box(const Particles &ps);
  void set_header(const algebra::VectorD<3> &lower_bound,
                  const algebra::VectorD<3> &upper_bound,
                  emreal maxradius, emreal resolution, emreal voxel_size,
                  int sig_offset);

  // bookkeeping functions
  int lower_voxel_shift(const emreal &loc, const emreal &kdist,
                        const emreal &orig, int ndim) const {
    int imin;
    imin = static_cast<int>(std::floor((loc-kdist-orig)
                                        / header_.get_spacing()));
    //bookkeeping
    if (imin < 0)
      imin = 0;
    if (imin > ndim-1)
      imin = ndim-1;
    return imin;
  }


  int upper_voxel_shift(const emreal &loc, const emreal &kdist,
                        const emreal &orig, int ndim) const {
    int imax;
    imax = static_cast<int>(std::floor((loc+kdist-orig)
                                        / header_.get_spacing()));
    //bookkeeping
    if (imax < 0) imax = 0;
    if (imax > ndim-1) imax = ndim-1;
    return imax;
  }
protected:
  //! kernel handling
  KernelParameters kernel_params_;
  Particles ps_;
  core::XYZRs xyzr_; //each voxel decorator would contain X,Y,Z,R
  // std::vector<atom::Mass> weight_;
  FloatKey weight_key_;
  FloatKey radius_key_;
  FloatKey x_key_,y_key_,z_key_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_SAMPLED_DENSITY_MAP_H */
