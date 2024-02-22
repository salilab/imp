/**
 *  \file IMP/npc/ProteinLocalizationRestraint.h
 *  \brief protein localization restraints
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types. It also handles multiple copies of the same particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H
#define IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H

#include <IMP/Pointer.h>
#include <IMP/Restraint.h>
#include <IMP/npc/npc_config.h>
#include <IMP/SingletonContainer.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPNPC_BEGIN_NAMESPACE

//! Restrain particles by their z coordinate
/** Each particle's z coordinate is harmonically restrained to lie between
    the given lower and upper bounds.
  */
class IMPNPCEXPORT ZAxialPositionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ZAxialPositionRestraint);

public:
  ZAxialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  ZAxialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  ZAxialPositionRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(ZAxialPositionRestraint);;
};

//! Restrain particles by their z coordinate
/** Each particle's z coordinate is harmonically restrained to lie above
    the given lower bound.
  */
class IMPNPCEXPORT ZAxialPositionLowerRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ZAxialPositionLowerRestraint);

public:
  ZAxialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  ZAxialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);
  ZAxialPositionLowerRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(ZAxialPositionLowerRestraint);;
};

//! Restrain particles by their z coordinate
/** Each particle's z coordinate is harmonically restrained to lie below
    the given upper bound.
  */
class IMPNPCEXPORT ZAxialPositionUpperRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ZAxialPositionUpperRestraint);

public:
  ZAxialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  ZAxialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);
  ZAxialPositionUpperRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(ZAxialPositionUpperRestraint);;
};


//! Restrain particles by their y coordinate
/** Each particle's y coordinate is harmonically restrained to lie between
    the given lower and upper bounds.
  */
class IMPNPCEXPORT YAxialPositionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(YAxialPositionRestraint);

public:
  YAxialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  YAxialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  YAxialPositionRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(YAxialPositionRestraint);;
};

//! Restrain particles by their y coordinate
/** Each particle's y coordinate is harmonically restrained to lie above
    the given lower bound.
  */
class IMPNPCEXPORT YAxialPositionLowerRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(YAxialPositionLowerRestraint);

public:
  YAxialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  YAxialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);
  YAxialPositionLowerRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(YAxialPositionLowerRestraint);;
};

//! Restrain particles by their y coordinate
/** Each particle's y coordinate is harmonically restrained to lie below
    the given upper bound.
  */
class IMPNPCEXPORT YAxialPositionUpperRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(YAxialPositionUpperRestraint);

public:
  YAxialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  YAxialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);
  YAxialPositionUpperRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(YAxialPositionUpperRestraint);;
};


//! Restrain particles by their distance from the z axis in the xy plane
/** Each particle's distance to the z axis is harmonically restrained to
    lie between the given lower and upper bounds.
  */
class IMPNPCEXPORT XYRadialPositionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(XYRadialPositionRestraint);

public:
  XYRadialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  XYRadialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  XYRadialPositionRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(XYRadialPositionRestraint);;
};

//! Restrain particles by their distance from the z axis in the xy plane
/** Each particle's distance to the z axis is harmonically restrained to
    lie above the given lower bound.
  */
class IMPNPCEXPORT XYRadialPositionLowerRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(XYRadialPositionLowerRestraint);

public:
  XYRadialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  XYRadialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);
  XYRadialPositionLowerRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(XYRadialPositionLowerRestraint);;
};

//! Restrain particles by their distance from the z axis in the xy plane
/** Each particle's distance to the z axis is harmonically restrained to
    lie below the given upper bound.
  */
class IMPNPCEXPORT XYRadialPositionUpperRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(XYRadialPositionUpperRestraint);

public:
  XYRadialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  XYRadialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);
  XYRadialPositionUpperRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(XYRadialPositionUpperRestraint);;
};


//! Try to keep all particle surfaces in contact
/** The particles (each of which should be a core::XYZR) are harmonically
    restrained such that each particle's sphere surface is in contact
    with every other surface.
  */
class IMPNPCEXPORT ProteinContactRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double tolerance_factor_;
  double sigma_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, tolerance_factor_, sigma_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ProteinContactRestraint);

public:
  ProteinContactRestraint(Model *m, SingletonContainerAdaptor sc,
      double tolerance_factor, double sigma=0.1);
  ProteinContactRestraint(Model *m,
      double tolerance_factor, double sigma=0.1);
  ProteinContactRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(ProteinContactRestraint);;
};


//! Try to keep particle surfaces in contact in a chain
/** The particles (each of which should be a core::XYZR) are harmonically
    restrained such that each particle's sphere surface is in contact
    with the surface of the previous particle.
  */
class IMPNPCEXPORT ProteinChainRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, sigma_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ProteinChainRestraint);

public:
  ProteinChainRestraint(Model *m, SingletonContainerAdaptor sc,
      double sigma=0.1);
  ProteinChainRestraint(Model *m,
      double sigma=0.1);
  ProteinChainRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(ProteinChainRestraint);;
};


//! Try to keep particles localized on a membrane surface
/** The membrane is defined in this case to be a half torus in the xy
    plane with center at the origin.
  */
class IMPNPCEXPORT MembraneSurfaceLocationRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, R_, r_, sigma_, thickness_);
  }
  IMP_OBJECT_SERIALIZE_DECL(MembraneSurfaceLocationRestraint);

public:
  MembraneSurfaceLocationRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, double sigma=2);
  MembraneSurfaceLocationRestraint(Model *m,
      double R, double r, double thickness, double sigma=2);
  MembraneSurfaceLocationRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(MembraneSurfaceLocationRestraint);;
};


//! Try to keep one set of particles localized on a membrane surface
/** The membrane is defined in this case to be a half torus in the xy
    plane with center at the origin. One of the two sets of particles
    (whichever is closer) is restrained to the membrane surface.
  */
class IMPNPCEXPORT MembraneSurfaceLocationConditionalRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc1_;
  IMP::PointerMember<IMP::SingletonContainer> sc2_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc1_, sc2_, R_, r_, sigma_, thickness_);
  }
  IMP_OBJECT_SERIALIZE_DECL(MembraneSurfaceLocationConditionalRestraint);

public:
  MembraneSurfaceLocationConditionalRestraint(Model *m, SingletonContainerAdaptor sc1, SingletonContainerAdaptor sc2,
      double R, double r, double thickness, double sigma=2);
  MembraneSurfaceLocationConditionalRestraint(Model *m,
      double R, double r, double thickness, double sigma=2);
  MembraneSurfaceLocationConditionalRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle1(Particle *p);
  void add_particle2(Particle *p);
  void add_particles1(const ParticlesTemp &ps);
  void add_particles2(const ParticlesTemp &ps);
  void set_particles1(const ParticlesTemp &ps);
  void set_particles2(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(MembraneSurfaceLocationConditionalRestraint);;
};


//! Try to keep particles away from a membrane
/** The membrane is defined in this case to be a half torus in the xy
    plane with center at the origin.
  */
class IMPNPCEXPORT MembraneExclusionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, R_, r_, sigma_, thickness_);
  }
  IMP_OBJECT_SERIALIZE_DECL(MembraneExclusionRestraint);

public:
  MembraneExclusionRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, double sigma=2);
  MembraneExclusionRestraint(Model *m,
      double R, double r, double thickness, double sigma=2);
  MembraneExclusionRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(MembraneExclusionRestraint);;
};


//! Try to keep particles on the pore side of a membrane
/** The membrane is defined in this case to be a half torus in the xy
    plane with center at the origin.
  */
class IMPNPCEXPORT PoreSideVolumeLocationRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, R_, r_, sigma_, thickness_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(PoreSideVolumeLocationRestraint);

public:
  PoreSideVolumeLocationRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, bool consider_radius, double sigma=2);
  PoreSideVolumeLocationRestraint(Model *m,
      double R, double r, double thickness, bool consider_radius, double sigma=2);
  PoreSideVolumeLocationRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(PoreSideVolumeLocationRestraint);;
};


//! Try to keep particles on the perinuclear side of a membrane
/** The membrane is defined in this case to be a half torus in the xy
    plane with center at the origin.
  */
class IMPNPCEXPORT PerinuclearVolumeLocationRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;
  bool consider_radius_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, R_, r_, sigma_, thickness_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(PerinuclearVolumeLocationRestraint);

public:
  PerinuclearVolumeLocationRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, bool consider_radius, double sigma=2);
  PerinuclearVolumeLocationRestraint(Model *m,
      double R, double r, double thickness, bool consider_radius, double sigma=2);
  PerinuclearVolumeLocationRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(PerinuclearVolumeLocationRestraint);;
};


//! Restrain two interparticle distances to be the same
/** This restraint must be given 4 particles. They are restrained to
    be symmetric, by enforcing a harmonic penalty on the difference
    between the distance between the first 2 particles and the distance
    between the last two.
  */
class IMPNPCEXPORT AssemblySymmetryByDistanceRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, sigma_);
  }
  IMP_OBJECT_SERIALIZE_DECL(AssemblySymmetryByDistanceRestraint);

public:
  AssemblySymmetryByDistanceRestraint(Model *m, SingletonContainerAdaptor sc,
      double sigma=4);
  AssemblySymmetryByDistanceRestraint(Model *m,
      double sigma=4);
  AssemblySymmetryByDistanceRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(AssemblySymmetryByDistanceRestraint);;
};


//! Restrain two interparticle dihedrals to be the same
/** This restraint must be given 8 particles. They are restrained to
    be symmetric, by enforcing a harmonic penalty on the difference
    between the dihedral angle between the first 4 particles and the dihedral
    between the last 4.
  */
class IMPNPCEXPORT AssemblySymmetryByDihedralRestraint
  : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, sigma_);
  }
  IMP_OBJECT_SERIALIZE_DECL(AssemblySymmetryByDihedralRestraint);

public:
  AssemblySymmetryByDihedralRestraint(Model *m, SingletonContainerAdaptor sc,
      double sigma=0.1);
  AssemblySymmetryByDihedralRestraint(Model *m,
      double sigma=0.1);
  AssemblySymmetryByDihedralRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(AssemblySymmetryByDihedralRestraint);;
};

//! Restrain a set of particles to be proximate to each other
/** Each particle is harmonically restrained to be no more than
    `max_dist` away from every other particle.
  */
class IMPNPCEXPORT ProteinProximityRestraint
  : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;
  double max_dist_;

  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this), sc_, sigma_, max_dist_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ProteinProximityRestraint);

public:
  ProteinProximityRestraint(Model *m, SingletonContainerAdaptor sc,
      double max_dist, double sigma=0.1);
  ProteinProximityRestraint(Model *m,
      double max_dist, double sigma=0.1);
  ProteinProximityRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;

  IMP_OBJECT_METHODS(ProteinProximityRestraint);;
};

//! Restrain particles by their x coordinate
/** Each particle's x coordinate is harmonically restrained to lie between
    the given lower and upper bounds.
  */
class IMPNPCEXPORT XAxialPositionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(XAxialPositionRestraint);
  
public:
  XAxialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  XAxialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  XAxialPositionRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;
  
    //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(XAxialPositionRestraint);;
};

//! Restrain particles by their x coordinate
/** Each particle's x coordinate is harmonically restrained to lie above
    the given lower bound.
  */
class IMPNPCEXPORT XAxialPositionLowerRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double lower_bound_;
  double sigma_;
  bool consider_radius_;
  
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, lower_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(XAxialPositionLowerRestraint);
  
public:
  XAxialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  XAxialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);
  XAxialPositionLowerRestraint() {}

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;
  
    //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(XAxialPositionLowerRestraint);;
};

//! Restrain particles by their x coordinate
/** Each particle's x coordinate is harmonically restrained to lie below
    the given upper bound.
  */
class IMPNPCEXPORT XAxialPositionUpperRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double upper_bound_;
  double sigma_;
  bool consider_radius_;
  
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, upper_bound_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(XAxialPositionUpperRestraint);
  
public:
  XAxialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  XAxialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);
  XAxialPositionUpperRestraint() {}

      
  

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;
  
    //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(XAxialPositionUpperRestraint);;
};

//! Restrain particle to a specific position
/** All distances are in Angstrom
While SphereDistanceToSingletonScore creates a score based on a UnaryFunction object,
OverallPositionRestraint assumes a harmonic restraint.
\param[in] x_start	x position to restrain to
\param[in] y_start	y position to restrain to
\param[in] z_start	z position to restrain to
\param[in] tolerance	range of distances where restraint=0
\param[in] consider_radius	bool, consider the radius of the particle
\param[in] sigma	inverse strength of harmonic potential
  */
class IMPNPCEXPORT OverallPositionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double x_start_;
  double y_start_;
  double z_start_;
  double tolerance_;
  double sigma_;
  bool consider_radius_;
  
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Restraint>(this),
       sc_, x_start_, y_start_, z_start_, tolerance_, sigma_, consider_radius_);
  }
  IMP_OBJECT_SERIALIZE_DECL(OverallPositionRestraint);
  
public:

  OverallPositionRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double x_start, double y_start, double z_start, double tolerance, bool consider_radius, double sigma=1);
  OverallPositionRestraint(Model *m,
    double x_start, double y_start, double z_start, double tolerance, bool consider_radius, double sigma=1);
  OverallPositionRestraint() {}


#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(
                  IMP::DerivativeAccumulator *accum) const override;
  ModelObjectsTemp do_get_inputs() const override;
  
    //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(OverallPositionRestraint);;
};




IMPNPC_END_NAMESPACE

#endif /* IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H */
