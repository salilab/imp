/**
 *  \file IMP/multifit/RigidLeavesRefiner.h
 *  \brief Return all rigid body members that are also hierarchy leaves.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_RIGID_LEAVES_REFINER_H
#define IMPMULTIFIT_RIGID_LEAVES_REFINER_H

#include <IMP/Refiner.h>
#include <IMP/multifit/multifit_config.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Return all rigid body members that are also hierarchy leaves.
class IMPMULTIFITEXPORT RigidLeavesRefiner : public Refiner {
 public:
  RigidLeavesRefiner(std::string name = "RigidLeavesRefiner%d")
      : Refiner(name) {}
  virtual bool get_can_refine(Particle *) const IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual const ParticlesTemp get_refined(Particle *) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidLeavesRefiner);
};

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_RIGID_LEAVES_REFINER_H */
