/**
 *  \file DensityFillingRestraint.cpp
 *  \brief Score how well a protein is inside its density
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/DensityFillingRestraint.h>
#include <IMP/em/envelope_penetration.h>
#include <IMP/log.h>

IMPEM_BEGIN_NAMESPACE

DensityFillingRestraint::DensityFillingRestraint(
   Particles ps,
   DensityMap *em_map,Float threshold
                                                 ):
    Restraint(IMP::internal::get_model(ps),"Envelope penetration restraint")
{
  IMP_LOG_TERSE("Load envelope penetration with the following input:"<<
          "number of particles:"<<ps.size()<<
           "\n");
  threshold_=threshold;
  target_dens_map_ = em_map;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(core::XYZR::particle_is_instance(ps[i]),
                      "Particle " << ps[i]->get_name()
                      << " is not XYZR"
                      << std::endl);
    }
  }
  add_particles(ps);
  ps_=ps;
  IMP_LOG_TERSE("after adding particles"<<std::endl);
  IMP_LOG_TERSE( "Finish initialization" << std::endl);
}

double
 DensityFillingRestraint::unprotected_evaluate(
                         DerivativeAccumulator *accum) const
{
  double ret_score;
  double covered_percentage=get_percentage_of_voxels_covered_by_particles(
                   target_dens_map_,ps_,core::XYZR(ps_[0]).get_radius(),
                   IMP::algebra::get_identity_transformation_3d(),
                   threshold_);
  ret_score=1.-covered_percentage;
  if (accum != nullptr) {
    IMP_WARN(
   "No derivatives have been assigned to the envelope penetration score\n");
  }
  return ret_score;
}

ParticlesTemp DensityFillingRestraint::get_input_particles() const
{
  ParticlesTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
      pt.push_back(*it);
  }
  return pt;
}

ContainersTemp DensityFillingRestraint::get_input_containers() const {
  ContainersTemp pt;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
  }
  return pt;
}

void DensityFillingRestraint::do_show(std::ostream& out) const
{
  out<<"DensityFillingRestraint"<<std::endl;
}

IMP_LIST_IMPL(
  DensityFillingRestraint, Particle, particle,Particle*, Particles);



IMPEM_END_NAMESPACE
