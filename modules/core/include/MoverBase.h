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
  void do_propose_value(unsigned int i, unsigned int j, Float t);
 public:
  virtual void reset_move();

  /** This sets everything up and then calls the generate_move method.
   */
  virtual ParticlesTemp propose_move(Float f);

  ParticlesTemp get_output_particles() const;

 protected:
  unsigned int get_number_of_particles() const { return particles_.size(); }
  unsigned int get_number_of_keys() const { return keys_.size(); }
  std::string get_particle_name(unsigned int i) const;

  //! implement this method to propose a move
  /** See NormalMover for a simple example.
   */
  virtual void do_move(Float f) = 0;

  //! Get the value of a controlled attribute
  /** \param [in] i The index of the particle.
      \param [in] j The index of the attribute.
   */
  Float get_value(unsigned int i, unsigned int j) const;

  //! Propose a value
  /** \param[in] i The index of the particle.
      \param[in] j The index of the key
      \param[in] t The value to propose
   */
  void propose_value(unsigned int i, unsigned int j, Float t);

  IMPCORE_DEPRECATED_OBJECT_DECL(2.1)
  MoverBase(const ParticlesTemp &ps, const FloatKeys &keys, std::string name);
};

IMP_OBJECTS(MoverBase, MoverBases);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MOVER_BASE_H */
