/**
 *  \file ProjectionFinder.h
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PROJECTION_FINDER_H
#define IMPEM2D_PROJECTION_FINDER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/align2D.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/ProjectionMask.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/Pointer.h"
#include "IMP/Particle.h"
#include <string>

IMPEM2D_BEGIN_NAMESPACE


//! Methods for registration used by the finder
const unsigned int ALIGN2D_NO_PREPROCESSING = 0;
const unsigned int ALIGN2D_PREPROCESSING = 1;
const unsigned int ALIGN2D_WITH_CENTERS = 2;


class IMPEM2DEXPORT Em2DRestraintParameters: public ProjectingParameters {

  void init_defaults() {
    coarse_registration_method = ALIGN2D_PREPROCESSING;
    save_match_images = false;
    optimization_steps = 5;
    simplex_initial_length = 0.1;
    simplex_minimum_size = 0.01;
  }

public:

  // Number of model projections to generate when scoring
  unsigned int n_projections;
  unsigned int  coarse_registration_method;
  bool save_match_images;
  unsigned int  optimization_steps;
  double simplex_initial_length;
  double simplex_minimum_size;

  Em2DRestraintParameters() {init_defaults();};

  Em2DRestraintParameters(double ps, double res, unsigned int n_proj=20):
      ProjectingParameters(ps, res), n_projections(n_proj) {
    init_defaults();
  }

  void show(std::ostream &out = std::cout) const {
    out << "Em2DRestraintParameters: " << std::endl
        << "pixel_size " << pixel_size << " resolution " << resolution
        << " coarse_registration_method " << coarse_registration_method
        << " optimization_steps " << optimization_steps
        << " simplex_initial_length " << simplex_initial_length
        << " simplex_minimum_size " << simplex_minimum_size << std::endl;};
};
IMP_VALUES(Em2DRestraintParameters,Em2DRestraintParametersList);




//! class to perform registration of model projections to images images
class IMPEM2DEXPORT ProjectionFinder: public IMP::base::Object {
public:

  ProjectionFinder(): Object("ProjectionFinder%1%"),
    parameters_setup_(false),registration_done_(false) {};

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
  void setup(ScoreFunction *score_function,
             const Em2DRestraintParameters &params) {

    score_function_= score_function;
    score_function_->set_was_used(true);
    params_ = params;
    masks_manager_ = MasksManagerPtr(new MasksManager);
    masks_manager_->setup_kernel(params.resolution,params.pixel_size);
    fast_optimization_mode_ = false;
    parameters_setup_=true;
    preprocessing_time_=0.0;
    coarse_registration_time_=0.0;
    fine_registration_time_ =0.0;
  }


  //! Set EM images to use as restraints
  void set_subjects(const em2d::Images &subjects);

  //! Set the projections of the model to use for initial coarse correlation
  void set_projections(const em2d::Images &projections);

  //! Set the projections of the model to use for initial coarse correlation
  void set_variance_images(const em2d::Images &variances);


  //! Set the particles where the em2D restraint is applied
  void set_model_particles(const ParticlesTemp &ps);

  //! The projections of the model that best match the subject EM images
  //! are saved.
  /*!
    Their name will be: coarse-match-i.spi for coarse registration and
    fine_match-i.spi for full registration.
  */
  void set_save_match_images(bool x) {
    params_.save_match_images = x;
  }

  bool get_save_match_images() const {
     return params_.save_match_images;
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
  /**
    \note Given that this registration is based on 2D alignment maximizing the
     cross correlation, the a better score is the best correlation
  **/
  void get_coarse_registration();

  //! Performs complete registration of projections against the images.
  //! This meaning the coarse registration followed by simplex optimization
  void get_complete_registration();

  //! Get the em2d score for a model after the registration performed:
  //! coarse or complete.
  double get_global_score() const;

  void show(std::ostream &out) const;

  //! Time employed for preprocessing
  double get_preprocessing_time() const;

  //! Time employed for the coarse registration part
  double get_coarse_registration_time() const;

  //! Time employed for the fine registration part
  double get_fine_registration_time() const;

  unsigned int get_number_of_subjects() const {
    return subjects_.size();
  }

  void set_number_of_class_averages_members(Ints n_members) {
    n_members_ = n_members;
  }

  unsigned int get_number_of_projections() const {
    return projections_.size();
  }

  IMP_OBJECT_METHODS(ProjectionFinder);

protected:

  double preprocessing_time_,coarse_registration_time_,fine_registration_time_;
   //! Coarse registration for one subject
  void get_coarse_registrations_for_subject(unsigned int i,
                                            RegistrationResults &coarse_RRs);

  void do_preprocess_projection(unsigned int j);
  void do_preprocess_subject(unsigned int i);

  //! Computes the weighted centroid and the FFT of the polar-resampled
  //!  autocorrelation.
  void do_preprocess_for_fast_coarse_registration(const cv::Mat &m,
                                               algebra::Vector2D &center,
                                               cv::Mat &POLAR_AUTOC);
  //! Main parameters
  em2d::Images subjects_;
  em2d::Images variances_;
  em2d::Images projections_;
  RegistrationResults registration_results_;
  ParticlesTemp model_particles_;
  Ints n_members_;
  Pointer<ScoreFunction> score_function_;

  bool particles_set_,
       parameters_setup_,
       registration_done_,
       fast_optimization_mode_;


  unsigned int number_of_optimized_projections_;
  // FFT of subjects (storing the FFT of projections is not neccessary
  std::vector< cv::Mat > SUBJECTS_;
  // FFT of the autocorrelation resampled images
  std::vector< cv::Mat > SUBJECTS_POLAR_AUTOC_;
  std::vector< cv::Mat > PROJECTIONS_POLAR_AUTOC_;
  algebra::Vector2Ds subjects_cog_;
  algebra::Vector2Ds projections_cog_;
  PolarResamplingParameters polar_params_;

  Em2DRestraintParameters params_;
/**
  MasksManager masks_manager_;
**/
  MasksManagerPtr masks_manager_;
};

IMP_OBJECTS(ProjectionFinder, ProjectionFinders);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PROJECTION_FINDER_H */
