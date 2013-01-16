/**
 *  \file IMP/PairScore.h
 *  \brief Define PairScore.
 *
 *  WARNING This file was generated from NAMEScore.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_PAIR_SCORE_H
#define IMPKERNEL_PAIR_SCORE_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "ParticleTuple.h"
#include "DerivativeAccumulator.h"
#include "internal/container_helpers.h"
#include <IMP/base/utility_macros.h>
#include "input_output_macros.h"

IMP_BEGIN_NAMESPACE

//! Abstract score function
/** PairScores will evaluate the score and derivatives
    for the passed particles. Use in conjunction with various
    restraints such as IMP::core::PairsRestraint or
    IMP::core::PairRestraint.

    Implementers should check out IMP_PAIR_SCORE().
*/
class IMPEXPORT PairScore : public base::Object
{
 public:
  typedef ParticlePair Argument;
  typedef ParticleIndexPair IndexArgument;
  typedef const ParticlePair& PassArgument;
  typedef const ParticleIndexPair& PassIndexArgument;
  typedef PairModifier Modifier;
  PairScore(std::string name="PairScore %1%");
  //! Compute the score and the derivative if needed.
  IMP_DEPRECATED_WARN
    virtual double evaluate(const ParticlePair& vt,
                            DerivativeAccumulator *da) const;

  //! Compute the score and the derivative if needed.
  virtual double evaluate_index(Model *m, const ParticleIndexPair& vt,
                                DerivativeAccumulator *da) const;

  /** Implementations
      for these are provided by the IMP_PAIR_SCORE()
      macro.
  */
  virtual double evaluate_indexes(Model *m,
                                  const ParticleIndexPairs &o,
                                  DerivativeAccumulator *da,
                                  unsigned int lower_bound,
                                  unsigned int upper_bound) const ;

  //! Compute the score and the derivative if needed.
  virtual double evaluate_if_good_index(Model *m,
                                        const ParticleIndexPair& vt,
                                        DerivativeAccumulator *da,
                                        double max)  const;

  /** Implementations
      for these are provided by the IMP_PAIR_SCORE()
      macro.
  */
  virtual double evaluate_if_good_indexes(Model *m,
                                          const ParticleIndexPairs &o,
                                          DerivativeAccumulator *da,
                                          double max,
                                          unsigned int lower_bound,
                                          unsigned int upper_bound)
    const;
  /** Decompose this pair score acting on the pair into a set of
      restraints. The scoring function and derivatives should
      be equal to the current score. The defualt implementation
      just returns this object bound to the pair.*/
  Restraints create_current_decomposition(Model *m,
                                          const ParticleIndexPair& vt) const;

  IMP_INPUTS_DECL(PairScore);

protected:
  /** Overide this to return your own decomposition.*/
  virtual Restraints do_create_current_decomposition
                       (Model *m, const ParticleIndexPair& vt)const;

  IMP_REF_COUNTED_DESTRUCTOR(PairScore);
};

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_PAIR_SCORE_H */
