/**
 *  \file BrownianDynamicsTAMD.cpp  \brief Simple Brownian dynamics optimizer
 *        with TAMD adjustments.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/BrownianDynamicsTAMD.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/log.h>
#include <IMP/random.h>
#include <IMP/constants.h>
#include <IMP/atom/constants.h>
#include <IMP/warning_macros.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/units.h>
#include <boost/random/normal_distribution.hpp>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/TAMDCentroid.h>
#include <IMP/atom/TAMDParticle.h>
#include <IMP/Configuration.h>
#include <IMP/algebra/LinearFit.h>
#include <IMP/thread_macros.h>

#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/vector_generators.h>

#include <cmath>
#include <limits>

#define DISABLE_TAMD 1
#ifndef DISABLE_TAMD
#define IF_TAMD(expr) \
  if(TAMDParticle::get_is_setup(m, pi)){expr}
#define IF_TAMD_CENTROID(expr) \
  if(TAMDCentroid::get_is_setup(m, pi)){expr}
#else
#define IF_TAMD(expr)
#define IF_TAMD_CENTROID(expr)
#endif


IMPATOM_BEGIN_NAMESPACE

// // DEBUG:
// static double Ek = 0.0; // kinetic energy (translational)
// static double M = 0.0; // total mass (estimated by 1/D)
// END DEBUG


namespace {

typedef boost::variate_generator<RandomNumberGenerator &,
                                 boost::normal_distribution<double> > RNG;
}


BrownianDynamicsTAMD::BrownianDynamicsTAMD(Model *m, std::string name,
                                   double wave_factor)
  : BrownianDynamics(m, name, wave_factor)
{}

/*
  radius
  if step is xi*radius^2/(3pi E) for some E=1
  then the motion per step should be sqrt(2kT/(pi E))

  T* is
 */

namespace {
  /** get the force dispacement term in the Ermak-Mccammon equation
      for particle pi in model m, with time step dt and ikT=1/kT
  */
  inline algebra::Vector3D get_force_displacement_bdb(double dt, double ikT,
                                                      double diffusion_coefficient,
                                                      algebra::Sphere3D const& xyzr_derivative) {
    algebra::Vector3D nforce(-xyzr_derivative.get_center()); // force acts opposite to derivative
    // unit::Angstrom R(sampler_());
    //    if(TAMDParticle::get_is_setup(m, pi)){
    IF_TAMD(
            TAMDParticle tamd(m, pi);
            // rescale D = [kT] / [m*gamma] ; T = temperature, gamma = friction
            diffusion_coefficient /= tamd.get_friction_scale_factor();
            // // DEBUG: next two lines even out so commented and kept just to
            // //        verify we got it right
            // dd *= tamd.get_temperature_scale_factor();
            // ikT /= tamd.get_temperature_scale_factor();
            );
    return nforce * diffusion_coefficient * dt * ikT;
  }

  // radians at each axis
  inline algebra::Vector3D get_torque_bdb(ParticleIndex pi,
                                          double dt, double ikT,
					  double rotational_diffusion_coefficient,
					  double const* torque_tables[]) {
    algebra::Vector3D torque(torque_tables[0][pi.get_index()],
			     torque_tables[1][pi.get_index()],
			     torque_tables[2][pi.get_index()]);
    double factor= rotational_diffusion_coefficient*dt*ikT;
  return -torque*factor; // minus because torque acts opposite to energy derivative
    // unit::Angstrom R(sampler_());
    // if(TAMDParticle::get_is_setup(m, pi)){
    //   TAMDParticle tamd(m, pi);
    //   dr /= tamd.get_friction_scale_factor();
    //   ikT /= tamd.get_temperature_scale_factor();
    // }
  }


  // returns the std-dev for the random displacement in the Ermak-Mccammon equation
  inline double get_sigma_displacement_bdb(double dtfs,
                                           double diffusion_coefficient) {
    // TAMD: 6.0 since 2.0 for each dof so that l2 magnitude of sigma
    // is 2.0 * D * dtfs per dof
    // Daniel: 6.0 since we are picking radius rather than the coordinates
    double& dd(diffusion_coefficient);
    IF_TAMD(
            // rescale D = [kT] / [m*gamma] ; T = temperature, gamma = friction
            TAMDParticle tamd(m, pi);
            dd *= tamd.get_temperature_scale_factor();
            dd /= tamd.get_friction_scale_factor();
            );
    return sqrt(6.0 * dd * dtfs);
  }

  inline double get_rotational_sigma_bdb(double dtfs,
					 double rotational_diffusion_coefficient) {
    // if(TAMDParticle::get_is_setup(m, pi)){
    //   TAMDParticle tamd(m, pi);
    //   dr /= tamd.get_friction_scale_factor();
  // }
    return sqrt(6.0 * rotational_diffusion_coefficient * dtfs);
}
}

IMP_GCC_DISABLE_WARNING(-Wuninitialized)

namespace {
void check_dX_dbd(algebra::Vector3D &dX, double max_step) {
  for (unsigned int j = 0; j < 3; ++j) {
    // if (std::abs(dX[j]) > max_step) {
    /*std::cerr << "Truncating motion: " << dX[j] << " to " << max_step
      << std::endl;*/
    dX[j] = std::min(dX[j], max_step);
    dX[j] = std::max(dX[j], -max_step);
    //}
  }
}
}

void BrownianDynamicsTAMD::advance_coordinates_1(ParticleIndex pi,
                                                 unsigned int i, double dt,
                                                 double ikT,
                                                 double diffusion_coefficient,
                                                 algebra::Sphere3D const& xyzr_derivative)
{
  core::XYZ xd(get_model(), pi);
  algebra::Vector3D force=
    get_force_displacement_bdb(dt, ikT,
                               diffusion_coefficient,
                               xyzr_derivative);
  algebra::Vector3D dX = (force - get_force(i)) / 2.0;
  check_dX_dbd(dX, get_max_step());
  xd.set_coordinates(xd.get_coordinates() + dX);
}

void BrownianDynamicsTAMD::advance_coordinates_0(unsigned int i, double dtfs,
                                                 double ikT,
                                                 double diffusion_coefficient,
                                                 algebra::Sphere3D const& xyzr_derivative,
                                                 algebra::Sphere3D& xyzr_access){
  IF_TAMD_CENTROID( // centroids do not move independently
                   return; // TODO: a bit wasteful - probably worth it to just let them move
                   // and reset in before_evaluate()
                    );
  //  core::XYZ xd(m, pi);
  double sigma = get_sigma_displacement_bdb(dtfs,
                                            diffusion_coefficient);
  double r = get_sample(sigma);
  algebra::Vector3D random_dX= r * algebra::get_random_vector_on_unit_sphere();
  algebra::Vector3D force_dX=
    get_force_displacement_bdb(dtfs, ikT,
                               diffusion_coefficient,
                               xyzr_derivative);
  if (get_is_srk()) {
    set_force(i, force_dX);
  }
  algebra::Vector3D dX = random_dX + force_dX;
  if (!get_is_srk()) {
    check_dX_dbd(dX, get_max_step());
  }

  // // DEBUG - get kinetic energy
  // algebra::Vector3D v = dX / dtfs;
  // double v2 = v.get_squared_magnitude();
  // Diffusion D(get_model(), pi);
  // // unit::Angstrom R(sampler_());
  // double DD = D.get_diffusion_coefficient();
  // double m = get_kt() / DD;  // for simplicity assume DD = kT / m
  // Ek += 0.5 * m * v2;
  // M += m;
  // std::cout << "Ek for particle " << pi << " is " << 0.5 * m * v2
  //           << std::endl;
  // // DEBUG - end kinetic energy

  xyzr_access._set_center(xyzr_access.get_center() + dX);
}

algebra::Rotation3D
BrownianDynamicsTAMD::compute_rotation_0
(ParticleIndex pi,
 double dtfs, double ikT,
 double rotational_diffusion_coefficient,
 double const* torque_tables[])
{
  //  core::RigidBody rb(get_model(), pi);
  double sigma = get_rotational_sigma_bdb(dtfs, rotational_diffusion_coefficient);
  double angle = get_sample(sigma);
  //  algebra::Transformation3D nt =
  //    rb.get_reference_frame().get_transformation_to();
  algebra::Vector3D axis( algebra::get_random_vector_on_unit_sphere() );
  algebra::Rotation3D rrot( algebra::get_rotation_about_normalized_axis(axis, angle) );
  //nt = nt * rrot;
  algebra::Vector3D torque( get_torque_bdb(pi, dtfs, ikT,
					   rotational_diffusion_coefficient,
                                           torque_tables) );
  double tangle = torque.get_magnitude();
  if (tangle == 0) {
    return rrot;
  } else {
    algebra::Vector3D taxis = torque / tangle;
    algebra::Rotation3D frot = algebra::get_rotation_about_normalized_axis(taxis, tangle);
    //    nt = nt * frot;
    return rrot*frot;
  }
  //  rb.set_reference_frame_lazy(algebra::ReferenceFrame3D(nt));
  //  IMP_LOG_VERBOSE("Advancing rigid body "
  //                 << get_model()->get_particle(pi)->get_name() << " to " << nt
  //                << std::endl);
}

void BrownianDynamicsTAMD::do_advance_chunk(double dtfs, double ikT,
                                     const ParticleIndexes &ps,
                                     unsigned int begin, unsigned int end) {
  IMP_LOG_TERSE("Advancing particles " << begin << " to " << end << std::endl);
  Model* m = get_model();

  // Rotate chunk:
  double const* rotational_diffusion_coefficient_table=
    m->access_attribute_data(RigidBodyDiffusion::get_rotational_diffusion_coefficient_key());
  double const* torque_tables[3];
  for(unsigned int i = 0; i < 3; i++){
    torque_tables[i]=
      core::RigidBody::access_torque_i_data(m, i);
  }
  double* quaternion_tables[4];
  for(unsigned int i = 0; i < 4; i++){
    quaternion_tables[i]=
      core::RigidBody::access_quaternion_i_data(m, i);
  }
  for (unsigned int i = begin; i < end; ++i) {
    ParticleIndex pi= ps[i];
    algebra::Rotation3D rot(1,0,0,0);
    double rdc=
      rotational_diffusion_coefficient_table[pi.get_index()];
    //    if (RigidBodyDiffusion::get_is_setup(m, pi)) {
    if(IMP::internal::FloatAttributeTableTraits::get_is_valid(rdc)){
      // std::cout << "rb" << std::endl;
      rot=compute_rotation_0
        (pi, dtfs, ikT, rdc, torque_tables);
      core::RigidBody(m, pi).apply_rotation_lazy_using_internal_tables
        (rot, quaternion_tables);
    }
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    else  {
      Particle *p = m->get_particle(ps[i]);
      IMP_INTERNAL_CHECK(!core::RigidBody::get_is_setup(p),
                         "A rigid body without rigid body diffusion info"
                         << " was found: " << p->get_name());
      IMP_INTERNAL_CHECK(!core::RigidMember::get_is_setup(p),
                         "A rigid member with diffusion info"
                         << " was found: " << p->get_name());
    }
#endif
  }

  // Translate chunk:
  algebra::Sphere3D* spheres_table=
    m->access_spheres_data();
  algebra::Sphere3D const* sphere_derivatives_table=
    m->access_sphere_derivatives_data();
  double const* diffusion_coefficients_table=
    m->access_attribute_data(Diffusion::get_diffusion_coefficient_key());
  for (unsigned int i = begin; i < end; ++i) {
    ParticleIndex pi= ps[i];
    advance_coordinates_0(i, dtfs, ikT,
                          diffusion_coefficients_table[pi.get_index()],
                          sphere_derivatives_table[pi.get_index()],
                          spheres_table[pi.get_index()]);
  } // for i
} // method def



IMPATOM_END_NAMESPACE
