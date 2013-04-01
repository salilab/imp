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

namespace {
  std::string get_normal_mover_name(Model *m, ParticleIndex pi) {
    return "NormalMover-" + m->get_particle(pi)->get_name();
  }
}


void NormalMover::initialize(ParticleIndexes pis,
                           FloatKeys keys,
                           double radius) {
  pis_ = pis;
  keys_ = keys;
  stddev_ = radius;
  originals_.resize(pis.size());
}

NormalMover::NormalMover(Model *m,
                     ParticleIndex pi,
                     const FloatKeys &keys,
                     double radius):
  MonteCarloMover(m, get_normal_mover_name(m, pi)) {
  initialize(ParticleIndexes(1, pi), keys, radius);
}

NormalMover::NormalMover(Model *m,
                     ParticleIndex pi,
                     double radius):
  MonteCarloMover(m, get_normal_mover_name(m, pi)) {
  initialize(ParticleIndexes(1, pi), XYZ::get_xyz_keys(), radius);
}

// backwards compat
NormalMover::NormalMover(const ParticlesTemp &sc,
                     const FloatKeys &vars,
                     double max):
  MonteCarloMover(sc[0]->get_model(), "NormalMover%1%")
{
  initialize(kernel::get_indexes(sc),
             vars, max);
}

// backwards compat
NormalMover::NormalMover(const ParticlesTemp &sc,
                     double max):
  MonteCarloMover(sc[0]->get_model(), "XYZNormalMover%1%")
{
  initialize(kernel::get_indexes(sc),
             XYZ::get_xyz_keys(), max);
}

IMP_GCC_DISABLE_WARNING(-Wuninitialized)
MonteCarloMoverResult NormalMover::do_propose() {
  IMP_OBJECT_LOG;
  boost::uniform_real<> rand(0,1);
  boost::normal_distribution<double> mrng(0, stddev_);
  boost::variate_generator<RandomNumberGenerator&,
                           boost::normal_distribution<double> >
                          sampler(random_number_generator, mrng);

  for (unsigned int i = 0; i < pis_.size(); ++i) {
    for (unsigned int j = 0; j < keys_.size(); ++j) {
      originals_[i][j] = get_model()->get_attribute(keys_[j], pis_[i]);
    }
    for (unsigned int j = 0; j < keys_.size(); ++j) {
      get_model()->set_attribute(keys_[j], pis_[i],
                                 originals_[i][j] + sampler());
    }
  }
  return MonteCarloMoverResult(pis_, 1.0);
}

void NormalMover::do_reject() {
  IMP_OBJECT_LOG;
  for (unsigned int i = 0; i < pis_.size(); ++i) {
    for (unsigned int j = 0; j < keys_.size(); ++j) {
      get_model()->set_attribute(keys_[j], pis_[i], originals_[i][j]);
    }
  }
}

kernel::ModelObjectsTemp NormalMover::do_get_inputs() const {
  kernel::ModelObjectsTemp ret(pis_.size());
  for (unsigned int i=0; i< pis_.size(); ++i) {
    ret[i] = get_model()->get_particle(pis_[i]);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
