/**
 *  \file atom/BondBondPairScore.h
 *  \brief Score on the distance between a pair of bonds.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_BOND_PAIR_SCORE_H
#define IMPATOM_BOND_BOND_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/UnaryFunction.h>
#include <utility>

IMPATOM_BEGIN_NAMESPACE

// exposed for testing
struct Segment {
  algebra::Vector3D first, second;
  Segment(const algebra::Vector3D &f,
          const algebra::Vector3D &s): first(f), second(s){}
};

// for testing
IMPATOMEXPORT
Segment shortest_segment(const Segment &s,
                         const algebra::Vector3D &p);

// for testing
IMPATOMEXPORT
Segment shortest_segment(const Segment &sa,
                        const Segment &sb);

//! Apply a function to the distance between two bonds.
/** A UnaryFunction is applied to the distance between the edges
    defined by two bonds. If the volume_key is non-default, then
    the bond is given a width to define a cylinder of that volume
    and the distance between the cylinders is used.

    \ingroup bond
    \see BondDecorator
 */
class IMPATOMEXPORT BondBondPairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
  FloatKey volume_;

  Segment get_endpoints(Particle *p) const;
  Float get_offset(Particle *p, Float d) const;
  algebra::Vector3D get_difference(const Segment &s0,
                                   const Segment &s1) const;
 public:
  //! Score based on f applied to the distance
  BondBondPairScore(UnaryFunction *f,
                            FloatKey volume= FloatKey());
  virtual ~BondBondPairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_BOND_PAIR_SCORE_H */
