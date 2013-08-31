/**
 * \file DerivativeCalculator \brief A class for computing SAXS derivatives
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/DerivativeCalculator.h>
#include <IMP/saxs/utility.h>

#include <boost/math/special_functions/sinc.hpp>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

DerivativeCalculator::DerivativeCalculator(const Profile* exp_profile) :
  exp_profile_(exp_profile) {}

//tabulates (sin(qr)/qr - cos(qr))/r^2 over the range of qs of the profile
//and up to max_distance for r.
void DerivativeCalculator::compute_sinc_cos(Float pr_resolution,
                                            Float max_distance,
                                            const Profile* model_profile,
                                std::vector<Floats>& output_values) const
{
  //can be input
  unsigned int nr=algebra::get_rounded(max_distance/pr_resolution) + 1;
  output_values.clear();
  unsigned int profile_size = std::min(model_profile->size(),
                                       exp_profile_->size());
  Floats r_size(nr, 0.0);
  output_values.insert(output_values.begin(),
                       profile_size, r_size);
  for(unsigned int iq = 0; iq<profile_size; iq++) {
    Float q = model_profile->get_q(iq);
    for (unsigned int ir=0; ir<nr; ir++) {
      Float r = pr_resolution * ir;
      Float qr = q * r;
      if(fabs(qr) < 1.0e-16) {
        output_values[iq][ir] = 0;
      } else {
        output_values[iq][ir] = (boost::math::sinc_pi(qr) - cos(qr)) /square(r);
      }
    }
  }
}

void DerivativeCalculator::compute_profile_difference(
                                       const Profile* model_profile,
                                       const Float c, const Float offset,
                                       std::vector<double>& profile_diff) const
{
  // compute difference of intensities and squares of weight
  // profile_diff[q] = -2 * c * weight_tilda * (I_exp[q] - c*I_mod[q] + offset)
  unsigned int profile_size = std::min(model_profile->size(),
                                       exp_profile_->size());
  profile_diff.clear();
  profile_diff.resize(profile_size, 0.0);

  for (unsigned int iq=0; iq<profile_size; iq++) {
    Float delta = exp_profile_->get_intensity(iq)
                  - c * model_profile->get_intensity(iq) + offset;
    Float square_error = square(exp_profile_->get_error(iq));
    Float weight_tilda = model_profile->get_weight(iq) / square_error;

    // Exclude the uncertainty originated from limitation of floating number
    if (fabs(delta/exp_profile_->get_intensity(iq)) < IMP_SAXS_DELTA_LIMIT)
      delta = 0.0;
    profile_diff[iq] = -2. * c * weight_tilda * delta;
  }
}

/* compute the model-specific part of the derivative of the chi square e.g.
 * -2 * c * w_tilda(q) * (Iexp(q)-c*Icalc(q) + o) for each q
 */
std::vector<double> DerivativeCalculator::compute_gaussian_effect_size(
        const Profile* model_profile,
        const ProfileFitter<ChiScore>* pf, bool use_offset) const
{
  Float offset = 0.0;
  if(use_offset) offset = pf->compute_offset(model_profile);
  Float c = pf->compute_scale_factor(model_profile);
  std::vector<double> effect_size;
  compute_profile_difference(model_profile, c, offset, effect_size);
  return effect_size;
}

/*
 * precompute sinc_cos function and derivative of distance distribution
 */
DeltaDistributionFunction DerivativeCalculator::precompute_derivative_helpers(
        const Profile* resampled_model_profile,
        const kernel::Particles& particles1, const kernel::Particles& particles2,
        std::vector<Floats>& sinc_cos_values) const
{

  // estimate upper limit on max_distance
  Float max_distance = compute_max_distance(particles1, particles2);
  DeltaDistributionFunction delta_dist
      = DeltaDistributionFunction(particles2, max_distance);
   // (sinc(qr) - cos(qr)) / (r*r)
  compute_sinc_cos(delta_dist.get_bin_size(), max_distance,
          resampled_model_profile, sinc_cos_values);
  return delta_dist;
}

/* compute dI(q)/dx_k for given q and k dI(q)/dx_k = - 2 E^2(q) \sum_l
 * (x_k-x_l)/d_{kl}^2 f_l f_k (sinc(q*d_{kl}) - * cos(q*d_{kl}))
*/
void DerivativeCalculator::compute_intensity_derivatives(
        const DeltaDistributionFunction& delta_dist,
        const std::vector<Floats>& sinc_cos_values,
        unsigned int iq, algebra::Vector3D &dIdx) const
{
      dIdx = algebra::Vector3D(0.0, 0.0, 0.0);
      for (unsigned int ir=0; ir<delta_dist.size(); ir++) {
        // delta_dist.distribution = sum_i [f_k(0) * f_i(0) * (x_k - x_i)]
        // sinc_cos_values = (sinc(qr) - cos_(qr)) / (r*r)
        dIdx +=  delta_dist[ir] * sinc_cos_values[iq][ir];
      }
      // profile_diff = weight_tilda * (I_exp - c*I_model)
      // e_q = exp( -0.23 * q*q )
      Float E_q = std::exp( - exp_profile_-> modulation_function_parameter_
                       * square( exp_profile_->get_q(iq)));
      dIdx = - 2 * E_q * dIdx;

}

/*
compute derivative for each particle in particles1 with respect to particles2
SCORING function : chi
For calculation in real space the quantity Delta(r) is needed to get
derivatives of an atom
Delta(r) = f_iatom * sum_i f_i delta(r-r_{i,iatom}) (x_iatom-x_i)
  // e_q = exp( -0.23 * q*q )
*/
void DerivativeCalculator::compute_chisquare_derivative(
        const Profile* model_profile,
        const kernel::Particles& particles1,
        const kernel::Particles& particles2,
        std::vector<algebra::Vector3D>& derivatives,
        const std::vector<double>& effect_size) const
{

  algebra::Vector3D dIdx, chisquare_derivative;

  std::vector<Floats> sinc_cos_values;
  DeltaDistributionFunction delta_dist =
      precompute_derivative_helpers(model_profile, particles1, particles2,
              sinc_cos_values);

  unsigned int profile_size = std::min(model_profile->size(),
                                       exp_profile_->size());
  derivatives.clear();
  derivatives.resize(particles1.size());
  for (unsigned int iatom=0; iatom<particles1.size(); iatom++) {
    // Compute a delta distribution per atom
    delta_dist.calculate_derivative_distribution(particles1[iatom]);
    chisquare_derivative = algebra::Vector3D(0.0, 0.0, 0.0);

    for (unsigned int iq=0; iq<profile_size; iq++) {
      compute_intensity_derivatives(delta_dist, sinc_cos_values, iq, dIdx);
      chisquare_derivative += dIdx * effect_size[iq];
    }
    derivatives[iatom] = chisquare_derivative;
  }
}

/*
 * loop over all particles and rigid bodies, and call compute_chi_derivative on
 * them
 */
void DerivativeCalculator::compute_all_derivatives(const kernel::Particles& particles,
               const std::vector<kernel::Particles>& rigid_bodies,
               const std::vector<core::RigidBody>& rigid_bodies_decorators,
               const Profile* model_profile,
               const std::vector<double>& effect_size,
               DerivativeAccumulator *acc) const
{
  std::vector<IMP::algebra::VectorD<3> > derivatives;
  const FloatKeys keys = IMP::core::XYZ::get_xyz_keys();

  // 1. compute derivatives for each rigid body
  for(unsigned int i=0; i<rigid_bodies.size(); i++) {
    if(!rigid_bodies_decorators[i].get_coordinates_are_optimized()) continue;
    // contribution from other rigid bodies
    for(unsigned int j=0; j<rigid_bodies.size(); j++) {
      if(i == j) continue;
      compute_chisquare_derivative(model_profile, rigid_bodies[i],
              rigid_bodies[j], derivatives, effect_size);
      for (unsigned int k = 0; k < rigid_bodies[i].size(); k++) {
        rigid_bodies[i][k]->add_to_derivative(keys[0],derivatives[k][0], *acc);
        rigid_bodies[i][k]->add_to_derivative(keys[1],derivatives[k][1], *acc);
        rigid_bodies[i][k]->add_to_derivative(keys[2],derivatives[k][2], *acc);
      }
    }
    if(particles.size() > 0) {
      // contribution from other particles
      compute_chisquare_derivative(model_profile, rigid_bodies[i],
                                          particles, derivatives, effect_size);
      for (unsigned int k = 0; k < rigid_bodies[i].size(); k++) {
        rigid_bodies[i][k]->add_to_derivative(keys[0],derivatives[k][0], *acc);
        rigid_bodies[i][k]->add_to_derivative(keys[1],derivatives[k][1], *acc);
        rigid_bodies[i][k]->add_to_derivative(keys[2],derivatives[k][2], *acc);
      }
    }
  }

  // 2. compute derivatives for other particles
  if(particles.size() > 0) {
    // particles own contribution
    compute_chisquare_derivative(model_profile, particles, derivatives,
            effect_size);
    for (unsigned int i = 0; i < particles.size(); i++) {
      particles[i]->add_to_derivative(keys[0], derivatives[i][0], *acc);
      particles[i]->add_to_derivative(keys[1], derivatives[i][1], *acc);
      particles[i]->add_to_derivative(keys[2], derivatives[i][2], *acc);
    }
    // rigid bodies contribution
    for(unsigned int i=0; i<rigid_bodies.size(); i++) {
      compute_chisquare_derivative(model_profile, particles, rigid_bodies[i],
              derivatives, effect_size);
      for (unsigned int i = 0; i < particles.size(); i++) {
        particles[i]->add_to_derivative(keys[0], derivatives[i][0], *acc);
        particles[i]->add_to_derivative(keys[1], derivatives[i][1], *acc);
        particles[i]->add_to_derivative(keys[2], derivatives[i][2], *acc);
      }
    }
  }
}


IMPSAXS_END_NAMESPACE
