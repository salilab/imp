/**
 *  \file IMP/atom/Resolution.h     \brief A decorator for Resolutions.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_RESOLUTION_H
#define IMPATOM_RESOLUTION_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Hierarchy.h"

#include <IMP/base_types.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

IMPATOMEXPORT extern const double ALL_RESOLUTIONS;

/** Eventually, other types of representation will be supported, eg gaussians or
 * density maps. */
enum RepresentationType {
  BALLS = 0
};

//! A decorator for a resolution.
/** It stores a number of copies of its sub hierarchy each with an associated
 * resolution. You can used it to get the representation at a given resolution.
 *
 * \note Only one Resolution node is allowed in any path up the tree as nesting
 * them does not have a clear meaning.
 */
class IMPATOMEXPORT Resolution : public Hierarchy {
  static IntsKey get_types_key();
  static ParticleIndexesKey get_children_key(unsigned int index);
  static FloatKey get_resolution_key(unsigned int index);

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const Hierarchies &ch, double resolution = -1);

 public:
  /** The children are added at a resolution computed using the get_resolution()
   * function */
  IMP_DECORATOR_SETUP_1(Resolution, const Hierarchies &, children);
  /** For testing only. Will go away. */
  IMP_DECORATOR_SETUP_2(Resolution, const Hierarchies &, children, double,
                        resolution);

  IMP_DECORATOR_METHODS(Resolution, Hierarchy);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_types_key(), pi);
  }
#ifndef SWIG
  using Hierarchy::get_children;
#else
  Hierarchies get_children() const;
#endif

  /** Return the children at the resolution closest to `resolution` of the
   * passed type. /*/
  Hierarchies get_children(double resolution, RepresentationType type = BALLS);

  /** Return all children at all resolutions. */
  Hierarchies get_all_children(RepresentationType type = BALLS);

  /** Set the children for a given resolution. The resolution is computed using
   * get_resolution().
   * \note The resolution parameter will go away.
   */
  void add_resolution(const Hierarchies &children,
                      RepresentationType type = BALLS, double resolution = -1);
};

IMP_DECORATORS(Resolution, Resolutions, Hierarchies);

/** Return an estimate of the resolution of the hierarchy as used by Resolution.

    It is currently the averages mass of the leaves. */
IMPATOMEXPORT double get_resolution(const Hierarchies &ch);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_RESOLUTION_H */
