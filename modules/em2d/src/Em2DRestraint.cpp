/**
 *  \file Em2DRestraint.cpp
 *  \brief A restraint to score the fitness of a model to a set of EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/em2d/Em2DRestraint.h"

IMPEM2D_BEGIN_NAMESPACE



 void Em2DRestraint::set_images(const em::Images em_images) {
   finder_.set_subjects(em_images_);
   em_images_ = em_images;
   for (unsigned int i=0;i<em_images_.size();++i) {
     em_images_[i]->set_was_used(true);
   }
 }

 void Em2DRestraint::set_particles(SingletonContainer *particles_container) {
  particles_container_ = particles_container;
  particles_container_->set_was_used(true);
 }

double
Em2DRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
// *** mirar si han cambiado las particulas
// *** si han cambiado:
//    projectar las particulas
//*** poner las proyecciones en el ProjectionFinder
//**** Darle al projection finder.
//**** Devolver el resultado
//


  //=========================> IMPLEMENT ????
  return 0.0;
}

/* We also need to know which particles are used (as some are
   used, but don't create interactions). */
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


//! Get the containers of particles
ContainersTemp Em2DRestraint::get_input_containers() const
{
  // Returns a vector of one container with the particles
  return ContainersTemp(1, particles_container_);
}


void Em2DRestraint::do_show(std::ostream& out) const
{
  out << "container " << *particles_container_ << std::endl;
}


//
//Em2DRestraint::get_version_info() {
//}


IMPEM2D_END_NAMESPACE
