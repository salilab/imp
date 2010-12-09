/**`
 *  \file ProjectionFinder.h
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_FINDER_H
#define IMPEM2D_PROJECTION_FINDER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/align2D.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/Particle.h"
#include <string>

IMPEM2D_BEGIN_NAMESPACE


//! Methods for registration used by the finder
const unsigned int ALIGN2D_NO_PREPROCESSING = 0;
const unsigned int ALIGN2D_PREPROCESSING = 1;
const unsigned int ALIGN2D_WITH_CENTERS = 2;



//! class to perform registration of model projections to images images
class IMPEM2DEXPORT ProjectionFinder
{
public:

  ProjectionFinder() {
    parameters_initialized_=false;
    registration_done_=false;
  }

  //! Initializes the parameters to generate and match projections
  /*!
    \param[in] resolution to employ to generate projections for matching with
              the EM images. Default is the maximum possible, 1.
    \param[in] coarse_registration_method Method for 1st step of projection
              finding, the 2D alignment:
                0 => FFT alignment no preprocessing.
                1 => FFT alignment with preprocessing (Default and recommended).
                2 => FFT alginment and centers of gravity
                        (fast, but only works for low noise)
                during testing.
    \param[in] optimization steps to use by the simplex optimizer. The default
              value is the one found during the benchmark to perform well
    \param[in] simplex_initial_length Initial value to start the simplex search
               The default value is based on the benchmark results
    \param[in] Value of the simplex length stop the search. The smaller, the
               more accurate the finder, but slower
  */
  void initialize(double apix,double resolution =1,
                 int coarse_registration_method = ALIGN2D_PREPROCESSING,
                 bool save_match_images =false,
                  int optimization_steps = 5,
                 double simplex_initial_length =0.1,
                 double simplex_minimum_size =0.01) {
    resolution_ = resolution;
    apix_ = apix;
    coarse_registration_method_ = coarse_registration_method;
    simplex_minimum_size_ = simplex_minimum_size;
    optimization_steps_ = optimization_steps;
    save_match_images_ = save_match_images;
    simplex_initial_length_ = simplex_initial_length;
    masks_manager_ = MasksManagerPtr(new MasksManager);
    masks_manager_->init_kernel(resolution_,apix_);


    fast_optimization_mode_ = false;
    parameters_initialized_=true;
    preprocessing_time_=0.0;
    coarse_registration_time_=0.0;
    fine_registration_time_ =0.0;
  }

  //! Set EM images to use as restraints
  void set_subjects(const em2d::Images &subjects);

  //! Set the projections of the model to use for initial coarse correlation
  void set_projections(const em2d::Images &projections);

  //! Set the particles where the em2D restraint is applied
  void set_model_particles(const ParticlesTemp &ps);

  //! The projections of the model that best match the subject EM images
  //! are saved.
  /*!
    Their name will be: coarse-match-i.spi for coarse registration and
    fine_match-i.spi for full registration.
  */
  void set_save_match_images(bool x) {
    save_match_images_=x;
  }

  bool get_save_match_images() const {
     return save_match_images_;
  }
  //! With this fast mode, only the first number n of best scoring
  //! projections during the coarse search are refined.
  //! Saves vast times of computation time with some loss of accuracy.
  //! Try starting with 1 (risky) or 2, and increased it for get more chances
  //! of finding the best projection
  void set_fast_mode(unsigned int n);

  //! Recover the registration results. Only works if a registration has been
  //! done previously
  RegistrationResults get_registration_results() const;

  //! Coarse registration of all the images using the projections
  //! Based in 2D alignments of the images
    void get_coarse_registration();

  //! Performs complete registration of projections against the images.
  //! This meaning the coarse registration followed by simplex optimization
  void get_complete_registration();

  //! Get the em2d score for a model after the registration performed:
  //! coarse or complete.
  double get_em2d_score() const;

  void show(std::ostream &out) const;

  //! Time employed for preprocessing
  double get_preprocessing_time() const;

  //! Time employed for the coarse registration part
  double get_coarse_registration_time() const;

  //! Time employed for the fine registration part
  double get_fine_registration_time() const;

protected:

  double preprocessing_time_,coarse_registration_time_,fine_registration_time_;
   //! Coarse registration for one subject
  void get_coarse_registrations_for_subject(unsigned int i,
                                            RegistrationResults &coarse_RRs);

  void preprocess_projection(unsigned int j);
  void preprocess_subject(unsigned int i);

  //! Computes the weighted centroid and the FFT of the polar-resampled
  //!  autocorrelation.
  void preprocess_for_fast_coarse_registration(const cv::Mat &m,
                                               algebra::Vector2D &center,
                                               cv::Mat &POLAR_AUTOC);
  //! Main parameters
  em2d::Images subjects_;
  em2d::Images projections_;
  RegistrationResults registration_results_;
  ParticlesTemp model_particles_;
  //! resolution used to generate projections during the fine registration
  double resolution_;
  //! Sampling of the images in Amstrong/pixel
  double apix_;

  bool save_match_images_,
       registration_results_set_,
       particles_set_,
       registration_done_,
       parameters_initialized_,
       fast_optimization_mode_;

  //! Coarse registration method
  unsigned int coarse_registration_method_;
  //! Simplex optimization parameters
  double simplex_minimum_size_,simplex_initial_length_;
  unsigned int optimization_steps_;
  unsigned int number_of_optimized_projections_;
  // FFT of subjects (storing the FFT of projections is not neccessary
  std::vector< cv::Mat > SUBJECTS_;
  // FFT of the autocorrelation resampled images
  std::vector< cv::Mat > SUBJECTS_POLAR_AUTOC_;
  std::vector< cv::Mat > PROJECTIONS_POLAR_AUTOC_;
  algebra::Vector2Ds subjects_cog_;
  algebra::Vector2Ds projections_cog_;
  PolarResamplingParameters polar_params_;
/**
  MasksManager masks_manager_;
**/
  MasksManagerPtr masks_manager_;
};


IMP_VALUES(ProjectionFinder,ProjectionFinders);
IMP_OUTPUT_OPERATOR(ProjectionFinder);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_FINDER_H */
