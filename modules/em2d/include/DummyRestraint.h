/**
 *  \file IMP/em2d/DummyRestraint.h
 *  \brief
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_DUMMY_RESTRAINT_H
#define IMPEM2D_DUMMY_RESTRAINT_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/log.h"
#include <IMP/Restraint.h>
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
class IMPEM2DEXPORT DummyRestraint : public Restraint {
  Pointer<Particle> p0_;
  Pointer<Particle> p1_;

 public:
  /**
   * Inits the dummy restraint between the particles
   * @param p First particle
   * @param q Second particle
   */
  DummyRestraint(IMP::Particle *p, IMP::Particle *q)
      : Restraint(p->get_model(), "DummyRestraint%1%") {
    p0_ = p;
    p1_ = q;
  }

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
  void show(std::ostream &out = std::cout) const {
    out << "Dummy Restraint" << std::endl;
  }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DummyRestraint);
};
IMP_OBJECTS(DummyRestraint, DummyRestraints);

//! Dummy restraint for a set of particles. Same use as DummyRestraint
class IMPEM2DEXPORT ParticlesDummyRestraint : public Restraint {
  Pointer<SingletonContainer> container_;

 public:
  /**
   * Sets a dummy restraint for a set of particles
   * @param sc Must contain all the particles that are going to be restrained
   */
  ParticlesDummyRestraint(SingletonContainer *sc)
      : Restraint(sc->get_model(), "ParticlesDummyRestraint%1%") {
    container_ = sc;
  }

  /**
   * Shows information about the class
   * @param out Stream used to show the information
   */
  void show(std::ostream &out = std::cout) const {
    out << "ParticlesDummyRestraint" << std::endl;
  }

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ParticlesDummyRestraint);
};
IMP_OBJECTS(ParticlesDummyRestraint, ParticlesDummyRestraints);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_DUMMY_RESTRAINT_H */
