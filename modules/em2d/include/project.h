/**
 *  \file project.h
 *  \brief Generation of projections from models or density maps
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECT_H
#define IMPEM2D_PROJECT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em/DensityMap.h"
#include "IMP/em/MRCReaderWriter.h"
#include "IMP/em/SpiderReaderWriter.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/core/XYZR.h"
#include "IMP/Pointer.h"
#include "IMP.h"
#include <complex>
#include <algorithm> // max,min
#include <fstream>


IMPEM2D_BEGIN_NAMESPACE

// Parameters needed for the core projection routine
class IMPEM2DEXPORT ProjectingParameters {
public:
  double pixel_size, resolution;

  ProjectingParameters() {};
  ProjectingParameters(double ps, double res):
                  pixel_size(ps), resolution(res) {};
  void show(std::ostream &out = std::cout) const {
    out << "ProjectingParameters " << pixel_size
              << " " << resolution << std::endl;};
};
IMP_VALUES(ProjectingParameters,ProjectingParametersList);


// Parameters given as options to the get_projections() functions
class IMPEM2DEXPORT ProjectingOptions: public ProjectingParameters {

  void init_defaults() {
    srw = new SpiderImageReaderWriter();
    save_images = false;
    normalize = true;
    clear_matrix_before_projecting = true;
  }
public:
  ImageReaderWriter *srw; // Writer used to save the images
  bool save_images; // Save images after projeting
  bool normalize; // Normalize the projection after generating it
  bool clear_matrix_before_projecting; // Set the matrix to zeros

  ProjectingOptions() {init_defaults();}
  ProjectingOptions(double ps, double res): ProjectingParameters(ps, res) {
    init_defaults();
  }


  void show(std::ostream &out = std::cout) const {
    out << "ProjectingOptions " << pixel_size
              << " " << resolution << std::endl;};
};
IMP_VALUES(ProjectingOptions,ProjectingOptionsList);


//! Generates projectios from particles
/*!
  \param[in] ps particles to project
  \param[in] vs set of spherical vectors with the directions of projection
  \param[in] rows size of the images
  \param[in] cols
  \param[in] options Options for control the projecting
  \param[in] names names of the images
*/
IMPEM2DEXPORT em2d::Images get_projections(const ParticlesTemp &ps,
        const algebra::SphericalVector3Ds &vs,
        int rows, int cols, const ProjectingOptions &options,
        Strings names=Strings());


//! Generates projectios from particles
/*!
  \param[in] registration_values Registration values with the parameters of
            the projections to generate
  \note See the function get_projections() for the rest of the parameters
*/
IMPEM2DEXPORT em2d::Images get_projections(const ParticlesTemp &ps,
        const RegistrationResults &registration_values,
        int rows, int cols, const ProjectingOptions &options,
        Strings names=Strings());


//! Generates a projection from particles
/*!
  \param[in] ps particles to project
  \param[in] reg Registration value with the parameters of the projection
  \param[in] masks Precomputed masks for projecting the particles. Very useful
             for speeding the projection procedure if they are given.
             If NULL, they are computed
  \param[out] img the projection will be stored here
  \note See the function get_projections() for the rest of the parameters
*/
IMPEM2DEXPORT void get_projection(em2d::Image *img,const ParticlesTemp &ps,
        const RegistrationResult &reg, const ProjectingOptions &options,
        MasksManagerPtr masks=MasksManagerPtr(), String name="");


//! Projects a set of particles. This is the core function that others call
/*!
  \param[in] ps particles to project
  \param[in] R rotation to apply to the particles (respect to the centroid)
  \param[in] translation Translation to apply after rotation
  \param[in] clear_matrix_before If true the matrix is set to zero before
             projecting the particles. You want this 99% of the time, as
             setting to false will add one projection on top of the other
  \note See the function get_projection() for the rest of the parameters
*/
IMPEM2DEXPORT void do_project_particles(const ParticlesTemp &ps,
             cv::Mat &m2,
             const algebra::Rotation3D &R,
             const algebra::Vector3D &translation,
             const ProjectingOptions &options,
             MasksManagerPtr masks);


/*! This function is slightly different than the other ones.
    Only generates evenly distributed projections and determines the size of
    the images that encloses the particles. Should not be used unless this is
    exactly what you want.
*/
IMPEM2DEXPORT Images create_evenly_distributed_projections(
                                             const ParticlesTemp &ps,
                                             unsigned int n,
                                             const ProjectingOptions &options);




//! Project the points contained in Vector3Ds to gen vectors in 2D
/*!
  \param[in] ps the points
  \param[in] R Rotation to apply to the points to project them in the Z axis
  \param[in] translation translation to apply to the points
  \return A set of Vector2D with the projected points
*/
IMPEM2DEXPORT algebra::Vector2Ds do_project_vectors(
            const algebra::Vector3Ds &ps,
            const algebra::Rotation3D &R,
            const  algebra::Vector3D &translation);




//! Project the points contained in Vector3Ds
/*!
  \param[in] ps the points
  \param[in] R Rotation to apply to the points to project them in the Z axis
  \param[in] translation translation to apply to the points
  \param[in] center Center point used for the rotation around it
  \return A set of Vector2D with the projected points
*/
IMPEM2DEXPORT algebra::Vector2Ds do_project_vectors(
              const algebra::Vector3Ds &ps,
              const algebra::Rotation3D &R,
              const algebra::Vector3D &translation,
              const algebra::Vector3D &center);


/*! Get an automatic size for an image that contains the particles
  \param[in] slack is the number of pixels left as border
*/
IMPEM2DEXPORT unsigned int get_enclosing_image_size(const ParticlesTemp &ps,
                                                    double pixel_size,
                                                    unsigned int slack);


//! Generates projectios of a density map using ray casting (real space).
/*!
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
*/
// IMPEM2DEXPORT em::Images get_projections(em::DensityMap *map,
//        const algebra::SphericalVector3Ds &vs,
//        int rows, int cols,
//        const em::ImageReaderWriter *srw,
//        bool project_and_save=false,
//        Strings names=Strings());




//! Projects a given DensityMap into a 2D matrix given the rotation and shift
//! stored in RegistrationResults
/*!
 * \param[in] map A DensityMap of values to project.
 * \param[out] m2  A Matrix2D to store the projection.
 * \param[in] rows number of rows desired for the Matrix2D
 * \param[in] cols number of  columns for the Matrix2D
 * \param[in] reg RegistrationResult containing a rotation and a shift
 * \param[in] equality_tolerance tolerance allowed to consider a value in the
 *            direction as zero.
*/
// IMPEMEXPORT void do_project_map(em::DensityMap *map,
//  algebra::Matrix2D_d &m2,int rows,int cols,
//    const RegistrationResult &reg,double equality_tolerance);




IMPEM2D_END_NAMESPACE


#endif /* IMPEM2D_PROJECT_H */
