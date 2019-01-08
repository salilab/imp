/**
 *  \file IMP/kinematics/RevoluteJointMover.h
 *  \brief A modifier which perturbs a point with a normal distribution.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_REVOLUTE_JOINT_MOVER_H
#define IMPKINEMATICS_REVOLUTE_JOINT_MOVER_H

#include <IMP/kinematics/kinematics_config.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/kinematics/revolute_joints.h>

IMPKINEMATICS_BEGIN_NAMESPACE

//! Modify a set of joints using a normal distribution
/** The normal distribution has a zero mean and the given standard deviation.
    \see MonteCarlo
 */
class IMPKINEMATICSEXPORT RevoluteJointMover : public core::MonteCarloMover {
    kinematics::RevoluteJoints joints_;
    Float stddev_;
    Floats originals_;

 public:
  RevoluteJointMover(Model *m,
          const kinematics::RevoluteJoints& joints,
          const double stddev=0.01);

  void set_sigma(Float sigma) {
    IMP_USAGE_CHECK(sigma > 0, "Sigma must be positive");
    stddev_ = sigma;
  }

  Float get_sigma() const { return stddev_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RevoluteJointMover);
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_REVOLUTE_JOINT_MOVER_H */
