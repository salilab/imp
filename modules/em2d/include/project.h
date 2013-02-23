/**
 *  \file project.h
 *  \brief Generation of projections from models or density maps
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECT_H
#define IMPEM2D_PROJECT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/ImageReaderWriter.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/core/XYZR.h"
#include "IMP/Pointer.h"
#include "IMP.h"
#include <complex>
#include <algorithm>
#include <fstream>


IMPEM2D_BEGIN_NAMESPACE

// Parameters needed for the core projection routine
class IMPEM2DEXPORT ProjectingParameters {
public:
  double pixel_size, resolution;

  ProjectingParameters() {};

  /**
   *
   * @param ps Pixel size of the image to generate when projection
   * @param res Resolution used to downsample an atomic model before projecting
   */
  ProjectingParameters(double ps, double res):
                  pixel_size(ps), resolution(res) {};

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
  void show(std::ostream &out = std::cout) const {
    out << "ProjectingParameters " << pixel_size
              << " " << resolution << std::endl;};
};
IMP_VALUES(ProjectingParameters,ProjectingParametersList);


// Parameters given as options to the get_projections() functions.
// This class augments ProjectingParameters adding values that are options
// for the projecting functions, not core parameters.
class IMPEM2DEXPORT ProjectingOptions: public ProjectingParameters {

  /**
   * Sets the default values for the options: Don't save matching images,
   * normalize the projections, and clear the data matrix after projecting
   */
  void init_defaults() {
    save_images = false;
    normalize = true;
    clear_matrix_before_projecting = true;
  }
public:
#ifndef SWIG
  IMP::Pointer<ImageReaderWriter> srw; // Writer used to save the images
#endif
  bool save_images; // Save images after projeting
  bool normalize; // Normalize the projection after generating it
  bool clear_matrix_before_projecting; // Set the matrix to zeros

  /**
   * Constructor. Calls init_defaults()
   */
  ProjectingOptions() {init_defaults();}

  /**
   * The arguments passed to the constructor are the same as the arguments to
   * to the constructor of ProjectingParameters class.
   * @param ps
   * @param res
   */
  ProjectingOptions(double ps, double res): ProjectingParameters(ps, res) {
    init_defaults();
  }

  ProjectingOptions(double ps, double res, ImageReaderWriter *irw):
    ProjectingParameters(ps, res), srw(irw) {
    init_defaults();
  }

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
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
             If nullptr, they are computed
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



IMPEM2D_END_NAMESPACE


#endif /* IMPEM2D_PROJECT_H */
