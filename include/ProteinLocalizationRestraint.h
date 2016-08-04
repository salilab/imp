/**
 *  \file IMP/npc/ProteinLocalizationRestraint.h
 *  \brief protein localization restraints
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types. It also handles multiple copies of the same particles.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H
#define IMPNPC_PROTEIN_LOCALIZATION_RESTRAINT_H

#include <IMP/Pointer.h>
#include <IMP/Restraint.h>
#include <IMP/npc/npc_config.h>
#include <IMP/SingletonContainer.h>

IMPNPC_BEGIN_NAMESPACE

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
