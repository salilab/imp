/**
 *  \file RadiusOfGyrationRestraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/RadiusOfGyrationRestraint.h>
#include <IMP/log.h>
#include <IMP/saxs/utility.h>

IMPSAXS_BEGIN_NAMESPACE

RadiusOfGyrationRestraint::RadiusOfGyrationRestraint(const Particles& particles,
                            const Profile& exp_profile, const double end_q_rg) :
    IMP::Restraint(IMP::internal::get_model(particles),
                   "SAXS Radius of Gyration restraint"), particles_(particles) {
  exp_rg_ = exp_profile.radius_of_gyration(end_q_rg);
}


ParticlesTemp RadiusOfGyrationRestraint::get_input_particles() const
{
  ParticlesTemp pts(particles_.begin(), particles_.end());
  return pts;
}


ContainersTemp RadiusOfGyrationRestraint::get_input_containers() const
{
  return ContainersTemp();
}

//! Calculate the score and the derivatives for particles of the restraint.
/** \param[in] acc If true (not nullptr), partial first derivatives should be
    calculated.
    \return score associated with this restraint for the given state of
            the model.
*/
double RadiusOfGyrationRestraint::unprotected_evaluate(
                                         DerivativeAccumulator *acc) const
{
  IMP_LOG_TERSE( "SAXS RadiusOfGyrationRestraint::evaluate score\n");

  //get centroid
  algebra::Vector3D centroid(0.0, 0.0, 0.0);
  std::vector<algebra::Vector3D> coordinates(particles_.size());
  get_coordinates(particles_, coordinates);
  for (unsigned int i = 0; i < particles_.size(); i++) {
    centroid += coordinates[i];
  }
  centroid /= particles_.size();
  double radg = 0;
  for (unsigned int i = 0; i < particles_.size(); i++) {
    radg += get_squared_distance(coordinates[i], centroid);
  }
  radg /= particles_.size();
  radg = sqrt(radg);

  double score = (radg - exp_rg_)/exp_rg_;  //TODO: improve
  bool calc_deriv = acc? true: false;
  if(!calc_deriv) return score;

  IMP_LOG_TERSE( "SAXS RadiusOfGyrationRestraint::compute derivatives\n");

  const FloatKeys keys = IMP::core::XYZ::get_xyz_keys();

  double factor = 1.0/(particles_.size()*radg);

  for (unsigned int i = 0; i < particles_.size(); i++) {
    IMP::algebra::Vector3D derivative = (coordinates[i]-centroid)*factor;
    for (int j = 0; j < 3; j++) {
        particles_[i]->add_to_derivative(keys[j],derivative[j],*acc);
    }
  }

  IMP_LOG_TERSE( "SAXS RadiusOfGyrationRestraint::done derivatives, score "
          << score << "\n");
  return score;
}

void RadiusOfGyrationRestraint::do_show(std::ostream& out) const
{
   out << "SAXSRadiusOfGyrationRestraint: for " << particles_.size()
       << " particles, target radius of gyration: " << exp_rg_
       << std::endl;
}

IMPSAXS_END_NAMESPACE
