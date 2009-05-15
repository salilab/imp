/**
 *  \file misc/BondBondPairScore.h
 *  \brief Score on the distance between a pair of bonds.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_BOND_BOND_PAIR_SCORE_H
#define IMPMISC_BOND_BOND_PAIR_SCORE_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/PairScore.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/UnaryFunction.h>
#include <utility>

IMPMISC_BEGIN_NAMESPACE

//! Apply a function to the distance between two bonds.
/** A UnaryFunction is applied to the distance between the edges
    defined by two bonds. If the volume_key is non-default, then
    the bond is given a width to define a cylinder of that volume
    and the distance between the cylinders is used.

    \ingroup bond
    \see Bond
 */
class IMPMISCEXPORT BondBondPairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
  FloatKey volume_;

  Float get_offset(Particle *p, Float d) const;
 public:
  //! Score based on f applied to the distance
  BondBondPairScore(UnaryFunction *f,
                            FloatKey volume= FloatKey());
  virtual ~BondBondPairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
  VersionInfo get_version_info() const {
    return internal::version_info;
  }

};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_BOND_BOND_PAIR_SCORE_H */
