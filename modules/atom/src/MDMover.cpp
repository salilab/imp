/**
 *  \file MDMover.cpp
 *  \brief A modifier which perturbs a set of coordinates doing a short MD.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/MDMover.h>
#include <IMP/random.h>
#include <IMP/core/XYZ.h>
#include <boost/random/normal_distribution.hpp>

IMPATOM_BEGIN_NAMESPACE

MDMover::MDMover(SingletonContainer *sc,
                         MolecularDynamics *md,
                         double temperature,
                         unsigned int nsteps): MoverBase(sc), md_(md),
                                    temperature_(temperature)
{
  add_float_keys(IMP::core::XYZ::get_xyz_keys());
  set_number_of_steps(nsteps);
}

void MDMover::do_move(Float probability)
{
  boost::uniform_real<> rand(0,1);
  if (rand(random_number_generator) < probability) {

    md_->assign_velocities(temperature_);
    md_->optimize(get_number_of_steps());

    for (unsigned int i = 0;
            i < get_container()->get_number_of_particles(); ++i) {
      for (unsigned int j = 0; j < get_number_of_float_keys(); ++j) {
        Float c = get_float(i, j);
        // Check for NaN (x!=x when x==NaN) (can only use std::isnan with C99)
        IMP_INTERNAL_CHECK(!is_nan(c), "Bad stored");
        propose_value(i, j, c);
      }
    }
  }
}

void MDMover::do_show(std::ostream &out) const {
  out << "nsteps " << nsteps_ << std::endl;
}

IMPATOM_END_NAMESPACE
