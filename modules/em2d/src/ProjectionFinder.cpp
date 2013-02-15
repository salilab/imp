/**
 *  \file ProjectionFinder.cpp
 *  \brief Coarse registration of 2D projections from a 3D volume
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/ProjectionFinder.h"
#include "IMP/em2d/align2D.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em2d/FFToperations.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/Fine2DRegistrationRestraint.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/atom/Mass.h"
#include "IMP/gsl/Simplex.h"
#include "IMP/log.h"
#include "IMP/Pointer.h"
#include "IMP/exception.h"
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <algorithm>
#include <iostream>
#include <limits>

IMPEM2D_BEGIN_NAMESPACE



 void ProjectionFinder::set_subjects(const em2d::Images &subjects) {
  IMP_LOG_TERSE("ProjectionFinder: Setting subject images" << std::endl);
  if(subjects.size()==0) {
    IMP_THROW("Passing empty set of subjects",ValueException);
  }
  if(polar_params_.get_is_setup() == false) {
    polar_params_.setup(subjects[0]->get_data().rows,
                             subjects[0]->get_data().cols);
    polar_params_.set_estimated_number_of_angles(
                    subjects[0]->get_header().get_number_of_columns());
    polar_params_.create_maps_for_resampling();
  }
  boost::timer preprocessing_timer;
  subjects_.resize(subjects.size());
  unsigned int n_subjects = subjects_.size();
  registration_results_.clear();
  registration_results_.resize(n_subjects);
  SUBJECTS_.clear();
  SUBJECTS_.resize(n_subjects);
  SUBJECTS_POLAR_AUTOC_.clear();
  SUBJECTS_POLAR_AUTOC_.resize(n_subjects);
  subjects_cog_.resize(n_subjects);
  for (unsigned int i=0;i<n_subjects;++i) {
    subjects_[i]=subjects[i]; // doest not deep copy
    std::ostringstream oss;
    oss << "Image subject " << i;
    subjects_[i]->set_name(oss.str());
    subjects_[i]->set_was_used(true);
    do_preprocess_subject(i);
  }
  preprocessing_time_ = preprocessing_timer.elapsed();
  IMP_LOG_TERSE("ProjectionFinder: Subject images set" << std::endl);
}

void ProjectionFinder::set_variance_images(const em2d::Images &variances) {
  variances_.resize(variances.size());
  unsigned int n_variances = variances_.size();
  for (unsigned int i=0;i<n_variances;++i) {
    variances_[i] = variances[i]; // doest not deep copy

    std::ostringstream oss;
    oss << "Variance subject " << i;
    variances_[i]->set_name(oss.str());
    variances_[i]->set_was_used(true);
  }

}

void ProjectionFinder::set_projections(const em2d::Images &projections) {
  IMP_LOG_TERSE("ProjectionFinder: Setting projections" << std::endl);

  if(projections.size()==0) {
    IMP_THROW("Passing empty set of projections",ValueException);
  }

  if(polar_params_.get_is_setup() == false) {
    polar_params_.setup(projections[0]->get_data().rows,
                             projections[0]->get_data().cols);
    polar_params_.set_estimated_number_of_angles(
                    projections[0]->get_header().get_number_of_columns());
    polar_params_.create_maps_for_resampling();
  }

  projections_.resize(projections.size());
  unsigned int n_projections = projections_.size();
  PROJECTIONS_POLAR_AUTOC_.clear();
  PROJECTIONS_POLAR_AUTOC_.resize(n_projections);
  projections_cog_.resize(n_projections);
  boost::timer preprocessing_timer;
  for (unsigned int i = 0; i < n_projections; ++i) {
    projections_[i] = projections[i]; // does not copy
    std::ostringstream oss;
    oss << "Projection" << i;
    projections_[i]->set_name(oss.str());
    do_preprocess_projection(i);
  }
  preprocessing_time_ = preprocessing_timer.elapsed();
  IMP_LOG_TERSE("ProjectionFinder: Projections set: "
          << projections_.size() << std::endl);

}


void ProjectionFinder::set_model_particles(const ParticlesTemp &ps) {
  IMP_LOG_TERSE( "ProjectionFinder: Setting model particles" << std::endl);

  if(parameters_setup_==false) {
    IMP_THROW("The ProjectionFinder is not initialized",ValueException);
  }
  model_particles_= ps;
  // Check the particles for coordinates, radius and mass
  for (unsigned int i=0; i<model_particles_.size() ; ++i) {
    IMP_USAGE_CHECK((core::XYZR::particle_is_instance(model_particles_[i]) &&
              atom::Mass::particle_is_instance(model_particles_[i])),
       "Particle " << i
       << " does not have the required attributes" << std::endl);
  }
  masks_manager_->create_masks(model_particles_);
  particles_set_=true;
  IMP_LOG_TERSE("ProjectionFinder: Model particles set" << std::endl);
}


void ProjectionFinder::set_fast_mode(unsigned int n) {
  if(n>projections_.size() || n==0) {
    IMP_THROW("ProjectionFinder fast mode: requested zero projections or "
              "more than available",ValueException);
  }
  number_of_optimized_projections_ = n;
  fast_optimization_mode_ = true;
  IMP_LOG_TERSE("ProjectionFinder: Fast mode, optimizing "
          << n << " results of the coarse registration " << std::endl);
}


void ProjectionFinder::do_preprocess_projection(unsigned int j) {
  IMP_LOG_TERSE("ProjectionFinder: Preprocessing projection " << j
          << std::endl);
  // FFT PREPROCESSING
  if(params_.coarse_registration_method == ALIGN2D_PREPROCESSING) {
    cv::Mat autoc,polar_autoc;
    em2d::get_autocorrelation2d(projections_[j]->get_data(),autoc);
    em2d::do_resample_polar(autoc,polar_autoc,polar_params_);
    get_fft_using_optimal_size(polar_autoc,PROJECTIONS_POLAR_AUTOC_[j]);
  }
  // CENTERS OF GRAVITY AND ROTATIONAL FFT PREPROCESSING
  if(params_.coarse_registration_method == ALIGN2D_WITH_CENTERS) {
    do_preprocess_for_fast_coarse_registration(projections_[j]->get_data(),
                                           projections_cog_[j],
                                           SUBJECTS_POLAR_AUTOC_[j]);
  }
}

void ProjectionFinder::do_preprocess_subject(unsigned int i) {
  IMP_LOG_TERSE("ProjectionFinder: Preprocessing subject " << i
          << std::endl);

  if(params_.coarse_registration_method == ALIGN2D_PREPROCESSING) {
    cv::Mat autoc,polar_autoc;
    get_fft_using_optimal_size(subjects_[i]->get_data(),SUBJECTS_[i]);
    get_autocorrelation2d(subjects_[i]->get_data(),autoc);
    do_resample_polar(autoc,polar_autoc,polar_params_);
    get_fft_using_optimal_size(polar_autoc,SUBJECTS_POLAR_AUTOC_[i]);
  }
  if(params_.coarse_registration_method == ALIGN2D_WITH_CENTERS) {
    do_preprocess_for_fast_coarse_registration(subjects_[i]->get_data(),
                                            subjects_cog_[i],
                                            SUBJECTS_POLAR_AUTOC_[i]);
  }
}


void ProjectionFinder::get_coarse_registrations_for_subject(
             unsigned int i,RegistrationResults &coarse_RRs) {
  IMP_LOG_TERSE("ProjectionFinder: Coarse registration for subject " << i
          << std::endl);
  algebra::Transformation2D best_2d_transformation;
  double max_ccc=0.0;
  unsigned int projection_index = 0;
  coarse_RRs.resize(projections_.size());
  for(unsigned long j=0;j<projections_.size();++j) {
    ResultAlign2D RA;
    // Method without preprocessing
    if(params_.coarse_registration_method == ALIGN2D_NO_PREPROCESSING) {
      RA=get_complete_alignment(subjects_[i]->get_data(),
                          projections_[j]->get_data(),false);
    }
    // Methods with preprocessing and FFT alignment
    if(params_.coarse_registration_method == ALIGN2D_PREPROCESSING) {
      RA=get_complete_alignment_no_preprocessing(subjects_[i]->get_data(),
                                           SUBJECTS_[i],
                                           SUBJECTS_POLAR_AUTOC_[i],
                                           projections_[j]->get_data(),
                                           PROJECTIONS_POLAR_AUTOC_[j]);
    }

    // Method with centers of gravity alignment
    if(params_.coarse_registration_method == ALIGN2D_WITH_CENTERS) {
      RA = get_complete_alignment_with_centers_no_preprocessing(
                                                subjects_cog_[i],
                                                projections_cog_[j],
                                                SUBJECTS_POLAR_AUTOC_[i],
                                                PROJECTIONS_POLAR_AUTOC_[j]);
      // get_complete_alignment_with_centers_no_preprocessing returns a value of
      // Cross correlation from the rotational alignment but not the ccc.
      // compute the ccc here:
      cv::Mat aux;
      get_transformed(projections_[j]->get_data(),aux,RA.first);
      RA.second=get_cross_correlation_coefficient(subjects_[i]->get_data(),aux);
    }

    // Set result
    algebra::Vector2D shift(0.,0.);
    // Get values from the image
    algebra::Vector3D euler = projections_[j]->get_header().get_euler_angles();
    algebra::Rotation3D R = algebra::get_rotation_from_fixed_zyz(euler[0],
                                                                euler[1],
                                                                euler[2]);
    RegistrationResult projection_result(R,shift,j,i);
    projection_result.set_ccc(RA.second);


    // The coarse registration is based on maximizing the
    // cross-correlation-coefficient, but any other score can be calculated
    // at this point.
    IMP_NEW(Image,aux,());
    aux->set_was_used(true);
    get_transformed(projections_[j]->get_data(), aux->get_data(), RA.first);

    if(variances_.size() > 0) {
      score_function_->set_variance_image(variances_[i]);
    }
    double score = score_function_->get_score(subjects_[i], aux);
    projection_result.set_score(score);


    // add the 2D alignment transformation to the registration result
    // for the projection
    projection_result.add_in_plane_transformation(RA.first);
    // and store
    coarse_RRs[j]=projection_result;
    IMP_LOG_VERBOSE(
            "Coarse registration: " << coarse_RRs[j] << std::endl);
    if(RA.second>max_ccc) {
      max_ccc = RA.second;
      best_2d_transformation =  RA.first;
      projection_index = j;
    }
///******/
//    cv::Mat xx;
//    get_transformed(projections_[j]->get_data(),xx,RA.first);
//    std::ostringstream strmm;
//    strmm << "individual-" << i << "-" << j << ".spi";
//    write_matrix(xx,strmm.str());
///******/
  }

  if(params_.save_match_images) {
    IMP_NEW(em2d::Image,match,());

    get_transformed(projections_[projection_index]->get_data(),
                    match->get_data(),
                    best_2d_transformation);
    do_normalize(match,true);
    coarse_RRs[projection_index].set_in_image(match->get_header());
    std::ostringstream strm;

    strm << "coarse_match-";
    strm.fill('0');
    strm.width(4);
    strm << i << ".spi";
    IMP_NEW(em2d::SpiderImageReaderWriter, srw, ());
    match->set_name(strm.str()); ////
    match->set_was_used(true);
    match->write(strm.str(),srw);
  }

}



void ProjectionFinder::get_coarse_registration() {
  IMP_LOG_TERSE("Coarse registration of subjects. " << std::endl);
  if(subjects_.size()==0) {
    IMP_THROW("get_coarse_registration:There are not subject images",
              ValueException);
  }
  if(projections_.size()==0) {
    IMP_THROW("get_coarse_registration:There are not projection images",
              ValueException);
  }

  coarse_registration_time_ = 0;
//  boost::progress_display show_progress(subjects_.size());
  for(unsigned long i=0;i<subjects_.size();++i) {
    RegistrationResults coarse_RRs(projections_.size());
    boost::timer timer_coarse_subject;
    get_coarse_registrations_for_subject(i,coarse_RRs);
    coarse_registration_time_ += timer_coarse_subject.elapsed();

    std::sort(coarse_RRs.begin(),
              coarse_RRs.end(),
              HasHigherCCC<RegistrationResult>());
    // Best result after coarse registration is based on the ccc
    registration_results_[i]=coarse_RRs[0];
    registration_results_[i].set_in_image(subjects_[i]->get_header());
    IMP_LOG_TERSE("Best coarse registration: "
                      << registration_results_[i] << std::endl);
//    ++show_progress;
  }
  registration_done_=true;
}


void ProjectionFinder::get_complete_registration() {
  IMP_LOG_TERSE("Complete registration of subjects" << std::endl);
  if(subjects_.size()==0) {
    IMP_THROW("get_complete_registration:There are not subject images",
              ValueException);
  }
  if(projections_.size()==0) {
    IMP_THROW("get_complete_registration:There are not projection images",
              ValueException);
  }
  if(particles_set_==false) {
    IMP_THROW("get_complete_registration: "
              "Model particles have not been set",ValueException);
  }

  unsigned int rows= subjects_[0]->get_header().get_number_of_rows();
  unsigned int cols= subjects_[0]->get_header().get_number_of_columns();
  IMP_NEW(em2d::Image, match, ());
  match->set_was_used(true);
  match->set_size(rows,cols);
  match->set_name("match image");

  // Set optimizer
  IMP_NEW(Model,scoring_model,());
  IMP_NEW(Fine2DRegistrationRestraint,fine2d,());
  IMP_NEW(IMP::gsl::Simplex,simplex_optimizer,());


  IMP_LOG_TERSE("ProjectionFinder: Setting Fine2DRegistrationRestraint "
          << std::endl);
  ProjectingParameters pp(params_.pixel_size, params_.resolution);
  fine2d->setup(model_particles_,
                pp,
                scoring_model,
                score_function_,
                masks_manager_);

  simplex_optimizer->set_model(scoring_model);
  simplex_optimizer->set_initial_length(params_.simplex_initial_length);
  simplex_optimizer->set_minimum_size(params_.simplex_minimum_size);


//  IMP::SetLogState log_state(fine2d,TERSE);

  // Computation
//   boost::progress_display show_progress(
//                    subjects_.size()*projections_.size());
  coarse_registration_time_ = 0;
  fine_registration_time_ = 0;
  for(unsigned long i=0;i<subjects_.size();++i) {
    RegistrationResults coarse_RRs(projections_.size());

    boost::timer timer_coarse_subject;
    get_coarse_registrations_for_subject(i,coarse_RRs);
    coarse_registration_time_ += timer_coarse_subject.elapsed();
    // The coarse registration scoring is done by cross-correlation
    std::sort(coarse_RRs.begin(),
              coarse_RRs.end(),
              HasHigherCCC<RegistrationResult>());

    unsigned int n_optimized=projections_.size();
    if(fast_optimization_mode_) {
      n_optimized = number_of_optimized_projections_;
    }

    RegistrationResult best_fine_registration;
    best_fine_registration.set_score(std::numeric_limits<double>::max());

    boost::timer timer_fine_subject;
    for (unsigned int k=0;k<n_optimized;++k) {
      // Fine registration of the subject using simplex
      coarse_RRs[k].set_in_image(subjects_[i]->get_header());
      IMP_LOG_TERSE("Setting subject image to "
              "Fine2DRegistrationRestraint "
             "from ProjectionFinder" << std::endl);
      fine2d->set_subject_image(subjects_[i]);
      simplex_optimizer->optimize(params_.optimization_steps);
      // Update the registration parameters
      RegistrationResult fine_registration = fine2d->get_final_registration();

      HasLowerScore<RegistrationResult> has_lower_score;
      if(has_lower_score(fine_registration,best_fine_registration)) {
        best_fine_registration=fine_registration;
      }
    }



    fine_registration_time_ += timer_fine_subject.elapsed();
    best_fine_registration.set_image_index(i);
    registration_results_[i]=best_fine_registration;
    IMP_LOG_TERSE("Fine2DRegistrationRestraint calls: "
                              << fine2d->get_calls() << std::endl);

    IMP_LOG_TERSE("Fine registration: "
                              << registration_results_[i] << std::endl);
    // save if requested
    if(params_.save_match_images) {
      IMP_NEW(em2d::SpiderImageReaderWriter, srw, ());
      srw->set_was_used(true);
      ProjectingOptions options(params_.pixel_size, params_.resolution);
      options.normalize = true;
      get_projection(match,model_particles_,registration_results_[i],
                    options, masks_manager_);
      std::ostringstream strm;
      strm << "fine_match-";
      strm.fill('0');
      strm.width(4);
      strm << i << ".spi";



      registration_results_[i].set_in_image(match->get_header());
      match->set_name(strm.str()); //
      match->write(strm.str(),srw);
    }
    // ++show_progress;
  }
  registration_done_=true;
}


RegistrationResults ProjectionFinder::get_registration_results() const {
  if(registration_done_ == false) {
    IMP_THROW("ProjectionFinder: trying to recover results "
     "before registration",ValueException);
  }
  RegistrationResults Regs(subjects_.size());
  std::copy(registration_results_.begin(),
            registration_results_.end(),
            Regs.begin());
  return Regs;
}

double ProjectionFinder::get_global_score() const {
  if(!registration_done_) {
    IMP_THROW("get_global_score: registration not done ",ValueException);
  }
  return em2d::get_global_score(registration_results_);
}

  //! Time employed for preprocessing
double ProjectionFinder::get_preprocessing_time() const {
  if(subjects_.size()==0 && projections_.size()==0) {
    IMP_THROW("get_preprocessing_time: Preprocessing not done ",
                                                          ValueException);
  }
  return preprocessing_time_;
}

  //! Time employed for the coarse registration part
double ProjectionFinder::get_coarse_registration_time() const {
  if(!registration_done_) {
    IMP_THROW("get_coarse_registration_time: Coarse registration "
              "not done ",ValueException);
  }
  return coarse_registration_time_;
}

double ProjectionFinder::get_fine_registration_time() const {
  if(!registration_done_) {
    IMP_THROW("get_fine_registration_time: fine registration "
              "not done ",ValueException);
  }
  return fine_registration_time_;
}



void ProjectionFinder::do_preprocess_for_fast_coarse_registration(
        const cv::Mat &m,algebra::Vector2D &center,
        cv::Mat &POLAR_AUTOC) {
  // Make the matrix positive to compute the weighted centroid
  double min_value=0;
  cv::minMaxLoc(m,&min_value);
  cv::Mat result;
  result -= min_value;
  center= internal::get_weighted_centroid(m);
  // Center the image in the weighted centroid
  algebra::Transformation2D T((-1)*center);
  get_transformed(m,result,T);
  // Get the autocorrelation in polar coordinates and its FFT
  cv::Mat autoc,polar_autoc;
  get_autocorrelation2d(result,autoc);
  do_resample_polar(autoc,polar_autoc,polar_params_);
  get_fft_using_optimal_size(polar_autoc,POLAR_AUTOC);
}

void ProjectionFinder::show(std::ostream &out) const {
  out << "ProjectionFinder:" << std::endl
  << "Number of projections = " << projections_.size()  << std::endl
  << "Number of subject images = " << subjects_.size() << std::endl
  << "Working parameters: " << std::endl
  << "Resolution: " <<  params_.resolution  << std::endl
  << "A/pixel: " << params_.pixel_size << std::endl
  << "Coarse egistration method: "
        << params_.coarse_registration_method << std::endl
  << "Simplex initial size: " <<  params_.simplex_initial_length << std::endl
  << "Simplex minimun size: " << params_.simplex_minimum_size << std::endl
  << "Simplex maximum optimization steps: "
                              << params_.optimization_steps << std::endl
  << "Save matching images: " << params_.save_match_images << std::endl;
}

IMPEM2D_END_NAMESPACE
