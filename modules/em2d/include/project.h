/**
 *  \file project.h
 *  \brief Generation of projections from models or density maps
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECT_H
#define IMPEM2D_PROJECT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em/Image.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/FFTinterface.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em/DensityMap.h"
#include "IMP/em/MRCReaderWriter.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/algebra/interpolation.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Matrix3D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/core/XYZR.h"
#include <complex>
#include <algorithm> // max,min
#include <fstream>


IMPEM2D_BEGIN_NAMESPACE


//! Generates projectios using particles and precomputed OldMasksManager
 IMPEM2DEXPORT em::Images generate_projections(const ParticlesTemp &ps,
        const algebra::SphericalVector3Ds vs,
        int rows, int cols,
        double resolution, double pixelsize,
        em::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());

//! Generates projectios using particles and precomputed OldMasksManager
 IMPEM2DEXPORT em2d::Images generate_projections(const ParticlesTemp &ps,
        const algebra::SphericalVector3Ds vs,
        int rows, int cols,
        double resolution, double pixelsize,
        em2d::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());

//! Generates projectios using particles precomputed OldMasksManager
 IMPEM2DEXPORT em::Images generate_projections(const ParticlesTemp &ps,
        RegistrationResults registration_values,
        int rows, int cols,
        double resolution, double pixelsize,
        em::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());

//! Generates projectios using particles precomputed OldMasksManager
 IMPEM2DEXPORT em2d::Images generate_projections(const ParticlesTemp &ps,
        RegistrationResults registration_values,
        int rows, int cols,
        double resolution, double pixelsize,
        em2d::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());




//! Generates projectios of a density map using ray casting (real space).
/**
  \param[in] map Map to project
  \param[in] vs set of vectors indicating the directions of projection in
              spherical coordinates
  \param[in] srw a Reader/Writer for the images to produce
  \param[in] rows size of the projection images desired (rows)
  \param[in] cols size of the projection images desired (columns)
  \param[in] project_and_save if true, the images are saved to files instead
             of keeping them in memory. Useful when memory is an issue
  \param[in] project_and_save if true, the images are saved to files instead
             of keeping them in memory. Useful when memory is an issue
  \param[in] names of the projections if saved to disk
**/
 IMPEM2DEXPORT em::Images generate_projections(em::DensityMap &map,
        const algebra::SphericalVector3Ds &vs,
        const int rows, const int cols,
        em::ImageReaderWriter<double> &srw,
        const bool project_and_save=false,
        Strings names=Strings());




IMPEM2DEXPORT void generate_projection(em::Image &img,const ParticlesTemp &ps,
        RegistrationResult &reg,double resolution,double pixelsize,
        em::ImageReaderWriter<double> &srw,bool save_image=false,
        OldMasksManager *masks=NULL,String name="");

IMPEM2DEXPORT void generate_projection(em2d::Image &img,const ParticlesTemp &ps,
        RegistrationResult &reg,double resolution,double pixelsize,
        em2d::ImageReaderWriter<double> &srw,bool save_image=false,
        Masks_Manager *masks=NULL,String name="");

//! Project particles using projection masks
/**
  \param[in] ps the particles.
  \param[in] m2 the matrix where the projection will be stored
  \param[in] Ydim size of the projection (rows)
  \param[in] Xdim size of the projection (cols)
  \param[in] R Rotation to apply to the particles in order to project in the
             Z axis
  \param[in] resolution in Angstrom
  \param[in] pixelsize in A/pixel.
  \param[in] masks if NULL, they are computed. To avoid the computation, they
             must be provided
**/
 IMPEM2DEXPORT void project_particles(const ParticlesTemp &ps,
             algebra::Matrix2D_d& m2,
             algebra::Rotation3D& R,
             algebra::Vector3D &translation,
             double resolution, double pixelsize,
             OldMasksManager *masks=NULL);

IMPEMEXPORT void project_particles(const ParticlesTemp &ps,
             cv::Mat &m2,
             algebra::Rotation3D &R,
             algebra::Vector3D &translation,
             double resolution, double pixelsize,
             Masks_Manager *masks);


//! Project the points contained in Vector3Ds
/**
  \param[in] ps the points
  \param[in] R Rotation to apply to the points to project them in the Z axis
  \param[in] translation translation to apply to the points
  \return A set of Vector2D with the projected points
**/
IMPEM2DEXPORT algebra::Vector2Ds project_vectors(const algebra::Vector3Ds &ps,
            const algebra::Rotation3D &R,const  algebra::Vector3D &translation);

//! Project the points contained in Vector3Ds
/**
  \param[in] ps the points
  \param[in] R Rotation to apply to the points to project them in the Z axis
  \param[in] translation translation to apply to the points
  \param[in] center Center point used for the rotation around it
  \return A set of Vector2D with the projected points
**/
IMPEM2DEXPORT algebra::Vector2Ds project_vectors(const algebra::Vector3Ds &ps,
              const algebra::Rotation3D &R,
              const algebra::Vector3D &translation,
              const algebra::Vector3D &center);






//! Projects a given DensityMap into a 2D matrix given the rotation and shift
//! stored in RegistrationResults
/**
 * \param[in] map A DensityMap of values to project.
 * \param[out] m2  A Matrix2D to store the projection.
 * \param[in] rows number of rows desired for the Matrix2D
 * \param[in] cols number of  columns for the Matrix2D
 * \param[in] reg RegistrationResult containing a rotation and a shift
 * \param[in] equality_tolerance tolerance allowed to consider a value in the
 *            direction as zero.
**/
 IMPEMEXPORT void project_map(em::DensityMap &map,
  algebra::Matrix2D_d &m2,const int rows,const int cols,
    RegistrationResult &reg,const double equality_tolerance);




IMPEM2D_END_NAMESPACE


#endif /* IMPEM2D_PROJECT_H */
