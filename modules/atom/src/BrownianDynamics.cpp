/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/BrownianDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/utility.h>
#include <IMP/log.h>
#include <IMP/random.h>
#include <IMP/constants.h>
#include <IMP/atom/constants.h>
#include <IMP/base/warning_macros.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/units.h>
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


typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;


BrownianDynamics::BrownianDynamics(Model *m,
                                   std::string name) :
  Simulator(m, name), //nd_(0,1),
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
  inline double get_force(Model *m, ParticleIndex p, unsigned int i,
                          double dt, double ikT) {
    Diffusion d(m, p);
    double nforce(-d.get_derivative(i));
    //unit::Angstrom R(sampler_());
    double dd=d.get_diffusion_coefficient();
    double force_term(nforce*dd*dt*ikT);
    /*if (force_term > unit::Angstrom(.5)) {
      std::cout << "Forces on " << _1->get_name() << " are "
      << force << " and " << nforce
      << " and " << force_term <<
      " vs " << delta[j] << ", " << sigma << std::endl;
      }
    std::cout << "Force " << i << " is " << force_term
              << "= " << nforce << "*" << dd << "*" << dt << "*" << ikT
              << std::endl;*/
    return force_term;
  }
  // radians
  inline double get_torque(Model *m, ParticleIndex p, unsigned int i,
                           double dt, double ikT) {
    RigidBodyDiffusion d(m, p);
    core::RigidBody rb(m, p);

    double cforce( rb.get_torque()[i]);
    //unit::Angstrom R(sampler_());
    double dr=d.get_rotational_diffusion_coefficient();
    double force_term=dr*cforce*dt*ikT;
    /*if (force_term > unit::Angstrom(.5)) {
      std::cout << "Forces on " << _1->get_name() << " are "
      << force << " and " << nforce
      << " and " << force_term <<
      " vs " << delta[j] << ", " << sigma << std::endl;
      }*/
    return -force_term;
  }

  inline double get_sigma(Model *m, ParticleIndex p,
                   double dtfs) {
    // 6.0 since we are picking radius rather than the coordinates
    double dd=Diffusion(m,
                        p).get_diffusion_coefficient();
    return sqrt(6.0*dd*dtfs);
  }
  inline double get_rotational_sigma(Model *m, ParticleIndex p,
                              double dtfs) {
    double dr=RigidBodyDiffusion(m, p)
        .get_rotational_diffusion_coefficient();
    return sqrt(6.0*dr*dtfs);
  }
}

void BrownianDynamics::setup(const ParticleIndexes& ips) {
  IMP_IF_LOG(TERSE) {
    ParticlesTemp ps= IMP::internal::get_particle(get_model(), ips);
    double dtfs=get_maximum_time_step();
    double ikT= 1.0/get_kt();
    double ms=0;
    double mf=0;
    get_scoring_function()->evaluate(true);
    for (unsigned int i=0; i< ps.size(); ++i) {
      double c= get_sigma(get_model(),
                          ips[i],
                          dtfs);
      ms= std::max(ms, c);
      for (unsigned int j=0; j< 3; ++j) {
        double f= get_force(get_model(), ips[i], j, dtfs, ikT);
        mf=std::max(mf, f);
      }
    }
    IMP_LOG_TERSE( "Maximum sigma is " << ms << std::endl);
    IMP_LOG_TERSE( "Maximum force is " << mf << std::endl);
  }
  forces_.resize(ips.size());
}
IMP_GCC_DISABLE_WARNING(-Wuninitialized)

namespace {
  void check_delta(algebra::Vector3D &delta,
                   double max_step) {
    for (unsigned int j=0; j< 3; ++j) {
      //if (std::abs(delta[j]) > max_step) {
        /*std::cerr << "Truncating motion: " << delta[j] << " to " << max_step
          << std::endl;*/
        delta[j]= std::min(delta[j], max_step);
        delta[j]= std::max(delta[j], -max_step);
        //}
    }
  }
}

void BrownianDynamics
::advance_coordinates_1(ParticleIndex pi,
                 unsigned int i,
                 double dt, double ikT) {
  Diffusion d(get_model(), pi);
  core::XYZ xd(get_model(), pi);
  algebra::Vector3D force(get_force(get_model(), pi,
                                    0, dt, ikT),
                          get_force(get_model(), pi,
                                    1, dt, ikT),
                          get_force(get_model(), pi,
                                    2, dt, ikT));
  algebra::Vector3D delta=(force-forces_[i])/2.0;
  check_delta(delta, max_step_);
  xd.set_coordinates(xd.get_coordinates()+delta);
}


void BrownianDynamics
::advance_coordinates_0(ParticleIndex pi, unsigned int i,
                 double dtfs,
                 double ikT) {
  core::XYZ xd(get_model(), pi);
  double sigma= get_sigma(get_model(), pi, dtfs);
  boost::normal_distribution<double> nd(0, sigma);
  RNG sampler(random_number_generator, nd);
  double r= sampler();
  algebra::Vector3D random
    = r*get_random_vector_on(algebra::get_unit_sphere_d<3>());
  algebra::Vector3D force(get_force(get_model(), pi, 0,
                                    dtfs, ikT),
                          get_force(get_model(), pi, 1,
                                    dtfs, ikT),
                          get_force(get_model(), pi, 2,
                                    dtfs, ikT));
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
::advance_orientation_0(ParticleIndex pi,
                       double dtfs,
                       double ikT) {
  core::RigidBody rb(get_model(), pi);
  double sigma= get_rotational_sigma(get_model(), pi, dtfs);
  boost::normal_distribution<double> nd(0, sigma);
  RNG sampler(random_number_generator, nd);
  double angle= sampler();
  algebra::Transformation3D nt=rb.get_reference_frame().get_transformation_to();
  algebra::Vector3D axis
    = algebra::get_random_vector_on(algebra::get_unit_sphere_d<3>());
  algebra::Rotation3D rrot= algebra::get_rotation_about_axis(axis, angle);
  nt=nt*rrot;
  algebra::Vector3D torque( get_torque(get_model(), pi, 0, dtfs, ikT),
                            get_torque(get_model(), pi, 1, dtfs, ikT),
                            get_torque(get_model(), pi, 2, dtfs, ikT));
  double tangle= torque.get_magnitude();
  if (tangle > 0) {
    algebra::Vector3D taxis = torque/tangle;
    algebra::Rotation3D frot= algebra::get_rotation_about_axis(taxis, tangle);
    nt= nt*frot;
  }
  rb.set_reference_frame_lazy(algebra::ReferenceFrame3D(nt));
  IMP_LOG_VERBOSE("Advancing rigid body "
                  << get_model()->get_particle(pi)->get_name()
                  << " to " << nt
                  << std::endl);
}

void BrownianDynamics::advance_chunk(double dtfs, double ikT,
                                     const ParticleIndexes &ps,
                                     unsigned int begin,
                                     unsigned int end) {
  IMP_LOG_TERSE("Advancing particles " << begin << " to " << end << std::endl);
  for (unsigned int i=begin; i< end; ++i) {
    if (RigidBodyDiffusion::particle_is_instance(get_model(), ps[i])) {
      //std::cout << "rb" << std::endl;
      advance_orientation_0(ps[i], dtfs, ikT);
    } else {
#if IMP_HAS_CHECKS >= IMP_INTERNAL
      Particle *p= get_model()->get_particle(ps[i]);
      IMP_INTERNAL_CHECK(!core::RigidBody::particle_is_instance(p),
                         "A rigid body without rigid body diffusion info"
                         << " was found: "
                         << p->get_name());
      IMP_INTERNAL_CHECK(!core::RigidMember::particle_is_instance(p),
                         "A rigid member with diffusion info"
                         << " was found: "
                         << p->get_name());
#endif
    }
    advance_coordinates_0(ps[i], i, dtfs, ikT);
  }
}

/**
    dx= D/2kT*(F(x0)+F(x0+D/kTF(x0)dt +R)dt +R
 */
double BrownianDynamics::do_step(const ParticleIndexes &ps,
                                 double dt) {
  double dtfs(dt);
  double ikT= 1.0/get_kt();
  get_scoring_function()->evaluate(true);
  const unsigned int chunk_size=20;
  for (unsigned int b=0; b< ps.size(); b+=chunk_size) {
    IMP_TASK_SHARED((dtfs, ikT, b), (ps),
                    advance_chunk(dtfs, ikT, ps, b,
                                  std::min<unsigned int>(b+chunk_size,
                                                         ps.size()));,
                    "brownian"
                    );
  }
IMP_OMP_PRAGMA(taskwait)
IMP_OMP_PRAGMA(flush)
  if (srk_) {
    get_scoring_function()->evaluate(true);
    for (unsigned int i=0; i< ps.size(); ++i) {
      advance_coordinates_1(ps[i], i, dtfs, ikT);
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
    IMP_LOG_TERSE( std::endl);
    algebra::Vector2Ds pts;
    for (It c= b; c< e; ++c) {
      pts.push_back(algebra::Vector2D(std::distance(b,c),
                                      *c));
    }
    algebra::LinearFit lf(pts);
    // add 1 to handle case where nothing much moves
    if (lf.get_a() < 1
        && lf.get_fit_error()/std::distance(b,e)
        < algebra::get_squared(lf.get_b())) {
      IMP_LOG_TERSE( "Accepting " << lf << std::endl);
      return true;
    } else {
      IMP_LOG_TERSE( "Rejecting " << lf << std::endl);
      return false;
    }
  }

  bool is_ok_step(BrownianDynamics *bd, Configuration *c, double step) {
    //std::cout << "Trying time step " << step << std::endl;
    ParticlesTemp ps=bd->get_simulation_particles();
    c->load_configuration();
    bd->set_maximum_time_step(step);
    IMP_LOG_TERSE( "Trying step " << step << "("
              << bd->get_maximum_time_step()
            << ", " << bd->get_maximum_time_step() << ")" << std::endl);
    IMP_USAGE_CHECK((step- bd->get_maximum_time_step()) < .001,
                    "In and out don't match " << bd->get_maximum_time_step());
    Floats es;
    unsigned int ns=100;
    base::Vector<algebra::Vector3Ds>
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
    //std::cout << "Distances are "  << max_dist << std::endl;
    //std::cout << "Energies are "  << es << std::endl;
    return is_constant(es.begin(), es.end())
      && is_constant(max_dist.begin(), max_dist.end());
  }
};


double get_maximum_time_step_estimate(BrownianDynamics *bd){
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
  IMP_UNUSED(D);
  IMP_UNUSED(f);
  IMP_NOT_IMPLEMENTED;
  return 0;
}

IMPATOM_END_NAMESPACE
