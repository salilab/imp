/**
 *  \file IMP/isd/Nuisance.h
 *  \brief A decorator for nuisance parameters particles
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_NUISANCE_H
#define IMPISD_NUISANCE_H

#include <IMP/isd/isd_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/macros.h>
#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

//! Add nuisance parameter to particle
/** The value of the nuisance parameter may express data
    or theory uncertainty. It can be initialized with or without
    specifying its value. Default is 1. On construction, the Nuisance is
    unbounded. It can be bounded with set_upper and set_lower. Setting it
    to values outside of bounds results in setting it to the bound value.
 */
class IMPISDEXPORT Nuisance: public Decorator
{
public:
    IMP_DECORATOR(Nuisance, Decorator);

  static Nuisance setup_particle(Particle *p, double nuisance=1.0);

  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_nuisance_key());
  }

  static FloatKey get_nuisance_key();
  Float get_nuisance() const {
    return get_particle()->get_value(get_nuisance_key());
  }
  void set_nuisance(Float d);

  /** set upper and lower bound of nuisance by specifying
   * either a float or another nuisance. Both can be set at the same
   * time in which case the upper bound is the minimum of the two values.
   * This constraint is enforced with the help of a ScoreState that will be
   * created on-the-fly.
   */
  bool has_lower() const;
  Float get_lower() const;
  static FloatKey get_lower_key();
  static ParticleIndexKey get_lower_particle_key();
  void set_lower(Float d);
  void set_lower(Particle * d);
  void remove_lower();

  bool has_upper() const;
  Float get_upper() const;
  static FloatKey get_upper_key();
  static ParticleIndexKey get_upper_particle_key();
  void set_upper(Float d);
  void set_upper(Particle * d);
  void remove_upper();

  Float get_nuisance_derivative() const {
    return get_particle()->get_derivative(get_nuisance_key());
  }

  void add_to_nuisance_derivative(Float d, DerivativeAccumulator &accum) {
    get_particle()->add_to_derivative(get_nuisance_key(), d, accum);
  }

  bool get_nuisance_is_optimized() const {
      return get_particle()->get_is_optimized(get_nuisance_key());
  }

  void set_nuisance_is_optimized(bool val) {
      get_particle()->set_is_optimized(get_nuisance_key(), val);
  }

  friend class NuisanceScoreState;

private:
  //scorestate-related bookkeeping
  static ObjectKey get_ss_key();
  void enforce_bounds();
  void remove_bounds();

};

IMP_DECORATORS(Nuisance, Nuisances, ParticlesTemp);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class IMPISDEXPORT NuisanceScoreState : public ScoreState
{
    private:
        IMP::WeakPointer<Particle> p_;

    private:
        NuisanceScoreState(Particle *p) : p_(p) {}

    public:
        //only the GPIR can create this and add it to the model
        friend class Nuisance;
        IMP_SCORE_STATE(NuisanceScoreState);
};
#endif


IMPISD_END_NAMESPACE

#endif  /* IMPISD_NUISANCE_H */
