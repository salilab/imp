/**
 *  \file RadiusOfGyrationRestraint.h    \brief radius of gyration restraint.
 *
 *  Restrict max distance between every pair of particle
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/RadiusOfGyrationRestraint.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/core/XYZ.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPMULTIFIT_BEGIN_NAMESPACE

RadiusOfGyrationRestraint::RadiusOfGyrationRestraint(Particles ps,
                                                     Float max_radius):
  Restraint("RadiusOfGyrationRestraint"){
  if (ps.size()==0) return;
  add_particles(ps);
  mdl_=ps[0]->get_model();
  max_radius_=max_radius;
  IMP_NEW(core::HarmonicUpperBound,hub,(max_radius_,0.1));
  //add all pairwise distance restraints
  dps_=new core::DistancePairScore(hub);
}

double
RadiusOfGyrationRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
const {
  double score = 0.;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
    for (ParticleConstIterator it1= it+1;
       it1 != particles_end(); ++it1) {
      score+=dps_->evaluate(ParticlePair(*it,*it1),accum);
    }}
  return score;
}


IMP_LIST_IMPL(RadiusOfGyrationRestraint,
              Particle, particle,Particle*, Particles,
              {
              IMP_INTERNAL_CHECK(get_number_of_particles()==0
                         || obj->get_model()
                         == (*particles_begin())->get_model(),
                "All particles in RadiusOfGyrationRestraint must belong to the "
                "same Model.");
              },{},{});



ParticlesTemp RadiusOfGyrationRestraint::get_input_particles() const
{
  ParticlesTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
      pt.push_back(*it);
  }
  return pt;
}

ContainersTemp RadiusOfGyrationRestraint::get_input_containers() const {
  ContainersTemp pt;
  return pt;
}

void RadiusOfGyrationRestraint::do_show(std::ostream& out) const
{
  out<<"RadiusOfGyrationRestraint"<<std::endl;
}
IMPMULTIFIT_END_NAMESPACE
