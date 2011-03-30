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

#include <cmath>
#include <limits>

IMPATOM_BEGIN_NAMESPACE


typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;


BrownianDynamics::BrownianDynamics(Model *m) :
  Simulator(m, "BrownianDynamics %1%"), nd_(0,1),
  sampler_(random_number_generator, nd_){
}


/*
  radius
  if step is xi*radius^2/(3pi E) for some E=1
  then the motion per step should be sqrt(2kT/(pi E))

  T* is
 */


bool BrownianDynamics::get_is_simulation_particle(Particle *p) const {
  return (Diffusion::particle_is_instance(p)
          && IMP::core::XYZ::particle_is_instance(p)
          && IMP::core::XYZ(p).get_coordinates_are_optimized());
}

namespace {
  unit::Angstrom get_force(Particle *p, unsigned int i,
                           unit::Divide<unit::Femtosecond,
                                        unit::Femtojoule>::type dtikt) {
    Diffusion d(p);
    unit::KilocaloriePerAngstromPerMol
      cforce( -d.get_derivative(i));
    unit::Femtonewton nforce
      = unit::convert_Cal_to_J(cforce/unit::ATOMS_PER_MOL);
    //unit::Angstrom R(sampler_());
    unit::Angstrom force_term(nforce*d.get_d()*dtikt);
    /*if (force_term > unit::Angstrom(.5)) {
      std::cout << "Forces on " << _1->get_name() << " are "
      << force << " and " << nforce
      << " and " << force_term <<
      " vs " << delta[j] << ", " << sigma << std::endl;
      }*/
    return force_term;
  }
  unit::Angstrom get_sigma(Particle *p, unit::Femtosecond dtfs) {
    return sqrt(2.0*Diffusion(p).get_d()*dtfs);
  }
}

void BrownianDynamics::setup(const ParticlesTemp& ps) {
  IMP_IF_LOG(TERSE) {
    unit::Femtosecond dtfs=unit::Femtosecond(get_maximum_time_step());
    unit::Divide<unit::Femtosecond,
                 unit::Femtojoule>::type dtikt
      =dtfs
      /IMP::unit::Femtojoule(IMP::internal::KB*unit::Kelvin(get_temperature()));
    double ms=0;
    double mf=0;
    evaluate(true);
    for (unsigned int i=0; i< ps.size(); ++i) {
      double c= strip_units(get_sigma(ps[i],
                                      dtfs));
      ms= std::max(ms, c);
      for (unsigned int j=0; j< 3; ++j) {
        double f= strip_units(get_force(ps[i], j, dtikt));
        mf=std::max(mf, f);
      }
    }
    IMP_LOG(TERSE, "Maximum sigma is " << ms << std::endl);
    IMP_LOG(TERSE, "Maximum force is " << mf << std::endl);
  }
}
IMP_GCC_DISABLE_WARNING("-Wuninitialized")

double BrownianDynamics::do_step(const ParticlesTemp &ps,
                                 double dt) {
  evaluate(true);
  unit::Femtosecond dtfs(dt);
  unit::Divide<unit::Femtosecond,
               unit::Femtojoule>::type dtikt
    =dtfs
    /IMP::unit::Femtojoule(IMP::internal::KB*unit::Kelvin(get_temperature()));
  for (unsigned int i=0; i< ps.size(); ++i) {
    Diffusion d(ps[i]);

    IMP_IF_CHECK(USAGE) {
      for (unsigned int j=0; j< 3; ++j) {
        // GDB 6.6 prints infinity as 0 on 64 bit machines. Grumble.
        /*int szf= sizeof(Float);
          int szi= sizeof(int);
          Float one=1;*/
        double mx= std::numeric_limits<double>::max();
        double c= d.get_coordinate(j);
        bool ba= is_nan(c);
        bool bb = (c >= mx);
        bool bc= -d.get_coordinate(j) >= std::numeric_limits<double>::max();
        if (ba || bb || bc ) {
          IMP_WARN("Bad value for coordinate in Brownian Dynamics on "
                   << "particle " << ps[i]->get_name() << std::endl);
          throw ValueException("Bad coordinate value");
        }
      }
    }

    IMP_USAGE_CHECK(unit::strip_units(d.get_d()) > 0
              && unit::strip_units(d.get_d())
              < std::numeric_limits<double>::max(),
              "Bad diffusion coefficient on particle " << ps[i]->get_name());
    double random[3];
    unit::Angstrom sigma= get_sigma(ps[i], dtfs);
    for (unsigned j = 0; j < 3; ++j) {
      double rv= sampler_();
      random[j]=unit::Angstrom(sigma*rv).get_value();
      //if (rv < 0) random[j]=-random[j];
      //delta[j]=unit::Angstrom(0);
    }
    /*std::cout << d.get_d() << " " << sigma << " " << dt
      << " " << d.get_d()*dt << std::endl;*/
    double force[3];
    for (unsigned j = 0; j < 3; ++j) {
      force[j]= strip_units(get_force(ps[i], j, dtikt));
    }

    //unit::Angstrom max_motion= unit::Scalar(4)*sigma;
    /*std::cout << "delta is " << delta << " mag is "
      << delta.get_magnitude() << " sigma " << sigma << std::endl;*/

    IMP_LOG(VERBOSE, "For particle " << ps[i]->get_name()
            << " random is " << random[0] << " "
            << random[1] << " " << random[2]
            << " from a force of "
            << force[0]<< " " << force[1] << " " << force[2] << std::endl);
    algebra::Vector3D sum;
    double nus3=5.0*strip_units(sigma);
    for (unsigned int j=0;j< 3; ++j) {
      sum[j]= force[j]+random[j];
      if (std::abs(sum[j]) > nus3) {
        std::cerr << "Truncating motion: " << sum[j] << " to " << nus3
                  << std::endl;
        sum[j]= std::min(sum[j], nus3);
        sum[j]= std::max(sum[j], -nus3);
      }
    }
    d.set_coordinates(d.get_coordinates()+sum);
  };
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
    std::vector<double> es;
    unsigned int ns=100;
    std::vector<algebra::Vector3Ds>
      coords(ns, algebra::Vector3Ds(ps.size()));
    for (unsigned int i=0; i< ns; ++i) {
      es.push_back(bd->optimize(1));
      for (unsigned int j=0; j< coords[i].size(); ++j) {
        coords[i][j]= core::XYZ(ps[j]).get_coordinates();
      }
    }
    std::vector<double> max_dist(es.size()-1, 0);
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

IMPATOM_END_NAMESPACE
