/**
 *  \file Em2DRestraint.cpp
 *  \brief A restraint to score the fitness of a model to a set of EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/Em2DRestraint.h"
#include "IMP/em2d/RegistrationResult.h"
#include "IMP/em2d/project.h"
#include "IMP/em/SpiderReaderWriter.h"

IMPEM2D_BEGIN_NAMESPACE



 void Em2DRestraint::set_images(const em2d::Images em_images) {
   em_images_ = em_images;
   finder_.set_subjects(em_images_);
   for (unsigned int i=0;i<em_images_.size();++i) {
     em_images_[i]->set_was_used(true);
   }
 }

 void Em2DRestraint::set_particles(SingletonContainer *particles_container) {
  particles_container_ = particles_container;
  particles_container_->set_was_used(true);
  finder_.set_model_particles(particles_container_->get_particles());
 }


void Em2DRestraint::set_fast_mode(unsigned int n) {
  fast_optimization_mode_=true;
  number_of_optimized_projections_=n;
}


double
Em2DRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_NEW(Model,model,());
  model=get_model();
  // Project the model
  RegistrationResults evenly_regs=get_evenly_distributed_registration_results(
                                  n_projections_for_coarse_registration_);
  unsigned int rows =  em_images_[0]->get_header().get_number_of_rows();
  unsigned int cols =  em_images_[0]->get_header().get_number_of_columns();
  em2d::SpiderImageReaderWriter<double> srw;
  em2d::Images projections=get_projections(
          particles_container_->get_particles(),evenly_regs,rows,cols,
                                resolution_,apix_,srw);
  finder_.set_projections(projections);
  if(fast_optimization_mode_) {
    finder_.set_fast_mode(number_of_optimized_projections_);
  }
  finder_.get_complete_registration();
  return finder_.get_em2d_score();
}

// We also need to know which particles are used (as some are
//   used, but don't create interactions).
ParticlesTemp Em2DRestraint::get_input_particles() const
{
  ParticlesTemp ret;
  for (SingletonContainer::ParticleIterator it
       = particles_container_->particles_begin();
       it != particles_container_->particles_end(); ++it) {
    ret.push_back(*it);
  }
  return ret;
}


ContainersTemp Em2DRestraint::get_input_containers() const
{
  // Returns a vector of one container with the particles
  return ContainersTemp(1, particles_container_);
}


void Em2DRestraint::do_show(std::ostream& out) const
{
  out << "container " << *particles_container_ << std::endl;
}



IMPEM2D_END_NAMESPACE
