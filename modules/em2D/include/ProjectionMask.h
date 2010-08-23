/**
 *  \file ProjectionMask.h
 *  \brief projection masks
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_MASK_H
#define IMPEM2D_PROJECTION_MASK_H

#include "IMP/em2D/em2D_config.h"
#include "IMP/em/Image.h"
#include "IMP/em/exp.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/em/KernelParameters.h"
#include "IMP/core/XYZR.h"
#include "IMP/Particle.h"
#include <complex>

IMPEM2D_BEGIN_NAMESPACE

//! Mask to apply for projecting a particle in 2D (does not take into account
//! The weight asigned to the particle)
class IMPEM2DEXPORT ProjectionMask : public algebra::Matrix2D_d
{

public:

  ProjectionMask(const em::KernelParameters &KP,
            const em::RadiusDependentKernelParameters *params,double voxelsize);
  //! Generates the mask
  /**
    \param[in] KP Kernel parameteres to employ
    \param[in] params Kernel parameteres associated with radius to employ
  **/
  void generate(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters *params);

  //! Adds the values of the mask to a matrix at the given pixel
  /**
    \param[out] m matrix to Add the values of the mask.
    \param[in] v pixel where to apply the values. Currently used as integer.
    \param[in] weight Weight given to the values of the mask.
  **/
  void apply(algebra::Matrix2D_d &m,
             const algebra::Vector2D &v,double weight);

protected:
  int dim_; // dimension of the mask
  double sq_voxsize_; // Used to save multiplications
};


// IMP_VALUES(ProjectionMask,ProjectionMasks);
IMP_OBJECTS(ProjectionMask,ProjectionMasks);

// typedef std::vector<ProjectionMask> ProjectionMasks;

//! Manage of projection masks
class IMPEM2DEXPORT MasksManager {
public:
  MasksManager() {};

  MasksManager(double resolution,double pixelsize) {
    init_kernel(resolution,pixelsize);
  }

  //! Initializes the kernel
  inline void init_kernel(double resolution,double pixelsize) {
    kernel_params_= em::KernelParameters((float)resolution);
    pixelsize_=pixelsize;
  }

  //! Generates all the masks for a set of particles. This is the function
  //! you typically want to use
  void generate_masks(const Particles &ps);

  //! Creates the adequate mask for a particle of given radius
  /**
    \param[in] params Kernel parameters for the particle
    \param[in] radius of the particle
  **/
  void create_mask(double radius);

  //! Returns the adequate mask for a particle of given radius
  ProjectionMask* find_mask(double radius);

protected:
  // A map to store the masks
  std::map <double,ProjectionMask* > radii2mask_;
  // Kernel Params for the particles
  em::KernelParameters kernel_params_;
  // Pixel size for the masks
  double pixelsize_;
};


IMP_VALUES(MasksManager,MasksManagers);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_MASK_H */
