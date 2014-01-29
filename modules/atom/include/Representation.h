/**
 *  \file IMP/atom/Representation.h     \brief A decorator for Representations.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REPRESENTATION_H
#define IMPATOM_REPRESENTATION_H

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
  BALLS = 0,
  GAUSSIANS = 1
};

//! A decorator for a representation.
/** It stores a number of copies of its sub hierarchy each with an associated
 * resolution. You can used it to get the representation at a given resolution.
 * \note The particle returned for a given representation should be thought of
 * as replacing this particle (not as a child of it).
 *
 * \note Only one Representation node is allowed in any path up the tree as
 * nesting them does not have a clear meaning.
 */
class IMPATOMEXPORT Representation : public Hierarchy {
  static IntsKey get_types_key();
  static ParticleIndexesKey get_representations_key();
  static FloatKey get_resolution_key(unsigned int index);
  static FloatKey get_base_resolution_key();

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                double resolution = -1);

 public:
  IMP_DECORATOR_SETUP_0(Representation);
  /** For testing only. Will go away. */
  IMP_DECORATOR_SETUP_1(Representation, double, resolution);

  IMP_DECORATOR_METHODS(Representation, Hierarchy);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_base_resolution_key(), pi);
  }

  /** Return the children at the resolution closest to `resolution` of the
   * passed type. */
  Hierarchy get_representation(double resolution,
                               RepresentationType type = BALLS);

  /** Return all children at all resolutions. */
  Hierarchies get_representations(RepresentationType type = BALLS);

  /** Representation for given resolution. The resolution is computed using
   * get_resolution().
   * \note The resolution parameter will go away, as, most likely will the type.
   */
  void add_representation(kernel::ParticleIndexAdaptor rep,
                          RepresentationType type = BALLS,
                          double resolution = -1);

  /** Return a list of all resolutions that are available for a specific
   * RepresentationType. */
  Floats get_resolutions(RepresentationType type = BALLS) const;
};

IMP_DECORATORS(Representation, Representations, Hierarchies);

/** Return an estimate of the resolution of the hierarchy as used by
   Representation.

    It is currently the inverse average radius of the leaves. */

IMPATOMEXPORT double get_resolution(kernel::Model *m, kernel::ParticleIndex pi);


/** \copydoc get_resolution(kernel::Model, kernel::particleIndex) */
inline double get_resolution(Hierarchy h) {
  return get_resolution(h.get_model(), h.get_particle_index());
}

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REPRESENTATION_H */
