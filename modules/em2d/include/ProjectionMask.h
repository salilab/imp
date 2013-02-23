/**
 *  \file ProjectionMask.h
 *  \brief projection masks
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_MASK_H
#define IMPEM2D_PROJECTION_MASK_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/CenteredMat.h"
#include "IMP/atom/Mass.h"
#include "IMP/em/exp.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/em/KernelParameters.h"
#include "IMP/core/XYZR.h"
#include "IMP/Particle.h"
#include "IMP/exception.h"
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
#ifdef SWIG
  ProjectionMask(){}
#endif
public:
#if !defined(DOXYGEN) && !defined(SWIG)
  ProjectionMask(const em::KernelParameters &KP,
            const em::RadiusDependentKernelParameters &params,
            double voxelsize,
            double mass =1.0);

  //! Generates the mask
  /*!
    \param[in] KP Kernel parameteres to employ. See the EM module
    \param[in] params Kernel parameteres associated with radius to employ
    \param[in] mass Mass to give to the mask
  */
  void create(const em::KernelParameters &KP,
                 const em::RadiusDependentKernelParameters &params,
                 double mass = 1.0);
#endif

  //! Adds the values of the mask to a matrix at the given pixel
  /*!
    \param[out] m matrix where the values of the mask are added.
    \param[in] v pixel where to apply the values. Currently used as integer.
    \param[in] weight Weight given to the values of the mask.
  */
  void apply(cv::Mat &m,
             const algebra::Vector2D &v);

  void show(std::ostream &out = std::cout) const;

  ~ProjectionMask();

protected:
  int dim_; // dimension of the mask
  double sq_pixelsize_; // Used to save multiplications
  cv::Mat data_; // actual matrix with the mask
};

IMP_VALUES(ProjectionMask,ProjectionMasks);




// Place a matrix in another
// Very ugly but very fast projecting function

/*!
  \param[in] mask matrix to place in m
  \param[in] m matrix
  \param[in] v Pixel of the matrix dest where the center of m is put.
*/
inline
void do_place(cv::Mat &mask, cv::Mat &m,
                const algebra::Vector2D &v) {

  // v is the vector of coordinates respect to the center of the matrix m
  int vi= algebra::get_rounded(v[0]);
  int vj= algebra::get_rounded(v[1]);

  // Centers for the matrix
  int center[2];
  center[0] = static_cast<int>(0.5*m.rows);
  center[1] = static_cast<int>(0.5*m.cols);

  int start[2], end[2];
  start[0] = -center[0];
  start[1] = -center[1];
  end[0]=m.rows - 1 - center[0];
  end[1]=m.cols - 1 - center[1];

  // Check range: If the vector is outside the matrix, don't do anything.
  if(vi < start[0] || vi > end[0]) return;
  if(vj < start[1] || vj > end[1]) return;


  // Centers for the mask
  int mcenter[2];
  mcenter[0] = static_cast<int>(0.5*mask.rows);
  mcenter[1] = static_cast<int>(0.5*mask.cols);

  int mstart[2], mend[2];
  mstart[0] = -mcenter[0];
  mstart[1] = -mcenter[1];
  mend[0] = mask.rows - 1 - mcenter[0];
  mend[1] = mask.cols - 1 - mcenter[1];

  // Get the admisible range for the mask
  int start_i = std::max(start[0] - vi, mstart[0]);
  int start_j = std::max(start[1] - vj, mstart[1]);
  int end_i = std::min(end[0] - vi, mend[0]);
  int end_j = std::min(end[1] - vj, mend[1]);


  int row = vi+center[0];
  int col = vj+center[1];

  for(int i = start_i; i <= end_i; ++i) {
    int p = i+row;
    for(int j = start_j; j <= end_j; ++j) {
      m.at<double>(p, j+col) += mask.at<double>(i+mcenter[0], j+mcenter[1]);
    }
  }
}














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
    \param[in] mass of the particle
  */
  void create_mask(double radius, double mass);

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
