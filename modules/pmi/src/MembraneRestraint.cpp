/**
 * \file IMP/pmi/MembraneRestraint.h
 * \brief membrane restraint
 *
 */
#include "../include/MembraneRestraint.h"
#include <IMP/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/core/XYZ.h>

#include <limits>

IMPPMI_BEGIN_NAMESPACE

MembraneRestraint::MembraneRestraint(Model *m, ParticleIndex z_nuisance,
                                     double thickness, double softness,
                                     double plateau, double linear)
    : ISDRestraint(m, "MembraneRestraint %1%"), z_nuisance_(z_nuisance),
      thickness_(thickness), softness_(softness), plateau_(plateau),
      linear_(linear), max_float_(std::numeric_limits<double>::max()),
      log_max_float_(log(std::numeric_limits<double>::max())) {}

void MembraneRestraint::add_particles_below(ParticleIndexes const &particles) {
  particles_below_.insert(particles_below_.end(), particles.begin(),
                          particles.end());
}
void MembraneRestraint::add_particles_above(ParticleIndexes const &particles) {
  particles_above_.insert(particles_above_.end(), particles.begin(),
                          particles.end());
}
void MembraneRestraint::add_particles_inside(ParticleIndexes const &particles) {
  particles_inside_.insert(particles_inside_.end(), particles.begin(),
                           particles.end());
}

double MembraneRestraint::get_score(double prob) const { return -log(prob); }

double MembraneRestraint::get_probability_above(double z,
                                            double z_slope_center_upper) const {
  double const argvalue((z - z_slope_center_upper) / softness_);
  return (1.0 - plateau_) / (1.0 + exp(-argvalue));
}
double MembraneRestraint::get_score_above(double z,
                                      double z_slope_center_upper) const {
  return get_score(get_probability_above(z, z_slope_center_upper));
}

double MembraneRestraint::get_probability_below(double z,
                                            double z_slope_center_lower) const {
  double const argvalue((z - z_slope_center_lower) / softness_);
  return (1.0 - plateau_) / (1.0 + exp(argvalue));
}
double MembraneRestraint::get_score_below(double z,
                                      double z_slope_center_lower) const {
  return get_score(get_probability_below(z, z_slope_center_lower));
}

double MembraneRestraint::get_score_inside(double z, double z_slope_center_lower,
                                       double z_slope_center_upper) const {
  return get_score(1.0 - get_probability_above(z, z_slope_center_upper)) +
         get_score(1.0 - get_probability_below(z, z_slope_center_lower));
}

double MembraneRestraint::unprotected_evaluate(DerivativeAccumulator *) const {
  IMP::Model *m = get_model();
  double const z_center = isd::Nuisance(m, z_nuisance_).get_nuisance();
  double const z_slope_center_lower = z_center - 0.5 * thickness_;
  double const z_slope_center_upper = z_center + 0.5 * thickness_;
  double sb(0.0);
  for (size_t i(0); i < particles_below_.size(); ++i) {
    sb += get_score_below(core::XYZ(m, particles_below_[i]).get_z(),
                      z_slope_center_lower);
  }
  double sa(0.0);
  for (size_t i(0); i < particles_above_.size(); ++i) {
    sa += get_score_above(core::XYZ(m, particles_above_[i]).get_z(),
                      z_slope_center_upper);
  }
  double si(0.0);
  for (size_t i(0); i < particles_inside_.size(); ++i) {
    si += get_score_inside(core::XYZ(m, particles_inside_[i]).get_z(),
                       z_slope_center_lower, z_slope_center_upper);
  }
  return sb + sa + si;
}

IMP::ModelObjectsTemp MembraneRestraint::do_get_inputs() const {
  ParticleIndexes ps = particles_below_;
  ps.insert(ps.end(), particles_above_.begin(), particles_above_.end());
  ps.insert(ps.end(), particles_inside_.begin(), particles_inside_.end());
  ParticlesTemp ret;
  ret.reserve(ps.size() + 1);
  IMP::Model *m = get_model();
  for (size_t i(0); i < ps.size(); ++i) {
    ret.push_back(m->get_particle(ps[i]));
  }
  ret.push_back(m->get_particle(z_nuisance_));
  return ret;
}

IMPPMI_END_NAMESPACE
