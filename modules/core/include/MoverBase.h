/**
 *  \file IMP/core/MoverBase.h    \brief Backwards compatibility.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MOVER_BASE_H
#define IMPCORE_MOVER_BASE_H

#include <IMP/core/core_config.h>
#include "Mover.h"

#include <IMP/internal/container_helpers.h>
#include <IMP/macros.h>

#include <vector>

IMPCORE_DEPRECATED_HEADER(2.1, "Use MonteCarloMover");

IMPCORE_BEGIN_NAMESPACE

/** \deprecated_at{2.1} Use IMP::core::MonteCarloMover instead
 */
class IMPCOREEXPORT MoverBase : public Mover {
  base::Vector<Floats> values_;
  base::Vector<FloatKey> keys_;
  ParticleIndexes particles_;
  void do_propose_value(unsigned int i, unsigned int j, Float t) {
    IMP_USAGE_CHECK(j < keys_.size(), "Out of range key");
    IMP_USAGE_CHECK(i < particles_.size(), "Out of range particle");
    if (get_model()->get_is_optimized(keys_[j], particles_[i])) {
      get_model()->set_attribute(keys_[j], particles_[i], t);
      IMP_USAGE_CHECK_FLOAT_EQUAL(
          get_model()->get_attribute(keys_[j], particles_[i]), t,
          "Tried to set, but it didn't work.");
    } else {
      IMP_LOG_TERSE("Dropping change to unoptimized attribute: "
                    << keys_[j] << " of particle "
                    << get_model()->get_particle(particles_[i])->get_name()
                    << std::endl);
    }
  }

 public:
  virtual void reset_move();

  /** This sets everything up and then calls the generate_move method.
   */
  virtual ParticlesTemp propose_move(Float f);

  ParticlesTemp get_output_particles() const {
    return IMP::internal::get_particle(get_model(), particles_);
  }

 protected:
  unsigned int get_number_of_particles() const { return particles_.size(); }
  unsigned int get_number_of_keys() const { return keys_.size(); }
  std::string get_particle_name(unsigned int i) const {
    return get_model()->get_particle(particles_[i])->get_name();
  }

  //! implement this method to propose a move
  /** See NormalMover for a simple example.
   */
  virtual void do_move(Float f) = 0;

  //! Get the value of a controlled attribute
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Float get_value(unsigned int i, unsigned int j) const {
    IMP_USAGE_CHECK(j < keys_.size(), "Out of range key");
    IMP_USAGE_CHECK(i < particles_.size(), "Out of range particle");
    return get_model()->get_attribute(keys_[j], particles_[i]);
  }

  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Float t) {
    do_propose_value(i, j, t);
  }

  IMPCORE_DEPRECATED_CLASS_DECL(2.1)
    MoverBase(const ParticlesTemp &ps, const FloatKeys &keys, std::string name);
};

IMP_OBJECTS(MoverBase, MoverBases);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MOVER_BASE_H */
