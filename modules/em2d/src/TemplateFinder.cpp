/**
 *  \file TemplateFinder
 *  \brief
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/TemplateFinder.h"
#include "IMP/em2d/project.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/Image.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/algebra/Sphere3D.h"
#include "IMP/core/CoverRefined.h"
#include "IMP/core/XYZR.h"

IMPEM2D_BEGIN_NAMESPACE

TemplateFinder::TemplateFinder(const ParticlesTemp &ps, Image *img):
                    Object("TemplateFinder%1%"),
                    projections_created_(false),
                    matches_computed_(false),
                    projection_parameters_set_(false) {

  subject_ = new Image();
  set_image(img);
  set_particles(ps);
}

void TemplateFinder::set_particles(const ParticlesTemp &ps) {
  IMP_LOG(IMP::TERSE, "TemplateFinder: Setting model particles" << std::endl);
  my_particles_= ps;
  // Check the particles for coordinates, radius and mass
  for (unsigned int i=0; i<my_particles_.size() ; ++i) {
    IMP_USAGE_CHECK((core::XYZR::particle_is_instance(my_particles_[i]) &&
              atom::Mass::particle_is_instance(my_particles_[i])),
       "Particle " << i
       << " does not have the required attributes" << std::endl);
  }
  particles_set_=true;
}

void TemplateFinder::set_image(Image *img) {
  IMP_USAGE_CHECK(img->get_data().cols > 0 and img->get_data().rows > 0,
                  "TemplateFinder: Image is empty");
  subject_->set_data(img->get_data());
}


void TemplateFinder::create_particles_projections(unsigned int n) {
  if(!particles_set_ || !projection_parameters_set_ ) {
    IMP_THROW("TemplateFinder: Can not create projections, particles or "
              "parameters not set",ValueException);
  }

  // Sphere that encloses_the_particles
  IMP_NEW(Particle, p, (my_particles_[0]->get_model(), "cover Particle") );
  core::XYZs xyzs(my_particles_);

  double diameter = 2 * get_enclosing_sphere(xyzs).get_radius();

  // Add 8 pixels to the size (4 each side) to get some space
  unsigned int rows = static_cast<unsigned int>(diameter/pixel_size_) + 8;
  unsigned int cols = rows;

  IMP_LOG(IMP::VERBOSE, "diameter " << diameter << " pixel_size "<< pixel_size_
      << " rows " << rows <<" cols " << cols<< std::endl);

  projections_information_ = get_evenly_distributed_registration_results(n);
  ProjectingOptions options( pixel_size_, resolution_);
  projections_ = get_projections(my_particles_, projections_information_,
                                 rows, cols, options);
 //  Strings names = create_filenames(n, "template","spi");
// save_images(projections_, names, dummy);
  //
  projections_created_ =  true;
}

void TemplateFinder::do_compute_best_matches(unsigned int n) {
  if(!projections_created_) {
    IMP_THROW("TemplateFinder: Projections are not generated",ValueException);
  }
  RegistrationResults a_lot_of_registrations;
  for (unsigned int i=0; i < projections_.size(); ++i) {
    MatchTemplateResults matches = get_best_template_matches(
                                                    subject_->get_data(),
                                                    projections_[i]->get_data(),
                                                    n);

    // Create a registration result that includes all the information about
    // the possible matches: The rotation used to generate the projection,
    // the translation, and the ccc
    for (unsigned int j=0; j<matches.size(); ++j) {
      double shift_rows = static_cast<double>(matches[j].pair.first);
      double shift_cols = static_cast<double>(matches[j].pair.second);
      algebra::Vector2D shift(shift_rows, shift_cols);
      RegistrationResult reg(projections_information_[i].get_rotation(), shift);
      reg.set_projection_index(i);
      reg.set_ccc(matches[j].cross_correlation);
      a_lot_of_registrations.push_back(reg);
    }
  }

  // Select the best matches among all the matches for this projection.
  best_matches_.resize(n);
  std::partial_sort_copy(a_lot_of_registrations.begin(),
                         a_lot_of_registrations.end(),
                         best_matches_.begin(),
                         best_matches_.end(),
                         HasHigherCCC<RegistrationResult>());
  matches_computed_ = true;
}

RegistrationResults TemplateFinder::get_best_matches() const {
  if(!matches_computed_) {
    IMP_THROW("TemplateFinder: Best matches not computed", ValueException);
  }
  return best_matches_;
}

IMPEM2D_END_NAMESPACE
