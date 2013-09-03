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
#include <IMP/kernel/Particle.h>
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
   /** \deprecated_at{2.1} Use set_period() instead. */
  IMPATOM_DEPRECATED_FUNCTION_DECL(2.1)
  RemoveTranslationOptimizerState(const kernel::Particles &pis, unsigned skip_steps);
  RemoveTranslationOptimizerState(kernel::Model *m, kernel::ParticleIndexesAdaptor pis);

  //! Set the particles to use.
  void set_particles(const kernel::Particles &pis) { pis_ = pis; }

  //! Remove translation now
  void remove_translation() const;

  IMP_OBJECT_METHODS(RemoveTranslationOptimizerState);
 protected:
   virtual void do_update(unsigned int call) IMP_OVERRIDE;
 private:
  kernel::Particles pis_;};

IMP_OBJECTS(RemoveTranslationOptimizerState, RemoveTranslationOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H */
