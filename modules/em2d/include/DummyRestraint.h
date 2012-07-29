/**
 *  \file DummyRestraint.h
 *  \brief
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_DUMMY_RESTRAINT_H
#define IMPEM2D_DUMMY_RESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/log.h"
#include <IMP/Restraint.h>
#include "IMP/restraint_macros.h"
#include "IMP/Pointer.h"


#include <IMP/Restraint.h>
#include <IMP/log.h>
#include <IMP/SingletonContainer.h>


IMPEM2D_BEGIN_NAMESPACE


//! Dummy restraint between two particles
/*!
    The restraint does do anything and returns 0 as a value. Use this
    restraint to force two particles to be considered by DOMINO as belonging
    to a restraint. This trick can be useful when building the merge tree for
    DOMINO, as can add branches that consider pairs of particles before entire
    subsets.
    NOTE: Using this restraint for DOMINO requires a fairly good knowledge of
        the works of merge trees.
*/

class IMPEM2DEXPORT DummyRestraint: public Restraint {
  Pointer<Particle> p0_;
  Pointer<Particle> p1_;

public:

  DummyRestraint(IMP::Particle *p, IMP::Particle *q) {
    p0_ = p;
    p1_ = q;
  }

  /**
   * Show information
   * @param out
   */
  void show(std::ostream &out = std::cout) const {
    out << "Dummy Restraint" << std::endl;
  }

  IMP_RESTRAINT(DummyRestraint);

};
IMP_OBJECTS(DummyRestraint,DummyRestraints);



//! Dummy restraint for a set of particles. Same use as DummyRestraint
/*!
  \param[in]
*/

class IMPEM2DEXPORT ParticlesDummyRestraint: public Restraint {
  Pointer<SingletonContainer> container_;

public:

  ParticlesDummyRestraint(SingletonContainer *sc) {
    container_ = sc;
  }


  /**
   * Show information
   * @param out
   */
  void show(std::ostream &out = std::cout) const {
    out << "Particles Restraint" << std::endl;
  }

  IMP_RESTRAINT(ParticlesDummyRestraint);

};
IMP_OBJECTS(ParticlesDummyRestraint, ParticlesDummyRestraints);


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_DUMMY_RESTRAINT_H */
