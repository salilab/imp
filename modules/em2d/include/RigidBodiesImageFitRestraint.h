/**
 *  \file IMP/em2d/RigidBodiesImageFitRestraint.h
 *  \brief Fit kernel::Restraint
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_RIGID_BODIES_IMAGE_FIT_RESTRAINT_H
#define IMPEM2D_RIGID_BODIES_IMAGE_FIT_RESTRAINT_H

#include "IMP/em2d/project.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/Image.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/Restraint.h"
#include "IMP/base/Pointer.h"
#include "IMP/macros.h"
#include "IMP/base_types.h"

IMPEM2D_BEGIN_NAMESPACE

// Comparison function for the keys of the mapping between quaternion of
// rotation and their corresponding cached mask
class IntsOrder {
 public:
  // The vector with lower values goes first
  bool operator()(const Ints &a, const Ints &b) const {
    for (unsigned int i = 0; i < a.size(); ++i) {
      if (a[i] < b[i]) return true;
      if (a[i] > b[i]) return false;
    }
    return false;
  }
};

typedef std::pair<Ints, unsigned int> KeyIndexPair;
typedef std::map<Ints, unsigned int, IntsOrder> KeyIndexMap;
typedef std::vector<KeyIndexMap> KeyIndexMaps;

//! Fit rigid bodies to an image.
class IMPEM2DEXPORT RigidBodiesImageFitRestraint : public kernel::Restraint {
  virtual double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
      const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(RigidBodiesImageFitRestraint);

 protected:
  // Storage for all the masks of the rigid bodies
  std::vector<em2d::Images> rigid_bodies_masks_;
  KeyIndexMaps maps_;
  base::Pointer<ScoreFunction> score_function_;
  core::RigidBodies rigid_bodies_;
  base::Pointer<Image> image_;  // Image to used when scoring
  base::Pointer<Image> projection_;
  ProjectingParameters params_;
  bool params_set_;

  // Return the internal identity index for the rigid body
  unsigned int get_rigid_body_index(const core::RigidBody &rb) const;

 public:
  /*! Initialize the class
    \param[in] scf Score function used to compute the value of the restraint
    \param[in] rbs Rigid bodies that are restrained
    \param[in] img Image used to compute the restraint
  */
  RigidBodiesImageFitRestraint(ScoreFunction *scf, const core::RigidBodies &rbs,
                               Image *img);

  void set_projecting_parameters(const ProjectingParameters &p);

  /*! Set the possible rotations that a rigid body can have. A projection of
      the rigid body in all possible orientations is stored
  */
  void set_orientations(const core::RigidBody &rb,
                        const algebra::Rotation3Ds &rots);

  /*! Return the number of precomputed masks associated with the rigid body
      Throws an exception if the rigid body is not in the set used to construct
      the restraint
  */
  unsigned int get_number_of_masks(const core::RigidBody &rb) const;
};
IMP_OBJECTS(RigidBodiesImageFitRestraint, RigidBodiesImageFitRestraints);

// For a unique 4 ints ( 2 first decimal positions) for a rotation
IMPEM2DEXPORT Ints get_unique_index(const algebra::Rotation3D &rot);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_RIGID_BODIES_IMAGE_FIT_RESTRAINT_H */
