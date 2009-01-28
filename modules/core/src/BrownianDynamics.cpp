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
#include <IMP/internal/constants.h>
#include <IMP/internal/units.h>
#include <boost/random/normal_distribution.hpp>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

typedef
unit::Shift<unit::Multiply<unit::Pascal,
                           unit::Second>::type,
            -3>::type MillipascalSecond;

static MillipascalSecond eta(unit::Kelvin T)
{
  const std::pair<unit::Kelvin, MillipascalSecond> points[]
    ={ std::make_pair(unit::Kelvin(273+10.0),
                      MillipascalSecond(1.308)),
       std::make_pair(unit::Kelvin(273+20.0),
                      MillipascalSecond(1.003)),
       std::make_pair(unit::Kelvin(273+30.0),
                      MillipascalSecond(0.7978)),
       std::make_pair(unit::Kelvin(273+40.0),
                      MillipascalSecond(0.6531)),
       std::make_pair(unit::Kelvin(273+50.0),
                      MillipascalSecond(0.5471)),
       std::make_pair(unit::Kelvin(273+60.0),
                      MillipascalSecond(0.4668)),
       std::make_pair(unit::Kelvin(273+70.0),
                      MillipascalSecond(0.4044)),
       std::make_pair(unit::Kelvin(273+80.0),
                      MillipascalSecond(0.3550)),
       std::make_pair(unit::Kelvin(273+90.0),
                      MillipascalSecond(0.3150)),
       std::make_pair(unit::Kelvin(273+100.0),
                      MillipascalSecond(0.2822))};

  const unsigned int npoints= sizeof(points)/sizeof(std::pair<float,float>);
  if (T < points[0].first) {
    return points[0].second;
  } else {
    for (unsigned int i=1; i< npoints; ++i) {
      if (points[i].first > T) {
        float f= ((T - points[i-1].first)
                  /(points[i].first - points[i-1].first))
          .get_normalized_value();
        MillipascalSecond ret=
          (1.0-f) *points[i-1].second + f*points[i].second;
        return ret;
      }
    }
  }
  return points[npoints-1].second;
}

BrownianDynamics::BrownianDynamics(FloatKey dkey) :
  max_change_(10), max_dt_(1e7), cur_dt_(max_dt_), cur_time_(0),
  T_(IMP::internal::DEFAULT_TEMPERATURE), dkey_(dkey)
{
  IMP_check(dkey_ != FloatKey(), "BrownianDynamics needs a valid key for the "
            << "diffusion coefficient",
            ValueException);
}


BrownianDynamics::~BrownianDynamics()
{
}

IMP_LIST_IMPL(BrownianDynamics, Particle, particle, Particle*,,,);


void BrownianDynamics::set_time_step(unit::Femtosecond t)
{
  time_steps_.clear();
  max_dt_ = t;
  cur_dt_= max_dt_;
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
        && p->has_attribute(dkey_) && !p->get_is_optimized(dkey_)) {
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


bool BrownianDynamics::step()
{
  std::vector<algebra::Vector3D> new_pos(get_number_of_particles());
  bool noshrink=true;
  while (!propose_step(new_pos)) {
    cur_dt_= cur_dt_/2.0;
    noshrink=false;
  }

  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    XYZDecorator d(get_particle(i));
    for (unsigned int j=0; j< 3; ++j) {
      d.set_coordinate(j, new_pos[i][j]);
    }
  }
  return noshrink;
}


//! Perform a single dynamics step.
bool BrownianDynamics::propose_step(std::vector<algebra::Vector3D>&
                                    new_pos)
{
  FloatKeys xyzk=XYZDecorator::get_xyz_keys();

  IMP_LOG(VERBOSE, "dt is " << cur_dt_ << std::endl);


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
    unit::SquareCentimeterPerSecond D(p->get_value(dkey_));
    IMP_check(D.get_value() > 0
              && D.get_value() < std::numeric_limits<Float>::max(),
              "Bad diffusion coefficient on particle " << p->get_name(),
              ValueException);
    unit::Angstrom sigma(compute_sigma_from_D(D));
    IMP_IF_CHECK(EXPENSIVE) {
      unit::Angstrom osigma(sqrt(2.0*D*cur_dt_));
      IMP_check(sigma - osigma
                 <= .01* sigma,
                 "Sigma computations don't match " << sigma
                 << " "
                << sqrt(2.0*D*cur_dt_),
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
        force( -d.get_coordinate_derivative(j));
      unit::Femtonewton nforce
        = unit::convert_Cal_to_J(force/unit::ATOMS_PER_MOL);
      unit::Angstrom R(sampler());
      unit::Angstrom force_term(nforce*cur_dt_*D/kt());
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
            << "[" << d.get_coordinate_derivative(0)
            << ", " << d.get_coordinate_derivative(1)
            << ", " << d.get_coordinate_derivative(2) << "]" << std::endl);

    unit::SquareAngstrom t= square(delta[0]);

    unit::SquareAngstrom magnitude2=square(delta[0])+square(delta[1])
      +square(delta[2]);

    //unit::SquaredAngstrom max_motion2= square(max_motion);
    if (magnitude2 > square(max_change_)) {
      return false;
    }
    for (unsigned int j=0; j< 3; ++j) {
      new_pos[i][j]= d.get_coordinate(j) + delta[j].get_value();
    }
  }
  return true;
}


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
          << " particles with a step of " << cur_dt_ << std::endl);
  setup_particles();
  for (unsigned int i = 0; i < max_steps; ++i) {
    take_step();
  }
  return get_model()->evaluate(false);
}


void BrownianDynamics::take_step() {
  update_states();
  get_model()->evaluate(true);
  if (step()) ++ num_const_dt_;

  cur_time_= cur_time_+cur_dt_;
  if (num_const_dt_ == 10) {
    num_const_dt_=0;
    cur_dt_= std::min(cur_dt_*2.0, max_dt_);
  }
  {
    unit::Femtosecond lb(max_dt_/2.0);
    std::vector<int>::size_type d=0;
    while (lb > cur_dt_) {
      lb = lb/2.0;
      ++d;
    }
    time_steps_.resize(std::max(time_steps_.size(), d+1), 0);
    ++time_steps_[d];
  }
}

void BrownianDynamics::simulate(float max_time)
{
  IMP_check(get_model() != NULL, "Must set model before calling simulate",
            ValueException);
  setup_particles();
  unit::Femtosecond mt(max_time);
  num_const_dt_=0;
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

  CubicAngstrom v((m/p)*(4.0/(3.0*IMP::internal::PI)));

  return unit::Angstrom(std::pow(v.get_value(), .3333));
}

unit::SquareCentimeterPerSecond
BrownianDynamics::estimate_D_from_radius(unit::Angstrom r) const
{
  MillipascalSecond e=eta(T_);
  //unit::MKSUnit<-13, 0, 1, 0, -1> etar( e*r);
  /*std::cout << e << " " << etar << " " << kt << std::endl;
  std::cout << "scalar etar " << (unit::Scalar(6*unit::PI)*etar)
            << std::endl;
  std::cout << "ret pre conv " << (kt/(unit::Scalar(6* unit::PI)*etar))
  << std::endl;*/
  unit::SquareCentimeterPerSecond ret(kt()/(6.0* IMP::internal::PI*e*r));
  //std::cout << "ret " << ret << std::endl;
  return ret;
}

unit::Angstrom
BrownianDynamics
::compute_sigma_from_D(unit::SquareCentimeterPerSecond D) const
{
  return sqrt(2.0*D*cur_dt_); //6*xi*kb*T_;
}

unit::KilocaloriePerAngstromPerMol BrownianDynamics
::get_force_scale_from_D(unit::SquareCentimeterPerSecond D) const
{
  // force motion is f*cur_dt_*D/kT
  // sigma*kT/(cur_dt_*D)
  unit::Angstrom s=compute_sigma_from_D(D);
  unit::Piconewton pn= s*kt()/(max_dt_*D);
  unit::YoctoKilocaloriePerAngstrom yc=unit::convert_J_to_Cal(pn);
  return unit::operator*(unit::ATOMS_PER_MOL,yc);
}

IMPCORE_END_NAMESPACE
