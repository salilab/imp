/**
 *  \file IMP/isd/Weight.h
 *  \brief Add weights constrained to the unit simplex to a particle.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_WEIGHT_H
#define IMPISD_WEIGHT_H

#include "isd_config.h"
#include <IMP/algebra/UnitSimplexD.h>
#include <IMP/Particle.h>
#include <IMP/decorator_macros.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPISD_BEGIN_NAMESPACE

static const int IMPISD_MAX_WEIGHTS = 1000;

//! Add weights to a particle.
/** Weights are constrained to the unit simplex.

    \see algebra::UnitSimplexD
    \ingroup decorators
*/
class IMPISDEXPORT Weight : public Decorator {

  static void do_setup_particle(Model *m, ParticleIndex pi);

  static void do_setup_particle(Model *m, ParticleIndex pi, Int nweights);

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::VectorKD& w);

  //! Add unit simplex constraint.
  static void add_constraint(Model *m, ParticleIndex pi);

  static ObjectKey get_constraint_key();

 public:
  IMP_DECORATOR_METHODS(Weight, Decorator);

  //! Set up an empty Weight.
  /** Weights must be added with add_weight() before use. */
  IMP_DECORATOR_SETUP_0(Weight);

  //! Set up Weight with a fixed number of weights.
  /** All weights are initialized with the same value. */
  IMP_DECORATOR_SETUP_1(Weight, Int, nweights);

  //! Set up Weight from the provided weight vector.
  IMP_DECORATOR_SETUP_1(Weight, const algebra::VectorKD&, w);

  IMPISD_DEPRECATED_METHOD_DECL(2.12)
  static IntKey get_nstates_key();

  //! Get number of weights key
  static IntKey get_number_of_weights_key();

  //! Get ith weight key
  static FloatKey get_weight_key(int i);

  //! Get all weight keys
  FloatKeys get_weight_keys() const;

  //! Get the ith weight
  Float get_weight(int i) const;

  //! Get all weights
  algebra::VectorKD get_weights() const;

  //! Set the ith weight lazily.
  /** Delay enforcing the simplex constraint until Model::update(). */
  void set_weight_lazy(int i, Float wi);

  //! Set all the weights
  /** Delay enforcing the simplex constraint until Model::update(). */
  void set_weights_lazy(const algebra::VectorKD& w);

  //! Set all weights, enforcing the simplex constraint
  /** \see algebra::get_projected */
  void set_weights(const algebra::VectorKD& w);

  //! Get weights are optimized
  bool get_weights_are_optimized() const;

  //! Set weights are optimized
  void set_weights_are_optimized(bool tf);

  //! Get derivative wrt ith weight.
  Float get_weight_derivative(int i) const;

  //! Get derivatives wrt all weights.
  algebra::VectorKD get_weights_derivatives() const;

  //! Add to derivative wrt ith weight.
  void add_to_weight_derivative(int i, Float dwi,
                                const DerivativeAccumulator &da);

  //! Add to derivatives wrt all weights.
  void add_to_weights_derivatives(const algebra::VectorKD& dw,
                                  const DerivativeAccumulator &da);

  //! Extend the weight vector by one element lazily.
  /** This should only be called during set-up and cannot be called
      for an optimized Weight.
      This version delays enforcing the simplex constraint until
      Model::update().
   */
  void add_weight_lazy(Float wi = 0);

  //! Extend the weight vector by one element.
  /** This should only be called during set-up and cannot be called
      for an optimized Weight.
   */
  void add_weight(Float wi = 0);

  IMPISD_DEPRECATED_METHOD_DECL(2.12)
  Int get_number_of_states() const;

  //! Get number of weights.
  Int get_number_of_weights() const;

  //! Get unit simplex on which weight vector lies.
  algebra::UnitSimplexKD get_unit_simplex() const;

  //! Set number of weights lazily
  /** This should only be called during set-up and cannot be called
      for an optimized Weight. New weights are initialized to 0.
      This version delays enforcing the simplex constraint until
      Model::update().
   */
  void set_number_of_weights_lazy(Int nweights);

  //! Set number of weights.
  /** This should only be called during set-up and cannot be called
      for an optimized Weight. New weights are initialized to 0.
   */
  void set_number_of_weights(Int nweights);

  static bool get_is_setup(Model *m, ParticleIndex pi);
};

IMP_DECORATORS(Weight, Weights, Decorators);


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
class IMPISDEXPORT WeightSimplexConstraint : public IMP::Constraint {
  private:
    ParticleIndex pi_;

  private:
    WeightSimplexConstraint(Particle *p)
        : IMP::Constraint(p->get_model(), "WeightSimplexConstraint%1%")
        , pi_(p->get_index()) {}

  public:
    friend class Weight;
    virtual void do_update_attributes() IMP_OVERRIDE;
    virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE;
    virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
    virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
    IMP_OBJECT_METHODS(WeightSimplexConstraint);
};
#endif

IMPISD_END_NAMESPACE

#endif /* IMPISD_WEIGHT_H */
