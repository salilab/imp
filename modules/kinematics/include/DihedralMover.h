/**
 *  \file IMP/kinematics/DihedralMover.h
 *  \brief A modifier which perturbs a point with a normal distribution.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_DIHEDRAL_MOVER_H
#define IMPKINEMATICS_DIHEDRAL_MOVER_H

#include <IMP/kinematics/kinematics_config.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/kinematics/revolute_joints.h>

IMPKINEMATICS_BEGIN_NAMESPACE

//! Modify a set of joints using a normal distribution
/** The normal distribution has a zero mean and the given standard deviation.
    \see MonteCarlo
 */
class IMPKINEMATICSEXPORT DihedralMover : public core::MonteCarloMover {
    kinematics::RevoluteJoints joints_;
    Float max_rot_;
    Floats originals_;

 public:
  DihedralMover(Model *m,
          const kinematics::RevoluteJoints& joints,
          const double max_rot=0.1);

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DihedralMover);
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_DIHEDRAL_MOVER_H */
