/**
 *  \file IMP/core/NormalMover.h
 *  \brief A modifier which perturbs a point with a normal distribution.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_NORMAL_MOVER_H
#define IMPCORE_NORMAL_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarloMover.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/vector.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Modify a set of continuous variables using a normal distribution.
/** \see MonteCarlo
 */
class IMPCOREEXPORT NormalMover : public MonteCarloMover {
  ParticleIndexes pis_;
  FloatKeys keys_;
  Float stddev_;
  algebra::VectorKDs originals_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<MonteCarloMover>(this), pis_, keys_, stddev_);
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      // clear originals
      initialize(pis_, keys_, stddev_);
    }
  }
  IMP_OBJECT_SERIALIZE_DECL(NormalMover);

  void initialize(ParticleIndexes pis, FloatKeys keys, double radius);

 public:
  NormalMover(Model *m, ParticleIndex pi, const FloatKeys &vars,
              double stddev);
  //! Move the x,y,z coordinates
  NormalMover(Model *m, ParticleIndex pi, double stddev);
#ifndef IMP_DOXYGEN
  /**  \param[in] sc The set of particles to perturb.
       \param[in] vars The variables to use (normally the keys for x,y,z)
       \param[in] sigma The standard deviation to use.
   */
  NormalMover(const ParticlesTemp &sc, const FloatKeys &vars,
              Float sigma);

  NormalMover(const ParticlesTemp &sc, Float radius);
#endif
  NormalMover() {}

  void set_sigma(Float sigma) {
    IMP_USAGE_CHECK(sigma > 0, "Sigma must be positive");
    stddev_ = sigma;
  }

  Float get_sigma() const { return stddev_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const override;
  virtual MonteCarloMoverResult do_propose() override;
  virtual void do_reject() override;
  IMP_OBJECT_METHODS(NormalMover);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_NORMAL_MOVER_H */
