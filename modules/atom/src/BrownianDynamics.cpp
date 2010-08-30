/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
namespace {
  struct BadStepException{
    Particle *blamed;
    BadStepException(Particle *p): blamed(p){}
  };


  void relax_model(Model *m, SingletonContainer *sc) {
    //bool old= m->get_gather_statistics();
    IMP_IF_LOG(TERSE) {
      m->set_gather_statistics(true);
      double oscore= m->evaluate(false);
      IMP_WARN("Relaxing the model from a score of " << oscore << std::endl);
      m->set_gather_statistics(false);
      m->show_restraint_score_statistics(std::cerr);
    }
    std::cerr << "relaxing" << std::endl;
    IMP_NEW(core::ConjugateGradients, cg, (m));
    cg->optimize(10);
    IMP_IF_LOG(TERSE) {
      m->set_gather_statistics(true);
      double nscore=m->evaluate(true);
      IMP_WARN("Relaxed the model to a score of " << nscore << std::endl);
      m->set_gather_statistics(false);
      m->show_restraint_score_statistics(std::cerr);
    }
  }

}


typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;


BrownianDynamics::BrownianDynamics(SimulationParameters si,
                                   SingletonContainer *sc) :
  Optimizer(si->get_model()),
  feature_size_2_(std::numeric_limits<double>::max()),
  si_(si)
{
  if (sc) sc_=sc;

  failed_steps_=0;
  successful_steps_=0;
  dynamic_steps_=true;
  maximum_score_= std::numeric_limits<double>::max();
}


void BrownianDynamics::do_show(std::ostream &) const {
}

void BrownianDynamics::set_maximum_score(double d) {
  maximum_score_=d;
}

SingletonContainer *BrownianDynamics::setup_particles()
{
  if (sc_) {
    // check them
    for (SingletonContainer::ParticleIterator it= sc_->particles_begin();
         it != sc_->particles_end(); ++it) {
      IMP_USAGE_CHECK(Diffusion::particle_is_instance(*it),
                "Particles must be Diffusion particles to be used in "
                << "Brownian dynamics. Particle "<< (*it)->get_name()
                << " is not.");
    }
    return sc_;
  } else {
    container::ListSingletonContainer *lsc
      = new container::ListSingletonContainer(get_model(),
                                              "Brownian dynamics particles");
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

void BrownianDynamics::set_adjust_step_size(bool tf) {
  dynamic_steps_=tf;
}

void BrownianDynamics::copy_coordinates(SingletonContainer *sc,
                                 std::vector<algebra::VectorD<3> > &v) const {
  v.resize(sc->get_number_of_particles());
  IMP_FOREACH_SINGLETON(sc, {
      core::XYZ d(_1);
      v[_2]= d.get_coordinates();
    });
}

void BrownianDynamics::revert_coordinates(SingletonContainer *sc,
                                      std::vector<algebra::VectorD<3> > &v) {
  IMP_FOREACH_SINGLETON(sc, {
      core::XYZ d(_1);
      d.set_coordinates(v[_2]);
    });
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
 IMP_USAGE_CHECK(get_model() != NULL, "Must set model before calling optimize");
 return simulate(si_.get_current_time_with_units().get_value()
                 +max_steps*si_.get_maximum_time_step_with_units().get_value());
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
      si_.set_current_time_with_units(orig_time_+accum_);
    }
    ~AddTime() {
      si_.set_current_time_with_units(orig_time_+accum_);
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
  typedef boost::variate_generator<RandomNumberGenerator&,
    boost::normal_distribution<double> > RNG;
  boost::normal_distribution<double> mrng(0, unit::strip_units(sqrt(dt)));
  RNG sampler(random_number_generator, mrng);
  IMP_FOREACH_SINGLETON(sc, {
      Diffusion d(_1);

    IMP_IF_CHECK(USAGE) {
      for (unsigned int j=0; j< 3; ++j) {
        // GDB 6.6 prints infinity as 0 on 64 bit machines. Grumble.
        /*int szf= sizeof(Float);
          int szi= sizeof(int);
          Float one=1;*/
        Float mx= std::numeric_limits<Float>::max();
        Float c= d.get_coordinate(j);
        bool ba= is_nan(c);
        bool bb = (c >= mx);
        bool bc= -d.get_coordinate(j) >= std::numeric_limits<Float>::max();
        if (ba || bb || bc ) {
          IMP_WARN("Bad value for coordinate in Brownian Dynamics on "
                   << "particle " << _1->get_name() << std::endl);
          throw ValueException("Bad coordinate value");
        }
      }
    }

    IMP_USAGE_CHECK(unit::strip_units(d.get_D()) > 0
              && unit::strip_units(d.get_D())
              < std::numeric_limits<Float>::max(),
              "Bad diffusion coefficient on particle " << _1->get_name());
    double random[3];
    unit::Angstrom sigma= sqrt(2.0*d.get_D()*dt);
    for (unsigned j = 0; j < 3; ++j) {
      double rv= sampler();
      random[j]=unit::Angstrom(sqrt(2*d.get_D()
                                    *unit::Femtosecond(rv*rv))).get_value();
      if (rv < 0) random[j]=-random[j];
      //delta[j]=unit::Angstrom(0);
    }

    double force[3];
    for (unsigned j = 0; j < 3; ++j) {
      unit::KilocaloriePerAngstromPerMol
        cforce( -d.get_derivative(j));
      unit::Femtonewton nforce
        = unit::convert_Cal_to_J(cforce/unit::ATOMS_PER_MOL);
      unit::Angstrom R(sampler());
      unit::Angstrom force_term(nforce*d.get_D()*dtikt);
      /*if (force_term > unit::Angstrom(.5)) {
        std::cout << "Forces on " << _1->get_name() << " are "
                  << force << " and " << nforce
                  << " and " << force_term <<
          " vs " << delta[j] << ", " << sigma << std::endl;
          }*/
      if (force_term > 3.0*sigma) {
        force_term= 3.0*sigma;
      } else if (force_term < -3.0*sigma) {
        force_term= -3.0*sigma;
      }
      force[j]= unit::strip_units(force_term);
    }

    //unit::Angstrom max_motion= unit::Scalar(4)*sigma;
    /*std::cout << "delta is " << delta << " mag is "
      << delta.get_magnitude() << " sigma " << sigma << std::endl;*/

    IMP_LOG(VERBOSE, "For particle " << _1->get_name()
            << " random is " << random[0] << " "
            << random[1] << " " << random[2]
            << " from a force of "
            << force[0]<< " " << force[1] << " " << force[2] << std::endl);
    if (dynamic_steps_) {
      /*if (square(delta[0])+square(delta[1])+square(delta[2])
          > feature_size_2_) {
        throw BadStepException(_1);
        }*/
    }
    for (unsigned int j=0; j< 3; ++j) {
      d.set_coordinate(j, d.get_coordinate(j) + random[j]+force[j]);
    }
    }
    );
}

double BrownianDynamics::simulate(float max_time_nu)
{
  unit::Femtosecond max_time(max_time_nu);
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(get_model() != NULL,
                  "Must set model before calling simulate");
  IMP::internal::OwnerPointer<SingletonContainer> sc
    = IMP::internal::OwnerPointer<SingletonContainer>(setup_particles());
  IMP_LOG(TERSE, "Running brownian dynamics on "
          << sc->get_number_of_particles()
          << " particles with a step of " << si_.get_maximum_time_step()
          << " until time " << max_time << std::endl);
  std::vector<algebra::VectorD<3> > old_forces, old_coordinates;
  unit::Femtosecond dt=si_.get_maximum_time_step_with_units();
  double score= get_model()->evaluate(true);
  if (score > maximum_score_) {
    relax_model(get_model(), sc);
  }
  update_states();
  copy_coordinates(sc, old_coordinates);
  AddTime at(si_);
  bool success=false;
  while (at.get_current_time() < max_time){
    dt= std::min(dt, max_time-at.get_current_time());
    double score= get_model()->evaluate(true);
    if (score > maximum_score_) {
      relax_model(get_model(), sc);
    }
    if (success) {
      update_states();
      if (dynamic_steps_) copy_coordinates(sc, old_coordinates);
    }
    try {
      take_step(sc, unit::Femtosecond(dt));
      at.add( dt );
      si_.set_last_time_step_with_units(dt);
      dt= std::min(si_.get_maximum_time_step_with_units(),dt*1.1);
      if (dt < si_.get_maximum_time_step_with_units()) {
        IMP_LOG(TERSE, "Updating dt to " << dt
                << " (" << si_.get_maximum_time_step_with_units()
                << ")" << std::endl);
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
        IMP_THROW("Something is wrong with the restraints"
                    << " and they are highly discontinuous due"
                    << " to particle " << *e.blamed
                    << "\n" << *get_model(),
                    ValueException);
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

namespace {
  template <class It>
  bool is_constant(It b, It e) {
    // smooth some
    for (It c= b+1 ; c<e-1; ++c) {
      *c= 1/3.0*(*(c-1)+*(c)+*(c+1));
    }
    std::copy(b,e, std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;
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
      std::cout << "Rejecting " << lf << std::endl;
      return false;
    }
  }

  bool is_ok_step(BrownianDynamics *bd, Configuration *c, double step,
                  SimulationParameters sp) {
    sp.set_maximum_time_step(step);
    c->load_configuration();
    std::cout << "Trying step " << step << std::endl;
    std::vector<double> es;
    unsigned int ns=100;
    for (unsigned int i=0; i< ns; ++i) {
      es.push_back(bd->optimize(1));
    }
    return is_constant(es.begin(), es.end());
  }
};


IMPATOMEXPORT double get_maximum_time_step_estimate(BrownianDynamics *bd,
                                                    SimulationParameters sp){
  IMP_NEW(Configuration, c, (bd->get_model()));
  double ots= sp.get_maximum_time_step();
  double lb=10;
  while (is_ok_step(bd, c, lb, sp)) {
    lb*=2;
  }
  double ub=lb*2;
  for (unsigned int i=0; i < 5; ++i) {
    double cur= (ub+lb)*.5;
    if (is_ok_step(bd, c, cur, sp)) {
      lb=cur;
    } else {
      ub=cur;
    }
  }
  sp.set_maximum_time_step(ots);
  c->load_configuration();
  return lb;
}

IMPATOM_END_NAMESPACE
