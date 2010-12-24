/**
 *  \file ProjectionMask.h
 *  \brief projection masks
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_MASK_H
#define IMPEM2D_PROJECTION_MASK_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/CenteredMat.h"
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
#include <boost/shared_ptr.hpp>

IMPEM2D_BEGIN_NAMESPACE

class ProjectionMask;
class MasksManager;

typedef boost::shared_ptr<ProjectionMask> ProjectionMaskPtr;
typedef boost::shared_ptr<MasksManager> MasksManagerPtr;


//! Mask that contains the projection of a given particles. This matrices
//! speed up projecting because the only have to be computed once for a model
class IMPEM2DEXPORT ProjectionMask {

public:

  ProjectionMask(const em::KernelParameters &KP,
            const em::RadiusDependentKernelParameters *params,double voxelsize);

  //! Generates the mask
  /*!
    \param[in] KP Kernel parameteres to employ. See the EM module
    \param[in] params Kernel parameteres associated with radius to employ
  */
  void create(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters *params);

  //! Adds the values of the mask to a matrix at the given pixel
  /*!
    \param[out] m matrix where the values of the mask are added.
    \param[in] v pixel where to apply the values. Currently used as integer.
    \param[in] weight Weight given to the values of the mask.
  */
  void apply(cv::Mat &m,
             const algebra::Vector2D &v,double weight);

  void show(std::ostream &out = std::cout) const;

  ~ProjectionMask();

protected:
  int dim_; // dimension of the mask
  double sq_pixelsize_; // Used to save multiplications
  cv::Mat data_; // actual matrix with the mask
};

IMP_VALUES(ProjectionMask,ProjectionMasks);

//! Manage of projection masks
class IMPEM2DEXPORT MasksManager {
public:
  MasksManager() {
    is_setup_ = false;
  };

  MasksManager(double resolution,double pixelsize) {
    setup_kernel(resolution,pixelsize);
  }

  //! Initializes the kernel
  inline void setup_kernel(double resolution,double pixelsize) {
    kernel_params_= em::KernelParameters((float)resolution);
    pixelsize_=pixelsize;
    is_setup_ = true;
  }

  //! Generates all the masks for a set of particles. This is the function
  //! you typically want to use
  void create_masks(const ParticlesTemp &ps);

  //! Creates the adequate mask for a particle of given radius
  /*!
    \param[in] params Kernel parameters for the particle
    \param[in] radius of the particle
  */
  void create_mask(double radius);

  //! Returns the adequate mask for a particle of given radius
  ProjectionMaskPtr find_mask(double radius);

  void show(std::ostream &out = std::cout) const;

  unsigned int get_number_of_masks() const {
    return radii2mask_.size();
  }

  ~MasksManager();


protected:
  // A map to store the masks
  std::map <double,ProjectionMaskPtr > radii2mask_;
  // Kernel Params for the particles
  em::KernelParameters kernel_params_;
  // Pixel size for the masks
  double pixelsize_;
  bool is_setup_;
};


IMP_VALUES(MasksManager,MasksManagers);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_MASK_H */
