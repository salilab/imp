/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/BrownianDynamics.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/algebra/Vector3D.h>

#include <IMP/log.h>
#include <IMP/random.h>
#include <IMP/constants.h>
#include <IMP/internal/constants.h>
#include <IMP/internal/units.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;


BrownianDynamics::BrownianDynamics() :
  dt_(1e7), cur_time_(0),
  T_(IMP::internal::DEFAULT_TEMPERATURE)
{
}


BrownianDynamics::~BrownianDynamics()
{
}

IMP_LIST_IMPL(BrownianDynamics, Particle, particle, Particle*,,,);


void BrownianDynamics::set_time_step(unit::Femtosecond t)
{
  dt_= t;
}

void BrownianDynamics::setup_particles()
{
  clear_particles();
  FloatKeys xyzk=XYZDecorator::get_xyz_keys();
  for (Model::ParticleIterator it = get_model()->particles_begin();
       it != get_model()->particles_end(); ++it) {
    Particle *p = *it;
    if (p->has_attribute(xyzk[0]) && p->get_is_optimized(xyzk[0])
        && p->has_attribute(xyzk[1]) && p->get_is_optimized(xyzk[1])
        && p->has_attribute(xyzk[2]) && p->get_is_optimized(xyzk[2])
        && p->has_attribute(DiffusionDecorator::get_D_key())
        && !p->get_is_optimized(DiffusionDecorator::get_D_key())) {
      add_particle(p);
    }
  }
}

// rt_dt= rt+Fdt/psi + sqrt(2kTdt/psi)omega
// omega has variance 6 phi kT


/*
  radius
  if step is xi*radius^2/(3pi E) for some E=1
  then the motion per step should be sqrt(2kT/(pi E))

  T* is
 */


//! Optimize the model.
/** \param[in] max_steps   Maximum number of iterations before aborting.
    \return score of the final state of the model.
 */
Float BrownianDynamics::optimize(unsigned int max_steps)
{
 IMP_check(get_model() != NULL, "Must set model before calling optimize",
           ValueException);
  setup_particles();
  IMP_LOG(TERSE, "Running brownian dynamics on " << get_number_of_particles()
          << " particles with a step of " << dt_ << std::endl);
  setup_particles();
  for (unsigned int i = 0; i < max_steps; ++i) {
    take_step();
  }
  return get_model()->evaluate(false);
}


void BrownianDynamics::take_step() {
  update_states();
  get_model()->evaluate(true);

  FloatKeys xyzk=XYZDecorator::get_xyz_keys();

  IMP_LOG(VERBOSE, "dt is " << dt_ << std::endl);


  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    Particle *p= get_particle(i);
    XYZDecorator d(p);
    IMP_IF_CHECK(CHEAP) {
      for (unsigned int j=0; j< 3; ++j) {
        // GDB 6.6 prints infinity as 0 on 64 bit machines. Grumble.
        /*int szf= sizeof(Float);
          int szi= sizeof(int);
          Float one=1;*/
        Float mx= std::numeric_limits<Float>::max();
        Float c= d.get_coordinate(j);
        bool ba= (c != c);
        bool bb = (c >= mx);
        bool bc= -d.get_coordinate(j) >= std::numeric_limits<Float>::max();
        if (ba || bb || bc ) {
          IMP_WARN("Bad value for coordinate in Brownian Dynamics on "
                   << "particle " << p->get_name() << std::endl);
          throw ValueException("Bad coordinate value");
        }
      }
    }

    //double xi= 6*pi*eta*radius; // kg/s
    DiffusionDecorator dd(p);
    unit::SquareCentimeterPerSecond D= dd.get_D();
    IMP_check(D.get_value() > 0
              && D.get_value() < std::numeric_limits<Float>::max(),
              "Bad diffusion coefficient on particle " << p->get_name(),
              ValueException);
    unit::Angstrom sigma(compute_sigma_from_D(D));
    IMP_IF_CHECK(EXPENSIVE) {
      unit::Angstrom osigma(sqrt(2.0*D*dt_));
      IMP_check(sigma - osigma
                <= .01* sigma,
                "Sigma computations don't match " << sigma
                << " "
                << sqrt(2.0*D*dt_),
                ErrorException);
    }
    IMP_LOG(VERBOSE, p->get_name() << ": sigma is "
            << sigma << std::endl);
    boost::normal_distribution<double> mrng(0, sigma.get_value());
    boost::variate_generator<RandomNumberGenerator&,
      boost::normal_distribution<double> >
      sampler(random_number_generator, mrng);

    //std::cout << p->get_name() << std::endl;

    unit::Angstrom delta[3];

    for (unsigned j = 0; j < 3; ++j) {
      unit::KilocaloriePerAngstromPerMol
        force( -d.get_derivative(j));
      unit::Femtonewton nforce
        = unit::convert_Cal_to_J(force/unit::ATOMS_PER_MOL);
      unit::Angstrom R(sampler());
      unit::Angstrom force_term(nforce*dt_*D/kt());
      if (force_term > 5*sigma) {
        IMP_failure("Forces are too high to stably integrate: "
                    << *p, InvalidStateException);
      }
      //std::cout << "Force term is " << force_term << " and R is "
      //<< R << std::endl;
      unit::Angstrom dr= force_term +  R; //std::sqrt(2*kb*T_*ldt/xi) *
      // get back from meters
      delta[j]=dr;
    }

    //unit::Angstrom max_motion= unit::Scalar(4)*sigma;
    /*std::cout << "delta is " << delta << " mag is "
      << delta.get_magnitude() << " sigma " << sigma << std::endl;*/

    IMP_LOG(VERBOSE, "For particle " << p->get_name()
            << " delta is " << delta[0] << " " << delta[1] << " " << delta[2]
            << " from a force of "
            << "[" << d.get_derivatives() << "]" << std::endl);

    for (unsigned int j=0; j< 3; ++j) {
      d.set_coordinate(j, d.get_coordinate(j) + delta[j].get_value());
    }
  }
  cur_time_= cur_time_+dt_;
}

void BrownianDynamics::simulate(float max_time)
{
  IMP_check(get_model() != NULL, "Must set model before calling simulate",
            ValueException);
  setup_particles();
  unit::Femtosecond mt(max_time);
  while (cur_time_ < mt) {
    take_step();
  }
}


unit::Femtojoule BrownianDynamics::kt() const
{
  return IMP::unit::Femtojoule(IMP::internal::KB*T_);
}


unit::Angstrom
BrownianDynamics
::estimate_radius_from_mass_units(unit::Kilodalton mass_in_kd)
{
  //unit::KG kg= convert_to_mks(mass_in_kd)
  unit::Kilodalton kd(mass_in_kd);
  unit::GramPerCubicCentimeter p= unit::GramPerCubicCentimeter(1.410)
    + unit::GramPerCubicCentimeter(0.145)* exp(-mass_in_kd.get_value()/13);

  unit::Kilogram m= convert_to_mks(kd);
  // m/(4/3 pi r^3) =p
  // r= (m/p)/(4/(3 pi))^(1/3)
  typedef unit::Multiply<unit::Angstrom,
    unit::Angstrom>::type SquareAngstrom;
  typedef unit::Multiply<unit::SquareAngstrom,
    unit::Angstrom>::type CubicAngstrom;

  CubicAngstrom v((m/p)*(4.0/(3.0*PI)));

  return unit::Angstrom(std::pow(v.get_value(), .3333));
}

unit::Angstrom
BrownianDynamics
::compute_sigma_from_D(unit::SquareCentimeterPerSecond D) const
{
  return sqrt(2.0*D*dt_); //6*xi*kb*T_;
}

unit::KilocaloriePerAngstromPerMol BrownianDynamics
::get_force_scale_from_D(unit::SquareCentimeterPerSecond D) const
{
  // force motion is f*dt_*D/kT
  // sigma*kT/(dt_*D)
  unit::Angstrom s=compute_sigma_from_D(D);
  unit::Piconewton pn= s*kt()/(dt_*D);
  unit::YoctoKilocaloriePerAngstrom yc=unit::convert_J_to_Cal(pn);
  return unit::operator*(unit::ATOMS_PER_MOL,yc);
}

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE
