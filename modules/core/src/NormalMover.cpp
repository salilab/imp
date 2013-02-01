/**
 *  \file NormalMover.cpp
 *  \brief A modifier which perturbs a point with a gaussian.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/NormalMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/random.h>
#include <IMP/macros.h>
#include <IMP/base/warning_macros.h>
#include <boost/random/normal_distribution.hpp>

IMPCORE_BEGIN_NAMESPACE

NormalMover::NormalMover(const ParticlesTemp &sc,
                         const FloatKeys &vars,
                         Float max):
  MoverBase(sc, vars, "NormalMover%1%")
{
  set_sigma(max);
}

NormalMover::NormalMover(const ParticlesTemp &sc,
                         Float max):
  MoverBase(sc, XYZ::get_xyz_keys(), "NormalMover%1%")
{
  set_sigma(max);
}


IMP_GCC_DISABLE_WARNING(-Wuninitialized)
void NormalMover::do_move(Float probability)
{
  boost::uniform_real<> rand(0,1);
  boost::normal_distribution<double> mrng(0, stddev_);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<double> >
                          sampler(random_number_generator, mrng);

  for (unsigned int i = 0;
       i < get_number_of_particles(); ++i) {
    if (rand(random_number_generator) > probability) continue;
    for (unsigned int j = 0; j < get_number_of_keys(); ++j) {
      Float c = get_value(i, j);
      Float r = sampler();
      // Check for NaN (x!=x when x==NaN) (can only use std::isnan with C99)
      IMP_INTERNAL_CHECK(!base::isnan(r), "Bad random");
      IMP_INTERNAL_CHECK(!base::isnan(c), "Bad stored");
      propose_value(i, j, c + r);
    }
  }
}
void NormalMover::do_show(std::ostream &out) const {
  out << "sigma " << stddev_ << std::endl;
}

IMPCORE_END_NAMESPACE
