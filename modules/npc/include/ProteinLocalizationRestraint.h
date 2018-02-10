/**
 *  \file IMP/npc/ProteinLocalizationRestraint.h
 *  \brief protein localization restraints
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types. It also handles multiple copies of the same particles.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H
#define IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H

#include <IMP/Pointer.h>
#include <IMP/Restraint.h>
#include <IMP/npc/npc_config.h>
#include <IMP/SingletonContainer.h>

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
public:
  ZAxialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  ZAxialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  ZAxialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  ZAxialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  ZAxialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  ZAxialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  YAxialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  YAxialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  YAxialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  YAxialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  YAxialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  YAxialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  XYRadialPositionRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);
  XYRadialPositionRestraint(Model *m,
      double lower_bound, double upper_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  XYRadialPositionLowerRestraint(Model *m, SingletonContainerAdaptor sc,
      double lower_bound, bool consider_radius, double sigma=1);
  XYRadialPositionLowerRestraint(Model *m,
      double lower_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  XYRadialPositionUpperRestraint(Model *m, SingletonContainerAdaptor sc,
      double upper_bound, bool consider_radius, double sigma=1);
  XYRadialPositionUpperRestraint(Model *m,
      double upper_bound, bool consider_radius, double sigma=1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  ProteinContactRestraint(Model *m, SingletonContainerAdaptor sc,
      double tolerance_factor, double sigma=0.1);
  ProteinContactRestraint(Model *m,
      double tolerance_factor, double sigma=0.1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  ProteinChainRestraint(Model *m, SingletonContainerAdaptor sc,
      double sigma=0.1);
  ProteinChainRestraint(Model *m,
      double sigma=0.1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

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
public:
  MembraneSurfaceLocationRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, double sigma=2);
  MembraneSurfaceLocationRestraint(Model *m,
      double R, double r, double thickness, double sigma=2);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(MembraneSurfaceLocationRestraint);;
};


//! Try to one set of particles localized on a membrane surface
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
public:
  MembraneSurfaceLocationConditionalRestraint(Model *m, SingletonContainerAdaptor sc1, SingletonContainerAdaptor sc2,
      double R, double r, double thickness, double sigma=2);
  MembraneSurfaceLocationConditionalRestraint(Model *m,
      double R, double r, double thickness, double sigma=2);

#ifndef IMP_DOXYGEN
  void add_particle1(Particle *p);
  void add_particle2(Particle *p);
  void add_particles1(const ParticlesTemp &ps);
  void add_particles2(const ParticlesTemp &ps);
  void set_particles1(const ParticlesTemp &ps);
  void set_particles2(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(MembraneSurfaceLocationConditionalRestraint);;
};


class IMPNPCEXPORT MembraneExclusionRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;
public:
  MembraneExclusionRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, double sigma=2);
  MembraneExclusionRestraint(Model *m,
      double R, double r, double thickness, double sigma=2);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(MembraneExclusionRestraint);;
};


class IMPNPCEXPORT PoreSideVolumeLocationRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;
  bool consider_radius_;
public:
  PoreSideVolumeLocationRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, bool consider_radius, double sigma=2);
  PoreSideVolumeLocationRestraint(Model *m,
      double R, double r, double thickness, bool consider_radius, double sigma=2);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(PoreSideVolumeLocationRestraint);;
};


class IMPNPCEXPORT PerinuclearVolumeLocationRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double R_;
  double r_;
  double sigma_;
  double thickness_;
  bool consider_radius_;
public:
  PerinuclearVolumeLocationRestraint(Model *m, SingletonContainerAdaptor sc,
      double R, double r, double thickness, bool consider_radius, double sigma=2);
  PerinuclearVolumeLocationRestraint(Model *m,
      double R, double r, double thickness, bool consider_radius, double sigma=2);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(PerinuclearVolumeLocationRestraint);;
};


class IMPNPCEXPORT AssemblySymmetryByDistanceRestraint : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;
public:
  AssemblySymmetryByDistanceRestraint(Model *m, SingletonContainerAdaptor sc,
      double sigma=4);
  AssemblySymmetryByDistanceRestraint(Model *m,
      double sigma=4);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(AssemblySymmetryByDistanceRestraint);;
};


class IMPNPCEXPORT AssemblySymmetryByDihedralRestraint
  : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;
public:
  AssemblySymmetryByDihedralRestraint(Model *m, SingletonContainerAdaptor sc,
      double sigma=0.1);
  AssemblySymmetryByDihedralRestraint(Model *m,
      double sigma=0.1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(AssemblySymmetryByDihedralRestraint);;
};


class IMPNPCEXPORT ProteinProximityRestraint
  : public Restraint
{
  IMP::PointerMember<IMP::SingletonContainer> sc_;
  double sigma_;
  double max_dist_;
public:
  ProteinProximityRestraint(Model *m, SingletonContainerAdaptor sc,
      double max_dist, double sigma=0.1);
  ProteinProximityRestraint(Model *m,
      double max_dist, double sigma=0.1);

#ifndef IMP_DOXYGEN
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
#endif

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;
  ModelObjectsTemp do_get_inputs() const;

  IMP_OBJECT_METHODS(ProteinProximityRestraint);;
};



IMPNPC_END_NAMESPACE

#endif /* IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H */
