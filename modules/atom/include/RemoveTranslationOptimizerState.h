/**
 *  \file IMP/atom/RemoveTranslationOptimizerState.h
 *  \brief Remove rigid translation during optimization.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H
#define IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/Particle.h>
#include <IMP/base_types.h>
#include <IMP/OptimizerState.h>

IMPATOM_BEGIN_NAMESPACE

//! Removes rigid translation from the particles.
/** Recenters the bunch of particles by putting the very first particle
    at the origin and rigidly translating all the others.
 */
class IMPATOMEXPORT RemoveTranslationOptimizerState : public OptimizerState {
 public:
  RemoveTranslationOptimizerState(Model *m,
                                  ParticleIndexesAdaptor pis);

  //! Set the particles to use.
  void set_particles(const Particles &pis) { pis_ = pis; }

  //! Remove translation now
  void remove_translation() const;

  IMP_OBJECT_METHODS(RemoveTranslationOptimizerState);

 protected:
  virtual void do_update(unsigned int call) IMP_OVERRIDE;

 private:
  Particles pis_;
};

IMP_OBJECTS(RemoveTranslationOptimizerState, RemoveTranslationOptimizerStates);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REMOVE_TRANSLATION_OPTIMIZER_STATE_H */
