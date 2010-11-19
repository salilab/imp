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
#include "IMP/em2d/Image.h"
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

//! Generates projectios from particles
/**
  \param[in] ps particles to project
  \param[in] vs set of spherical vectors with the directions of projection
  \param[in] rows size of the images
  \param[in] cols
  \param[in] resolution resolution used to generate the projections
  \param[in] pixelsize in A/pixel
  \param[in] srw Reader writer for the images. Currently uses Spider format
  \param[in] project_and_save if true, save the images
  \param[in] names names of the images
**/
IMPEM2DEXPORT em2d::Images generate_projections(const ParticlesTemp &ps,
        const algebra::SphericalVector3Ds &vs,
        int rows, int cols,
        double resolution, double pixelsize,
        const em2d::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());


//! Generates projectios from particles
/**
  \param[in] registration_values Registration values with the parameters of
            the projections to generate
  \note See the function generate_projections() for the rest of the parameters
**/
IMPEM2DEXPORT em2d::Images generate_projections(const ParticlesTemp &ps,
        RegistrationResults registration_values,
        int rows, int cols,
        double resolution, double pixelsize,
        const em2d::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());


//! Generates a projection from particles
/**
  \param[in] ps particles to project
  \param[in] reg Registration value with the parameters of the projection
  \param[in] masks Precomputed masks for projecting the particles. Very useful
             for speeding the projection procedure if they are given.
             If NULL, they are computed
  \param[out] img the projection will be stored here
  \note See the function generate_projections() for the rest of the parameters
**/
IMPEM2DEXPORT void generate_projection(em2d::Image *img,const ParticlesTemp &ps,
        const RegistrationResult &reg,double resolution,double pixelsize,
        const em2d::ImageReaderWriter<double> &srw,bool save_image=false,
        MasksManager *masks=NULL,String name="");

//! Projects a set of particles. This is the core function that others call
/**
  \param[in] ps particles to project
  \param[in] R rotation to apply to the particles (respect to the centroid)
  \param[in] translation Translation to apply after rotation
  \note See the function generate_projection() for the rest of the parameters
**/
IMPEMEXPORT void project_particles(const ParticlesTemp &ps,
             cv::Mat &m2,
             const algebra::Rotation3D &R,
             const algebra::Vector3D &translation,
             double resolution, double pixelsize,
             MasksManager *masks);


//! Project the points contained in Vector3Ds to gen vectors in 2D
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
 IMPEM2DEXPORT em::Images generate_projections(em::DensityMap *map,
        const algebra::SphericalVector3Ds &vs,
        int rows, int cols,
        const em::ImageReaderWriter<double> &srw,
        bool project_and_save=false,
        Strings names=Strings());




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
 IMPEMEXPORT void project_map(em::DensityMap *map,
  algebra::Matrix2D_d &m2,int rows,int cols,
    const RegistrationResult &reg,double equality_tolerance);




IMPEM2D_END_NAMESPACE


#endif /* IMPEM2D_PROJECT_H */
