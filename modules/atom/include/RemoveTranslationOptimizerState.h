/**
 *  \file IMP/atom/RemoveTranslationOptimizerState.h
 *  \brief Remove rigid translation during optimization.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H
#define IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>

IMPATOM_BEGIN_NAMESPACE

//! Removes rigid translation from the particles.
/** Recenters the bunch of particles by putting the very first particle
    at the origin and rigidly translating all the others.
 */
class IMPATOMEXPORT RemoveTranslationOptimizerState : public OptimizerState {
 public:
  RemoveTranslationOptimizerState(const Particles &pis, unsigned skip_steps);

  //! Set the number of update calls to skip between removals.
  void set_skip_steps(unsigned skip_steps) { skip_steps_ = skip_steps; }

  //! Get the number of update calls to skip between removals.
  unsigned int get_skip_steps() { return skip_steps_; }

  //! Set the particles to use.
  void set_particles(const Particles &pis) { pis_ = pis; }

  //! Remove translation now
  void remove_translation() const;

  virtual void update() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RemoveTranslationOptimizerState);

 private:
  Particles pis_;
  unsigned skip_steps_;
  unsigned call_number_;
};

IMP_OBJECTS(RemoveTranslationOptimizerState, RemoveTranslationOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H */
