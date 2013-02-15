/**
 *  \file RadiusOfGyrationRestraint.h    \brief radius of gyration restraint.
 *
 *  Restrict max distance between every pair of particle
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/container_helpers.h>
#include <IMP/core/XYZ.h>
#include <IMP/multifit/RadiusOfGyrationRestraint.h>

IMPMULTIFIT_BEGIN_NAMESPACE


 //!get radius as function of number of amino acids
/** rad = pow((n_aa times 3.478 ) ,(1./3.))
    Alber et al, Structure 2005 (basically the same as rho=1.3)
    /param[in] len number of amino acids
*/
float get_approximated_radius_of_gyration(int len) {

  //follow shen et al, 2005: 4N pi a^3 / 3 alpha = 4 pi r^3 / 3
  //  N: no of residues, a: bead radius, 3.5 A,
  //alpha: packing ratio (~.64 for proteins)
  //rad = (n_aa * (3.5**3)  / .64 ) **(1./3.)
  //simpler approximation: assume density_prot=1.3
  //  float rad=pow((len*33.54),1./3.);
  //Alber Structure 2005 (basically the same as rho=1.3)
  float rad=get_approximated_radius(len);
  return rad*std::sqrt(3./5);
}

float get_approximated_radius(int len) {

  //follow shen et al, 2005: 4N pi a^3 / 3 alpha = 4 pi r^3 / 3
  //  N: no of residues, a: bead radius, 3.5 A,
  //alpha: packing ratio (~.64 for proteins)
  //rad = (n_aa * (3.5**3)  / .64 ) **(1./3.)
  //simpler approximation: assume density_prot=1.3
  //  float rad=pow((len*33.54),1./3.);
  //Alber Structure 2005 (basically the same as rho=1.3)
  float rad=3.478*pow(len,1./3.);
  return rad;
}
float get_actual_radius_of_gyration(ParticlesTemp ps) {
  algebra::Vector3D cm(0,0,0);
  for (unsigned int i=0; i< ps.size(); ++i) {
    cm+= core::XYZ(ps[i]).get_coordinates();
  }
  cm/=ps.size();
  double ret=0;
  for (unsigned int i=0; i < ps.size(); ++i) {
    double d= get_distance(core::XYZ(ps[i]).get_coordinates(),cm);
    ret+= d;
  }
  return ret/ps.size();
}


RadiusOfGyrationRestraint::RadiusOfGyrationRestraint(Particles ps,
                                                     int num_residues,
                                                     Float scale):
    Restraint(IMP::internal::get_model(ps), "RadiusOfGyrationRestraint"){
  if (ps.size()==0) return;
  add_particles(ps);
  mdl_=ps[0]->get_model();
  predicted_rog_ = get_approximated_radius_of_gyration(num_residues);
  scale_=scale;
  hub_=new core::HarmonicUpperBound(predicted_rog_*scale_,1);
}

double
RadiusOfGyrationRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
const {
  IMP_UNUSED(accum);
  //IMP_USAGE_CHECK(!accum, "No derivatives computed");
  if (accum) {
    IMP_WARN("Can not calcaulte derivatives\n");
  }
  //calculate actual rog
  //todo - do not use get_input_particles function
  float actual_rog=get_actual_radius_of_gyration(get_input_particles());
  IMP_LOG_VERBOSE("actual_rog:"<<actual_rog<<" predicted:"<<predicted_rog_<<
          " scale:"<<predicted_rog_*scale_<<" score: "<<
          hub_->evaluate(actual_rog)<<std::endl);
  return hub_->evaluate(actual_rog);
}


IMP_LIST_IMPL(RadiusOfGyrationRestraint,
              Particle, particle,Particle*, Particles);



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
  out<<get_name()<<" predicted_rog:"<<predicted_rog_<<
    " scale:"<<scale_<<std::endl;
}
IMPMULTIFIT_END_NAMESPACE
