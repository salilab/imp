/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/BrownianDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/base/log.h>
#include <IMP/base/random.h>
#include <IMP/constants.h>
#include <IMP/atom/constants.h>
#include <IMP/base/warning_macros.h>
#include <IMP/kernel/internal/constants.h>
#include <IMP/kernel/internal/units.h>
#include <boost/random/normal_distribution.hpp>
#include <IMP/atom/Diffusion.h>
#include <IMP/Configuration.h>
#include <IMP/algebra/LinearFit.h>
#include <IMP/base/thread_macros.h>

#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/vector_generators.h>

#include <cmath>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

// // DEBUG:
// static double Ek = 0.0; // kinetic energy (translational)
// static double M = 0.0; // total mass (estimated by 1/D)
// END DEBUG

namespace {

typedef boost::variate_generator<base::RandomNumberGenerator &,
                                 boost::normal_distribution<double> > RNG;
}
BrownianDynamics::BrownianDynamics(kernel::Model *m, std::string name,
                                   double wave_factor)
    : Simulator(m, name, wave_factor),  // nd_(0,1),
      // sampler_(base::random_number_generator, nd_),
      max_step_(std::numeric_limits<double>::max()),
      srk_(false) {}

/*
  radius
  if step is xi*radius^2/(3pi E) for some E=1
  then the motion per step should be sqrt(2kT/(pi E))

  T* is
 */

bool BrownianDynamics::get_is_simulation_particle(kernel::ParticleIndex pi)
    const {
  return (Diffusion::get_is_setup(get_model(), pi) &&
          IMP::core::XYZ(get_model(), pi).get_coordinates_are_optimized());
}

namespace {
/** get the force dispacement term in the Ermak-Mccammon equation
    for coordinate i of  particle pi in model m, with time step dt and ikT=1/kT
*/
inline double get_force_displacement(kernel::Model *m, kernel::ParticleIndex pi,
                        unsigned int i, double dt, double ikT) {
  Diffusion d(m, pi);
  double nforce(-d.get_derivative(i));
  // unit::Angstrom R(sampler_());
  double dd = d.get_diffusion_coefficient();
  double force_term(nforce * dd * dt * ikT);
  /*if (force_term > unit::Angstrom(.5)) {
    std::cout << "Forces on " << _1->get_name() << " are "
    << force << " and " << nforce
    << " and " << force_term <<
    " vs " << dX[j] << ", " << sigma << std::endl;
    }
  std::cout << "Force " << i << " is " << force_term
            << "= " << nforce << "*" << dd << "*" << dt << "*" << ikT
            << std::endl;*/
  return force_term;
}
// radians
inline double get_torque(kernel::Model *m, kernel::ParticleIndex p,
                         unsigned int i, double dt, double ikT) {
  RigidBodyDiffusion d(m, p);
  core::RigidBody rb(m, p);

  double cforce(rb.get_torque()[i]);
  // unit::Angstrom R(sampler_());
  double dr = d.get_rotational_diffusion_coefficient();
  double force_term = dr * cforce * dt * ikT;
  /*if (force_term > unit::Angstrom(.5)) {
    std::cout << "Forces on " << _1->get_name() << " are "
    << force << " and " << nforce
    << " and " << force_term <<
    " vs " << dX[j] << ", " << sigma << std::endl;
    }*/
  return -force_term;
}

  // returns the std-dev for the random displacement in the Ermak-Mccammon equation
inline double get_sigma_displacement(kernel::Model *m, kernel::ParticleIndex p,
                        double dtfs) {
  // 6.0 is 2.0 for each dof (Barak)
  // 6.0 since we are picking radius rather than the coordinates (Daniel)
  double dd = Diffusion(m, p).get_diffusion_coefficient();
  return sqrt(6.0 * dd * dtfs);
}
inline double get_rotational_sigma(kernel::Model *m, kernel::ParticleIndex p,
                                   double dtfs) {
  double dr = RigidBodyDiffusion(m, p).get_rotational_diffusion_coefficient();
  return sqrt(6.0 * dr * dtfs);
}
}

void BrownianDynamics::setup(const kernel::ParticleIndexes &ips) {
  IMP_IF_LOG(TERSE) {
    kernel::ParticlesTemp ps = IMP::internal::get_particle(get_model(), ips);
    double dtfs = get_maximum_time_step();
    double ikT = 1.0 / get_kt();
    double ms = 0;
    double mf = 0;
    get_scoring_function()->evaluate(true);
    for (unsigned int i = 0; i < ps.size(); ++i) {
      double c = get_sigma_displacement(get_model(), ips[i], dtfs);
      ms = std::max(ms, c);
      for (unsigned int j = 0; j < 3; ++j) {
        double f = get_force_displacement(get_model(), ips[i], j, dtfs, ikT);
        mf = std::max(mf, f);
      }
    }
    IMP_LOG_TERSE("Maximum sigma is " << ms << std::endl);
    IMP_LOG_TERSE("Maximum force is " << mf << std::endl);
  }
  forces_.resize(ips.size());
}
IMP_GCC_DISABLE_WARNING(-Wuninitialized)

namespace {
void check_dX(algebra::Vector3D &dX, double max_step) {
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

void BrownianDynamics::advance_coordinates_1(kernel::ParticleIndex pi,
                                             unsigned int i, double dt,
                                             double ikT) {
  Diffusion d(get_model(), pi);
  core::XYZ xd(get_model(), pi);
  algebra::Vector3D force(get_force_displacement(get_model(), pi, 0, dt, ikT),
                          get_force_displacement(get_model(), pi, 1, dt, ikT),
                          get_force_displacement(get_model(), pi, 2, dt, ikT));
  algebra::Vector3D dX = (force - forces_[i]) / 2.0;
  check_dX(dX, max_step_);
  xd.set_coordinates(xd.get_coordinates() + dX);
}

void BrownianDynamics::advance_coordinates_0(kernel::ParticleIndex pi,
                                             unsigned int i, double dtfs,
                                             double ikT) {
  core::XYZ xd(get_model(), pi);
  double sigma = get_sigma_displacement(get_model(), pi, dtfs);
  boost::normal_distribution<double> nd(0, sigma);
  RNG sampler(base::random_number_generator, nd);
  double r = sampler();
  algebra::Vector3D random_dX = r * algebra::get_random_vector_on_unit_sphere();
  algebra::Vector3D force_dX
    (get_force_displacement(get_model(), pi, 0, dtfs, ikT),
     get_force_displacement(get_model(), pi, 1, dtfs, ikT),
     get_force_displacement(get_model(), pi, 2, dtfs, ikT));
  if (srk_) {
    forces_[i] = force_dX;
  }
  algebra::Vector3D dX = random_dX + force_dX;
  if (!srk_) {
    check_dX(dX, max_step_);
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

  xd.set_coordinates(xd.get_coordinates() + dX);
}

void BrownianDynamics::advance_orientation_0(kernel::ParticleIndex pi,
                                             double dtfs, double ikT) {
  core::RigidBody rb(get_model(), pi);
  double sigma = get_rotational_sigma(get_model(), pi, dtfs);
  boost::normal_distribution<double> nd(0, sigma);
  RNG sampler(base::random_number_generator, nd);
  double angle = sampler();
  algebra::Transformation3D nt =
      rb.get_reference_frame().get_transformation_to();
  algebra::Vector3D axis = algebra::get_random_vector_on_unit_sphere();
  algebra::Rotation3D rrot = algebra::get_rotation_about_axis(axis, angle);
  nt = nt * rrot;
  algebra::Vector3D torque(get_torque(get_model(), pi, 0, dtfs, ikT),
                           get_torque(get_model(), pi, 1, dtfs, ikT),
                           get_torque(get_model(), pi, 2, dtfs, ikT));
  double tangle = torque.get_magnitude();
  if (tangle > 0) {
    algebra::Vector3D taxis = torque / tangle;
    algebra::Rotation3D frot = algebra::get_rotation_about_axis(taxis, tangle);
    nt = nt * frot;
  }
  rb.set_reference_frame_lazy(algebra::ReferenceFrame3D(nt));
  IMP_LOG_VERBOSE("Advancing rigid body "
                  << get_model()->get_particle(pi)->get_name() << " to " << nt
                  << std::endl);
}

void BrownianDynamics::advance_chunk(double dtfs, double ikT,
                                     const kernel::ParticleIndexes &ps,
                                     unsigned int begin, unsigned int end) {
  IMP_LOG_TERSE("Advancing particles " << begin << " to " << end << std::endl);
  for (unsigned int i = begin; i < end; ++i) {
    if (RigidBodyDiffusion::get_is_setup(get_model(), ps[i])) {
      // std::cout << "rb" << std::endl;
      advance_orientation_0(ps[i], dtfs, ikT);
    } else {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
      kernel::Particle *p = get_model()->get_particle(ps[i]);
      IMP_INTERNAL_CHECK(!core::RigidBody::get_is_setup(p),
                         "A rigid body without rigid body diffusion info"
                             << " was found: " << p->get_name());
      IMP_INTERNAL_CHECK(!core::RigidMember::get_is_setup(p),
                         "A rigid member with diffusion info"
                             << " was found: " << p->get_name());
#endif
    }
    advance_coordinates_0(ps[i], i, dtfs, ikT);
  }
}

/**
    dx= D/2kT*(F(x0)+F(x0+D/kTF(x0)dt +R)dt +R
 */
double BrownianDynamics::do_step(const kernel::ParticleIndexes &ps, double dt) {
  double dtfs(dt);
  double ikT = 1.0 / get_kt();
  get_scoring_function()->evaluate(true);
  //  Ek = 0.0; // DEBUG: monitor kinetic energy
  //  M = 0.0; // DEBUG: monitor kinetic energy
  const unsigned int chunk_size = 20;
  for (unsigned int b = 0; b < ps.size(); b += chunk_size) {
    IMP_TASK_SHARED(
        (dtfs, ikT, b), (ps),
        advance_chunk(dtfs, ikT, ps, b,
                      std::min<unsigned int>(b + chunk_size, ps.size()));
        , "brownian");
  }
  IMP_OMP_PRAGMA(taskwait)
  IMP_OMP_PRAGMA(flush)

    // DEBUG: monitor kinetic energy
    //  std::cout << "Kinetic energy (translational) for step is " << Ek
    //            << " , per mass: " << Ek / M << " kT = " << get_kt()
    //            << std::endl;

  if (srk_) {
    get_scoring_function()->evaluate(true);
    for (unsigned int i = 0; i < ps.size(); ++i) {
      advance_coordinates_1(ps[i], i, dtfs, ikT);
    }
  }
  return dt;
}

namespace {
template <class It>
bool is_constant(It b, It e) {
  // smooth some
  for (It c = b + 1; c < e - 1; ++c) {
    *c = 1 / 3.0 * (*(c - 1) + *(c) + *(c + 1));
  }
  IMP_LOG_WRITE(
      TERSE, std::copy(b, e, std::ostream_iterator<double>(IMP_STREAM, " ")));
  IMP_LOG_TERSE(std::endl);
  algebra::Vector2Ds pts;
  for (It c = b; c < e; ++c) {
    pts.push_back(algebra::Vector2D(std::distance(b, c), *c));
  }
  algebra::LinearFit lf(pts);
  // add 1 to handle case where nothing much moves
  if (lf.get_a() < 1 && lf.get_fit_error() / std::distance(b, e) <
                            algebra::get_squared(lf.get_b())) {
    IMP_LOG_TERSE("Accepting " << lf << std::endl);
    return true;
  } else {
    IMP_LOG_TERSE("Rejecting " << lf << std::endl);
    return false;
  }
}

bool is_ok_step(BrownianDynamics *bd, Configuration *c, double step) {
  // std::cout << "Trying time step " << step << std::endl;
  kernel::ParticlesTemp ps = bd->get_simulation_particles();
  c->load_configuration();
  bd->set_maximum_time_step(step);
  IMP_LOG_TERSE("Trying step " << step << "(" << bd->get_maximum_time_step()
                               << ", " << bd->get_maximum_time_step() << ")"
                               << std::endl);
  IMP_USAGE_CHECK((step - bd->get_maximum_time_step()) < .001,
                  "In and out don't match " << bd->get_maximum_time_step());
  Floats es;
  unsigned int ns = 100;
  base::Vector<algebra::Vector3Ds> coords(ns, algebra::Vector3Ds(ps.size()));
  for (unsigned int i = 0; i < ns; ++i) {
    es.push_back(bd->optimize(1));
    for (unsigned int j = 0; j < coords[i].size(); ++j) {
      coords[i][j] = core::XYZ(ps[j]).get_coordinates();
    }
  }
  Floats max_dist(es.size() - 1, 0);
  for (unsigned int i = 0; i < ns - 1; ++i) {
    for (unsigned int j = 0; j < coords[i].size(); ++j) {
      max_dist[i] = std::max(max_dist[i],
                             (coords[i][j] - coords[i + 1][j]).get_magnitude());
    }
  }
  // std::cout << "Distances are "  << max_dist << std::endl;
  // std::cout << "Energies are "  << es << std::endl;
  return is_constant(es.begin(), es.end()) &&
         is_constant(max_dist.begin(), max_dist.end());
}
};

double get_maximum_time_step_estimate(BrownianDynamics *bd) {
  IMP_NEW(Configuration, c, (bd->get_model()));
  double ots = bd->get_maximum_time_step();
  double lb = 10;
  while (is_ok_step(bd, c, lb)) {
    lb *= 2;
  }
  double ub = lb * 2;
  for (unsigned int i = 0; i < 5; ++i) {
    double cur = (ub + lb) * .5;
    if (is_ok_step(bd, c, cur)) {
      lb = cur;
    } else {
      ub = cur;
    }
  }
  bd->set_maximum_time_step(ots);
  c->load_configuration();
  return lb;
}

double get_harmonic_sigma(double D, double f) {
  IMP_UNUSED(D);
  IMP_UNUSED(f);
  IMP_NOT_IMPLEMENTED;
  return 0;
}

IMPATOM_END_NAMESPACE
