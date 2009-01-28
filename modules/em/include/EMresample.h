/**
 *  \file EMresample.h
 *  \brief Resample operation for EM volumes
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_EMRESAMPLE_H
#define IMPEM_EMRESAMPLE_H

#include "config.h"
#include "IMP/math_macros.h"

IMPEM_BEGIN_NAMESPACE

//! Resamples the particles given by the ParticlesAccessPoint into the volume
/**
   \param[in] volume A class containing a 3D volume. Any class with ability
    to reset data and use voxel locations is accepted.
   \param[in] access_p A set of particles

  \note Required properties of the volume class:
 - reset_data()
 - precompute_locations()
 - get_header()
 - locations_ Matrix3D containing the coordinates of the voxels in the volume.
    Each location (k,j,i) must contain the coordinates (z,y,x) of the voxel
    accessed with indexes [0],[1] and [2], respectively. Example:
    locations_(k,j,i)[2] is coordinate x for voxel (k,j,i) of the volume
  - rms_calculated_ flag.
  - normalized_ flag
 */
template<typename T>
IMPEMEXPORT void resample(T& volume, const ParticlesAccessPoint &access_p)
{
  volume.reset_data();
  volume.precompute_locations();

  // TODO - probably the top is not set
  Int  ivox, ivoxx, ivoxy, ivoxz, iminx, imaxx, iminy, imaxy, iminz, imaxz;
  // actual sampling
  Float tmpx, tmpy, tmpz;

  // variables to avoid some multiplications
  Float rsq, tmp;

  // this variable receives the results of the find_params function
  const  KernelParameters::Parameters *params;
  for (Int ii = 0; ii < access_p.get_size(); ii++) {
    // If the kernel parameters for the particles have not been
    // precomputed, do it
    try {
      params = kernel_params_.find_params(access_p.get_r(ii));
    } catch (EMBED_LogicError) {
      kernel_params_.set_params(access_p.get_r(ii));
      params = kernel_params_.find_params(access_p.get_r(ii));
    }
    // compute the box affected by each particle
    calc_sampling_bounding_box(volume,
                               access_p.get_x(ii),
                               access_p.get_y(ii),
                               access_p.get_z(ii),
                               params->get_kdist(),
                               iminx, iminy, iminz, imaxx, imaxy, imaxz);

    for (ivoxz = iminz;ivoxz <= imaxz;ivoxz++) {
      for (ivoxy = iminy;ivoxy <= imaxy;ivoxy++) {
        for (ivoxx = iminx;ivoxx <= imaxx;ivoxx++) {
          tmpz = volume.locations_(ivoxz, ivoxy, ivoxx)[0] - access_p.get_z(ii);
          tmpy = volume.locations_(ivoxz, ivoxy, ivoxx)[1] - access_p.get_y(ii);
          tmpx = volume.locations_(ivoxz, ivoxy, ivoxx)[2] - access_p.get_x(ii);
          rsq = tmpx * tmpx + tmpy * tmpy + tmpz * tmpz;
          tmp = EXP(-rsq * params->get_inv_sigsq());
          // Ensure even sampling within the box
          if (tmp > kernel_params_.get_lim()) {
            data_(ivoxz, ivoxy, ivoxx) += params->get_normfac() *
                                          access_p.get_w(ii) * tmp;
          }
        }
      }
    }
  }
  // The values of dmean, dmin,dmax, and rms have changed
  volume.rms_calculated_ = false;
  volume.normalized_ = false;
}


//! Computes the box of a volume class afected by a particle.
/**
   \param[in] volume volume class
   \param[in] x x-coordinate of the particle
   \param[in] y y-coordinate of the particle
   \param[in] z z-coordinate of the particle
   \param[in] kdist kernel distance
   \param[out] iminx minimum index for the box in the x axis
   \param[out] iminy minimum index for the box in the y axis
   \param[out] iminz minimum index for the box in the z axis
   \param[out] imaxx maximum index for the box in the x axis
   \param[out] imaxy maximum index for the box in the y axis
   \param[out] imaxz maximum index for the box in the z axis
 */
template<typename T>
void calc_sampling_bounding_box(const T& volume, const Float &x, \
  const Float &y, const Float &z, const Float &kdist,
  Int &iminx, Int &iminy, Int &iminz, Int &imaxx, Int &imaxy, Int &imaxz)
const
{
  iminx = lower_voxel_shift(x, kdist, volume.get_header().get_xorigin(),
                            volume.get_header().get_columns());
  iminy = lower_voxel_shift(y, kdist, volume.get_header().get_yorigin(),
                            volume.get_header().get_rows());
  iminz = lower_voxel_shift(z, kdist, volume.get_header().get_zorigin(),
                            volume.get_header().get_slices());
  imaxx = upper_voxel_shift(x, kdist, volume.get_header().get_xorigin(),
                            volume.get_header().get_columns());
  imaxy = upper_voxel_shift(y, kdist, volume.get_header().get_yorigin(),
                            volume.get_header().get_rows());
  imaxz = upper_voxel_shift(z, kdist, volume.get_header().get_zorigin(),
                            volume.get_header().get_slices());
}


//! Lower voxel shift in a volume for a given coordinate,
//! kernel distance, origin and size
Int lower_voxel_shift(const Float &coordinate,
                      const Float &kernel_distance,
                      const Float &origin,
                      Int size) const
{
  Int imin = (Int)std::floor(((coordinate - kernel_distance - origin) /
                              header_.get_object_pixel_size()));
  //bookkeeping
  if (imin < 0) imin = 0;
  if (imin > size - 1) imin = size - 1;
  return imin;
}


//! Upper voxel shift in a volume for a given coordinate,
//! kernel distance, origin and size
int upper_voxel_shift(const Float &coordinate,
                      const Float &kernel_distance,
                      const Float &origin,
                      Int size) const
{
  int imax = (Int)std::floor(((coordinate + kernel_distance - origin) /
                              header_.get_object_pixel_size()));
  //bookkeeping
  if (imax < 0) imax = 0;
  if (imax > size - 1) imax = size - 1;
  return imax;
}


IMPEM_END_NAMESPACE
#endif  /* IMPEM_EMRESAMPLE_H */
