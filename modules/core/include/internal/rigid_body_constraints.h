/**
 *  \file rigid_body_constraints.h
 *  \brief Constraints to keep rigid bodies rigid
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_RIGID_BODY_CONSTRAINTS_H
#define IMPCORE_INTERNAL_RIGID_BODY_CONSTRAINTS_H

#include <IMP/core/core_config.h>
#include <IMP/singleton_macros.h>
#include <IMP/SingletonModifier.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/StaticListContainer.h>
#include <IMP/internal/ContainerConstraint.h>
#include <IMP/internal/TupleConstraint.h>
#include <IMP/Model.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class NullSDM : public SingletonModifier {
 public:
  NullSDM(std::string name = "NullModifier%1%") : SingletonModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *, const ParticleIndexes &) const override {
    return ModelObjectsTemp();
  }
  virtual ModelObjectsTemp do_get_outputs(
      Model *, const ParticleIndexes &) const override {
    return ModelObjectsTemp();
  }
  IMP_SINGLETON_MODIFIER_METHODS(NullSDM);
};

inline void NullSDM::apply_index(Model *, ParticleIndex) const {}

/** \brief Update global coordinates of all members of a given rigid body

    It is rarely necessary to use this modifier explicitly; one is
    automatically added for every rigid body that is created.
    It is applied before evaluate to keep the body rigid.
 */
class IMPCOREEXPORT _UpdateRigidBodyMembers : public SingletonModifier {
 public:
  _UpdateRigidBodyMembers(std::string name = "UpdateRigidBodyMembers%1%")
      : SingletonModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_SINGLETON_MODIFIER_METHODS(_UpdateRigidBodyMembers);
  IMP_OBJECT_METHODS(_UpdateRigidBodyMembers);
};

//! Accumulate the derivatives from the refined particles in the rigid body
/** It is rarely necessary to use this modifier explicitly; one is
    automatically added for every rigid body that is created.
    It is applied after evaluate to map derivatives from the body's rigid
    members back onto the body itself.
*/
class IMPCOREEXPORT _AccumulateRigidBodyDerivatives : public SingletonModifier {
 public:
  _AccumulateRigidBodyDerivatives(std::string name =
                                     "AccumulateRigidBodyDerivatives%1%")
      : SingletonModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_SINGLETON_MODIFIER_METHODS(_AccumulateRigidBodyDerivatives);
  IMP_OBJECT_METHODS(_AccumulateRigidBodyDerivatives);
};

/** \brief Normalize the rigid body's rotation quaternion
    It is rarely necessary to use this modifier explicitly; one is
    automatically added for every rigid body that is created.
 */
class IMPCOREEXPORT _NormalizeRotation : public SingletonModifier {
 public:
  _NormalizeRotation(std::string name = "NormalizeRotation%1%")
      : SingletonModifier(name) {}
  virtual void apply_index(Model *m, ParticleIndex pi) const
      override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual ModelObjectsTemp do_get_outputs(
      Model *m, const ParticleIndexes &pis) const override;
  virtual void apply_indexes(
      Model *m, const ParticleIndexes &pis, unsigned int lower_bound,
      unsigned int upper_bound) const override final;
  IMP_OBJECT_METHODS(_NormalizeRotation);
};

/** Normalize the rotation quaternions of all rigid bodies in the system */
class IMPCOREEXPORT _RigidBodyNormalizeConstraint
#ifdef SWIG
          : public IMP::Constraint
#else
          : public IMP::internal::ContainerConstraint<
               _NormalizeRotation, NullSDM,
               IMP::internal::StaticListContainer<SingletonContainer> >
#endif
{
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<
          IMP::internal::ContainerConstraint<
            _NormalizeRotation, NullSDM,
            IMP::internal::StaticListContainer<SingletonContainer> > >(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(_RigidBodyNormalizeConstraint);
public:
#ifndef SWIG
  _RigidBodyNormalizeConstraint(
       _NormalizeRotation *before, NullSDM *after,
       IMP::internal::StaticListContainer<SingletonContainer> *c,
       std::string name, bool can_skip=false)
 : IMP::internal::ContainerConstraint<_NormalizeRotation, NullSDM,
            IMP::internal::StaticListContainer<SingletonContainer> >(
                            before, after, c, name, can_skip) {}
#endif

  _RigidBodyNormalizeConstraint() {}

#ifdef SWIG
  // Expose base class methods to SWIG
  _NormalizeRotation *get_before_modifier() const;
  Container *get_container() const { return c_; }
#endif

  IMP_OBJECT_METHODS(_RigidBodyNormalizeConstraint);
};

/* Make a simple subclass rather than using
   IMP::internal::create_tuple_constraint(), so that we can serialize it */
class IMPCOREEXPORT _RigidBodyPositionConstraint
#ifdef SWIG
    : public IMP::Constraint
#else
    : public IMP::internal::TupleConstraint<_UpdateRigidBodyMembers,
                                            _AccumulateRigidBodyDerivatives>
#endif
{
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<
                    IMP::internal::TupleConstraint<_UpdateRigidBodyMembers,
                                      _AccumulateRigidBodyDerivatives> >(this));
  }
  IMP_OBJECT_SERIALIZE_DECL(_RigidBodyPositionConstraint);

public:
#ifndef SWIG
  _RigidBodyPositionConstraint(_UpdateRigidBodyMembers *before,
                               _AccumulateRigidBodyDerivatives *after,
                               Model *m, const ParticleIndex &vt,
                               std::string name, bool can_skip)
          : IMP::internal::TupleConstraint<
                _UpdateRigidBodyMembers, _AccumulateRigidBodyDerivatives>(
                                    before, after, m, vt, name, can_skip) {}
#endif

  _RigidBodyPositionConstraint() {}
  IMP_OBJECT_METHODS(_RigidBodyPositionConstraint);

#ifdef SWIG
  // Expose base class methods to SWIG
  _UpdateRigidBodyMembers *get_before_modifier() const;
  _AccumulateRigidBodyDerivatives *get_after_modifier() const;
  ParticleIndex get_index() const;
#endif
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_RIGID_BODY_CONSTRAINTS_H */
