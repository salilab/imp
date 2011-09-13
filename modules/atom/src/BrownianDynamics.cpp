/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/BrownianDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/log.h>
#include <IMP/random.h>
#include <IMP/constants.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/units.h>
#include <boost/random/normal_distribution.hpp>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/Configuration.h>
#include <IMP/algebra/LinearFit.h>

#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/vector_generators.h>

#include <cmath>
#include <limits>

IMPATOM_BEGIN_NAMESPACE


typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;


BrownianDynamics::BrownianDynamics(Model *m) :
  Simulator(m, "BrownianDynamics %1%"), //nd_(0,1),
  //sampler_(random_number_generator, nd_),
  max_step_(std::numeric_limits<double>::max()),
  srk_(false) {
}


/*
  radius
  if step is xi*radius^2/(3pi E) for some E=1
  then the motion per step should be sqrt(2kT/(pi E))

  T* is
 */


bool BrownianDynamics::get_is_simulation_particle(ParticleIndex pi) const {
  return (Diffusion::particle_is_instance(get_model(), pi)
          && IMP::core::XYZ(get_model(), pi).get_coordinates_are_optimized());
}

namespace {
  inline unit::Angstrom get_force(Model *m, ParticleIndex p, unsigned int i,
                           unit::Divide<unit::Femtosecond,
                                        unit::Femtojoule>::type dtikt) {
    Diffusion d(m, p);
    unit::Femtonewton nforce(get_force_in_femto_newtons(-d.get_derivative(i)));
    //unit::Angstrom R(sampler_());
    unit::Angstrom force_term(nforce*
                              unit::SquareAngstromPerFemtosecond(d.get_d())
                              *dtikt);
    /*if (force_term > unit::Angstrom(.5)) {
      std::cout << "Forces on " << _1->get_name() << " are "
      << force << " and " << nforce
      << " and " << force_term <<
      " vs " << delta[j] << ", " << sigma << std::endl;
      }*/
    return force_term;
  }
  // radians
  inline double get_torque(Model *m, ParticleIndex p, unsigned int i,
                    unit::Divide<unit::Femtosecond,
                                 unit::Femtojoule>::type dtikt) {
    RigidBodyDiffusion d(m, p);
    core::RigidBody rb(m, p);

    unit::KilocaloriePerMol cforce( rb.get_torque()[i]);
    unit::Joule nforce
      = unit::convert_Cal_to_J(cforce/unit::ATOMS_PER_MOL);
    //unit::Angstrom R(sampler_());
    double force_term=unit::strip_units(d.get_d_rotation()*(nforce*dtikt));
    /*if (force_term > unit::Angstrom(.5)) {
      std::cout << "Forces on " << _1->get_name() << " are "
      << force << " and " << nforce
      << " and " << force_term <<
      " vs " << delta[j] << ", " << sigma << std::endl;
      }*/
    return force_term;
  }

  unit::Angstrom get_sigma(Model *m, ParticleIndex p,
                           unit::Femtosecond dtfs) {
    return sqrt(6.0*unit::SquareAngstromPerFemtosecond(Diffusion(m,
                                                                 p).get_d())
                *dtfs);
  }
  double get_rotational_sigma(Model *m, ParticleIndex p,
                              unit::Femtosecond dtfs) {
    return sqrt(6.0*unit::PerFemtosecond(RigidBodyDiffusion(m,
                                                             p)
                                         .get_d_rotation())
                *dtfs);
  }
}

void BrownianDynamics::setup(const ParticleIndexes& ips) {
  IMP_IF_LOG(TERSE) {
    ParticlesTemp ps= IMP::internal::get_particle(get_model(), ips);
    unit::Femtosecond dtfs=unit::Femtosecond(get_maximum_time_step());
    unit::Divide<unit::Femtosecond,
                 unit::Femtojoule>::type dtikt
      =dtfs
      /IMP::unit::Femtojoule(IMP::internal::KB*unit::Kelvin(get_temperature()));
    double ms=0;
    double mf=0;
    evaluate(true);
    for (unsigned int i=0; i< ps.size(); ++i) {
      double c= strip_units(get_sigma(get_model(),
                                      ips[i],
                                      dtfs));
      ms= std::max(ms, c);
      for (unsigned int j=0; j< 3; ++j) {
        double f= strip_units(get_force(get_model(), ips[i], j, dtikt));
        mf=std::max(mf, f);
      }
    }
    IMP_LOG(TERSE, "Maximum sigma is " << ms << std::endl);
    IMP_LOG(TERSE, "Maximum force is " << mf << std::endl);
  }
  forces_.resize(ips.size());
}
IMP_GCC_DISABLE_WARNING("-Wuninitialized")

namespace {
  void check_delta(algebra::Vector3D &delta,
                   double max_step) {
    for (unsigned int j=0; j< 3; ++j) {
        if (std::abs(delta[j]) > max_step) {
          std::cerr << "Truncating motion: " << delta[j] << " to " << max_step
                    << std::endl;
          delta[j]= std::min(delta[j], max_step);
          delta[j]= std::max(delta[j], -max_step);
        }
      }
  }
}

void BrownianDynamics
::advance_ball_1(ParticleIndex pi,
                 unsigned int i,
                 unit::Femtosecond /*dtfs*/,
                 unit::Divide<unit::Femtosecond,
                              unit::Femtojoule>::type dtikt) {
  Diffusion d(get_model(), pi);
  core::XYZ xd(get_model(), pi);
  algebra::Vector3D force(get_force(get_model(), pi,
                                    0, dtikt).get_value(),
                          get_force(get_model(), pi,
                                    1, dtikt).get_value(),
                          get_force(get_model(), pi,
                                    2, dtikt).get_value());
  algebra::Vector3D delta=(force-forces_[i])/2.0;
  check_delta(delta, max_step_);
  xd.set_coordinates(xd.get_coordinates()+delta);
}


void BrownianDynamics
::advance_ball_0(ParticleIndex pi, unsigned int i,
                 unit::Femtosecond dtfs,
                 unit::Divide<unit::Femtosecond,
                              unit::Femtojoule>::type dtikt) {
  core::XYZ xd(get_model(), pi);
  double sigma= get_sigma(get_model(), pi, dtfs).get_value();
  boost::normal_distribution<double> nd(0, sigma);
  RNG sampler(random_number_generator, nd);
  double r= sampler();
  algebra::Vector3D random
    = r*get_random_vector_on(algebra::get_unit_sphere_d<3>());
  algebra::Vector3D force(get_force(get_model(), pi, 0,
                                    dtikt).get_value(),
                          get_force(get_model(), pi, 1,
                                    dtikt).get_value(),
                          get_force(get_model(), pi, 2,
                                    dtikt).get_value());
  if (srk_) {
    forces_[i]=force;
  }
  algebra::Vector3D delta=random+force;
  if (!srk_) {
    check_delta(delta, max_step_);
  }
  xd.set_coordinates(xd.get_coordinates()+delta);
}

void BrownianDynamics
::advance_rigid_body_0(ParticleIndex pi, unsigned int ,
                       unit::Femtosecond dtfs,
                       unit::Divide<unit::Femtosecond,
                                    unit::Femtojoule>::type dtikt) {
  core::RigidBody rb(get_model(), pi);
  double sigma= get_rotational_sigma(get_model(), pi, dtfs);
  boost::normal_distribution<double> nd(0, sigma);
  RNG sampler(random_number_generator, nd);
  double angle= sigma*sampler();
  algebra::Vector3D axis
    = algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
  algebra::Rotation3D rrot= algebra::get_rotation_about_axis(axis, angle);
  algebra::Vector3D torque( get_torque(get_model(), pi, 0, dtikt),
                            get_torque(get_model(), pi, 1, dtikt),
                            get_torque(get_model(), pi, 2, dtikt));
  double tangle= torque.get_magnitude();
  algebra::Vector3D taxis;
  if (tangle > 0) {
    taxis = torque/tangle;
  } else {
    taxis= algebra::Vector3D(0,0,0);
  }
  algebra::Rotation3D frot= algebra::get_rotation_about_axis(taxis, tangle);
  algebra::Transformation3D nt
    = rb.get_reference_frame().get_transformation_to()*
    algebra::Transformation3D(rrot)*algebra::Transformation3D(frot);
  rb.set_reference_frame(algebra::ReferenceFrame3D(nt));
}


/**
    dx= D/2kT*(F(x0)+F(x0+D/kTF(x0)dt +R)dt +R
 */
double BrownianDynamics::do_step(const ParticleIndexes &ps,
                                 double dt) {
  unit::Femtosecond dtfs(dt);
  unit::Divide<unit::Femtosecond,
               unit::Femtojoule>::type dtikt
    =dtfs
    /IMP::unit::Femtojoule(IMP::internal::KB*unit::Kelvin(get_temperature()));
  evaluate(true);
  unsigned int numrb=0;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (RigidBodyDiffusion::particle_is_instance(get_model(), ps[i])) {
      advance_rigid_body_0(ps[i], numrb, dtfs, dtikt);
      ++numrb;
    }
    advance_ball_0(ps[i], i, dtfs, dtikt);
  }
  if (srk_) {
    evaluate(true);
    for (unsigned int i=0; i< ps.size(); ++i) {
      advance_ball_1(ps[i], i, dtfs, dtikt);
    }
  }
  return dt;
}

namespace {
  template <class It>
  bool is_constant(It b, It e) {
    // smooth some
    for (It c= b+1 ; c<e-1; ++c) {
      *c= 1/3.0*(*(c-1)+*(c)+*(c+1));
    }
    IMP_LOG_WRITE(TERSE, std::copy(b,e,
                       std::ostream_iterator<double>(IMP_STREAM, " ")));
    IMP_LOG(TERSE, std::endl);
    algebra::Vector2Ds pts;
    for (It c= b; c< e; ++c) {
      pts.push_back(algebra::Vector2D(std::distance(b,c),
                                      *c));
    }
    algebra::LinearFit lf(pts);
    if (lf.get_a() < std::sqrt(lf.get_fit_error())/square(std::distance(b,e))
        +.01*lf.get_b()) {
      return true;
    } else {
      IMP_LOG(TERSE, "Rejecting " << lf << std::endl);
      return false;
    }
  }

  bool is_ok_step(BrownianDynamics *bd, Configuration *c, double step) {
    ParticlesTemp ps(bd->particles_begin(), bd->particles_end());
    c->load_configuration();
    bd->set_maximum_time_step(step);
    IMP_LOG(TERSE, "Trying step " << step << "("
              << bd->get_maximum_time_step()
            << ", " << bd->get_maximum_time_step() << ")" << std::endl);
    IMP_USAGE_CHECK((step- bd->get_maximum_time_step()) < .001,
                    "In and out don't match " << bd->get_maximum_time_step());
    Floats es;
    unsigned int ns=100;
    std::vector<algebra::Vector3Ds>
      coords(ns, algebra::Vector3Ds(ps.size()));
    for (unsigned int i=0; i< ns; ++i) {
      es.push_back(bd->optimize(1));
      for (unsigned int j=0; j< coords[i].size(); ++j) {
        coords[i][j]= core::XYZ(ps[j]).get_coordinates();
      }
    }
    Floats max_dist(es.size()-1, 0);
    for (unsigned int i=0; i< ns-1; ++i) {
      for (unsigned int j=0; j< coords[i].size(); ++j) {
        max_dist[i]= std::max(max_dist[i],
                              (coords[i][j]-coords[i+1][j]).get_magnitude());
      }
    }
    return is_constant(es.begin(), es.end())
      && is_constant(max_dist.begin(), max_dist.end());
  }
};


IMPATOMEXPORT double get_maximum_time_step_estimate(BrownianDynamics *bd){
  IMP_NEW(Configuration, c, (bd->get_model()));
  double ots= bd->get_maximum_time_step();
  double lb=10;
  while (is_ok_step(bd, c, lb)) {
    lb*=2;
  }
  double ub=lb*2;
  for (unsigned int i=0; i < 5; ++i) {
    double cur= (ub+lb)*.5;
    if (is_ok_step(bd, c, cur)) {
      lb=cur;
    } else {
      ub=cur;
    }
  }
  bd->set_maximum_time_step(ots);
  c->load_configuration();
  return lb;
}




double get_harmonic_sigma(double D, double f) {
  return 0;
}

IMPATOM_END_NAMESPACE
