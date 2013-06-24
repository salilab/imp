/**
 *  \file IMP/display/particle_geometry.h
 *  \brief Represent an XYZR particle with a sphere
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_PARTICLE_GEOMETRY_H
#define IMPDISPLAY_PARTICLE_GEOMETRY_H

#include <IMP/display/display_config.h>
#include "geometry_macros.h"
#include "declare_Geometry.h"
#include "Colored.h"
#include <IMP/base/Pointer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonScore.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! A base class for geometry contained in particles
/** */
class IMPDISPLAYEXPORT SingletonGeometry : public Geometry {
  IMP::base::Pointer<Particle> p_;

 public:
  SingletonGeometry(Particle *p);

  bool get_has_color() const {
    return Geometry::get_has_color()
      || Colored::get_is_setup(p_);
  }

  Color get_color() const {
    if (Geometry::get_has_color()) {
      return Geometry::get_color();
    } else {
      return Colored(p_).get_color();
    }
  }

  Particle *get_particle() const { return p_; }

  virtual ~SingletonGeometry() {}
};

//! A base class for geometry from a set of particles
/**
 */
class IMPDISPLAYEXPORT SingletonsGeometry : public Geometry {
  IMP::base::OwnerPointer<SingletonContainer> sc_;

 public:
  SingletonsGeometry(SingletonContainer *pc, Color c);
  SingletonsGeometry(SingletonContainer *pc);

  SingletonContainer *get_container() const { return sc_; }

  virtual ~SingletonsGeometry() {}
};

//! A base class for geometry contained in particles
/** */
class IMPDISPLAYEXPORT PairGeometry : public Geometry {
  IMP::base::Pointer<Particle> p0_, p1_;

 public:
  PairGeometry(const ParticlePair &pp);

  bool get_has_color() const {
    return Geometry::get_has_color() || Colored::get_is_setup(p0_) ||
           Colored::get_is_setup(p1_);
  }

  Color get_color() const {
    if (Geometry::get_has_color()) {
      return Geometry::get_color();
    } else if (Colored::get_is_setup(p0_))
      return Colored(p0_).get_color();
    else
      return Colored(p1_).get_color();
  }

  ParticlePair get_particle_pair() const { return ParticlePair(p0_, p1_); }

  virtual ~PairGeometry() {}
};

//! A base class for geometry from a set of particles
/**
 */
class IMPDISPLAYEXPORT PairsGeometry : public Geometry {
  IMP::base::OwnerPointer<PairContainer> sc_;

 public:
  PairsGeometry(PairContainer *pc, Color c);
  PairsGeometry(PairContainer *pc);

  PairContainer *get_container() const { return sc_; }

  virtual ~PairsGeometry() {}
};

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_PARTICLE_GEOMETRY_H */
