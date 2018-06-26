/**
 *  \file IMP/spb/TransformationSymmetry.h
 *  \brief Implement various types of symmetry
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_TRANSFORMATION_SYMMETRY_H
#define IMPSPB_TRANSFORMATION_SYMMETRY_H

#include <IMP/SingletonModifier.h>
#include <IMP/algebra/Reflection3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/singleton_macros.h>
#include "IMP/core/XYZ.h"
#include "IMP/core/rigid_bodies.h"
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

//! Set the coordinates of a particle to be a transformed version of a reference
/** The passed particles must be Reference particles and XYZ particles and
    can be RigidBody particles iff the reference particle is a rigid body.
    \see TransformationAndReflectionSymmetry
 */
class IMPSPBEXPORT TransformationSymmetry : public SingletonModifier {
  algebra::Transformation3D t_;
  IMP::PointerMember<Particle> px_;
  IMP::PointerMember<Particle> py_;
  IMP::PointerMember<Particle> pz_;

  algebra::Vector3D get_vector(algebra::Vector3D center) const;
  algebra::Transformation3D get_transformation() const;

 public:
  //! Create with both reflection and translation
  TransformationSymmetry(algebra::Transformation3D t, Particle *px,
                         Particle *py, Particle *pz);

  virtual void apply_index(IMP::Model *m,
                           IMP::ParticleIndex p) const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_outputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

  IMP_SINGLETON_MODIFIER_METHODS(TransformationSymmetry);

  IMP_OBJECT_METHODS(TransformationSymmetry);
  IMP_SHOWABLE(TransformationSymmetry);
};

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_TRANSFORMATION_SYMMETRY_H */
