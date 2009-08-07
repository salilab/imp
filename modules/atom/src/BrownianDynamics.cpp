/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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
#include <IMP/core/ListSingletonContainer.h>
#include <IMP/atom/Diffusion.h>

#include <cmath>
#include <limits>

IMPATOM_BEGIN_NAMESPACE
namespace {
  struct BadStepException{
    Particle *blamed;
    BadStepException(Particle *p): blamed(p){}
  };
}


typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;


BrownianDynamics::BrownianDynamics(SimulationParameters si,
                                   SingletonContainer *sc) :
  feature_size_2_(std::numeric_limits<double>::max()),
  si_(si)
{
  if (sc) sc_=sc;

  failed_steps_=0;
  successful_steps_=0;
}


void BrownianDynamics::show(std::ostream &out) const {
  out << "BrownianDynamics optimizer" << std::endl;
}

SingletonContainer *BrownianDynamics::setup_particles()
{
  if (sc_) {
    // check them
    for (SingletonContainer::ParticleIterator it= sc_->particles_begin();
         it != sc_->particles_end(); ++it) {
      IMP_check(Diffusion::particle_is_instance(*it),
                "Particles must be Diffusion particles to be used in "
                << "Brownian dynamics. Particle "<< (*it)->get_name()
                << " is not.",
                InvalidStateException);
    }
    return sc_;
  } else {
    core::ListSingletonContainer *lsc= new core::ListSingletonContainer();
    for (Model::ParticleIterator it = get_model()->particles_begin();
         it != get_model()->particles_end(); ++it) {
      Particle *p = *it;
      Diffusion d= Diffusion::decorate_particle(p);
      if (d && d.get_coordinates_are_optimized()) {
        lsc->add_particle(p);
      }
    }
    return lsc;
  }
}

void BrownianDynamics::copy_coordinates(SingletonContainer *sc,
                                        algebra::Vector3Ds &v) const {
  v.resize(sc->get_number_of_particles());
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    core::XYZ d(sc->get_particle(i));
    v[i]= d.get_coordinates();
  }
}

void BrownianDynamics::revert_coordinates(SingletonContainer *sc,
                                          algebra::Vector3Ds &v) {
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    core::XYZ d(sc->get_particle(i));
    d.set_coordinates(v[i]);
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
 return simulate(si_.get_current_time().get_value()
                 +max_steps*si_.get_maximum_time_step().get_value());
}

namespace {
  struct AddTime {
    SimulationParameters si_;
    unit::Femtosecond orig_time_;
    unit::Femtosecond accum_;
    AddTime(SimulationParameters si): si_(si),
                                      orig_time_(si_.get_current_time()),
                                      accum_(0){}
    void add(unit::Femtosecond a) {
      accum_= accum_+a;
      si_.set_current_time(orig_time_+accum_);
    }
    ~AddTime() {
      si_.set_current_time(orig_time_+accum_);
    }
    unit::Femtosecond get_current_time() const {
      return orig_time_ + accum_;
    }
  };
}


void BrownianDynamics::take_step(SingletonContainer *sc,
                                 unit::Femtosecond dt) {
  unit::Divide<unit::Femtosecond,
    unit::Femtojoule>::type dtikt=dt/si_.get_kT();
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    Particle *p= sc->get_particle(i);
    Diffusion d(p);

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

    IMP_check(unit::strip_units(d.get_D()) > 0
              && unit::strip_units(d.get_D())
              < std::numeric_limits<Float>::max(),
              "Bad diffusion coefficient on particle " << p->get_name(),
              ValueException);
    unit::Angstrom sigma= d.get_sigma(dt);

    IMP_LOG(VERBOSE, p->get_name() << ": sigma is "
            << sigma << std::endl);
    boost::normal_distribution<double> mrng(0, sigma.get_value());
    boost::variate_generator<RandomNumberGenerator&,
      boost::normal_distribution<double> >
      sampler(random_number_generator, mrng);

    //std::cout << p->get_name() << std::endl;

    unit::Angstrom delta[3];

    do {
      for (unsigned j = 0; j < 3; ++j) {
        delta[j]=unit::Angstrom(sampler());
      }
      // kill the tail
    } while (square(delta[0])+square(delta[1])+square(delta[2])
             > square(10.0*sigma));

    for (unsigned j = 0; j < 3; ++j) {
      unit::KilocaloriePerAngstromPerMol
        force( -d.get_derivative(j));
      unit::Femtonewton nforce
        = unit::convert_Cal_to_J(force/unit::ATOMS_PER_MOL);
      unit::Angstrom R(sampler());
      unit::Angstrom force_term(nforce*d.get_D()*dtikt);
      delta[j]= delta[j]+force_term;
    }

    //unit::Angstrom max_motion= unit::Scalar(4)*sigma;
    /*std::cout << "delta is " << delta << " mag is "
      << delta.get_magnitude() << " sigma " << sigma << std::endl;*/

    IMP_LOG(VERBOSE, "For particle " << p->get_name()
            << " delta is " << delta[0] << " " << delta[1] << " " << delta[2]
            << " from a force of "
            << "[" << d.get_derivatives() << "]" << std::endl);

    if (square(delta[0])+square(delta[1])+square(delta[2]) > feature_size_2_) {
      throw BadStepException(p);
    }
    for (unsigned int j=0; j< 3; ++j) {
      d.set_coordinate(j, d.get_coordinate(j) + unit::strip_units(delta[j]));
    }
  }
}

double BrownianDynamics::simulate(float max_time_nu)
{
  unit::Femtosecond max_time(max_time_nu);
  IMP_OBJECT_LOG;
  IMP_check(get_model() != NULL, "Must set model before calling simulate",
            ValueException);
  Pointer<SingletonContainer> sc
    = Pointer<SingletonContainer>(setup_particles());
  IMP_LOG(TERSE, "Running brownian dynamics on "
          << sc->get_number_of_particles()
          << " particles with a step of " << si_.get_maximum_time_step()
          << " until time " << max_time << std::endl);
  algebra::Vector3Ds old_forces, old_coordinates;
  unit::Femtosecond dt=si_.get_maximum_time_step();
  get_model()->evaluate(true);
  update_states();
  copy_coordinates(sc, old_coordinates);
  AddTime at(si_);
  bool success=false;
  while (at.get_current_time() < max_time){
    dt= std::min(dt, max_time-at.get_current_time());
    get_model()->evaluate(true);
    if (success) {
      update_states();
      copy_coordinates(sc, old_coordinates);
    }
    try {
      take_step(sc, unit::Femtosecond(dt));
      at.add( dt );
      si_.set_last_time_step(dt);
      dt= std::min(si_.get_maximum_time_step(),dt*1.1);
      if (dt < si_.get_maximum_time_step()) {
        IMP_LOG(TERSE, "Updating dt to " << dt
                << " (" << si_.get_maximum_time_step() << ")" << std::endl);
      }
      ++successful_steps_;
      success=true;
     } catch (const BadStepException &e) {
      ++failed_steps_;
      IMP_LOG(TERSE, "Reducing step size to " << dt/2.0
              << " because of particle "
              << e.blamed->get_name()
              << " steps: " << failed_steps_ << " vs "
              << successful_steps_ << std::endl);
      revert_coordinates(sc, old_coordinates);
      get_model()->evaluate(true);
      dt= dt*.5;
      if (dt < unit::Femtosecond(1)) {
        IMP_failure("Something is wrong with the restraints"
                    << " and they are highly discontinuous due"
                    << " to particle " << *e.blamed
                    << "\n" << *get_model(),
                    InvalidStateException);
      }
      success=false;
    }
  }
  double v= get_model()->evaluate(false);
  update_states();
  return v;
}

/*
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

unit::KilocaloriePerAngstromPerMol BrownianDynamics
::get_force_scale_from_D(unit::SquareCentimeterPerSecond D) const
{
  // force motion is f*dt_*D/kT
  // sigma*kT/(dt_*D)
  unit::Angstrom s=compute_sigma_from_D(D);
  unit::Piconewton pn= s*kt()/(si_.get_maximum_time_step()*D);
  unit::YoctoKilocaloriePerAngstrom yc=unit::convert_J_to_Cal(pn);
  return unit::operator*(unit::ATOMS_PER_MOL,yc);
}

*/
IMPATOM_END_NAMESPACE
