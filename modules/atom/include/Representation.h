/**
 *  \file IMP/atom/Representation.h     \brief A decorator for Representations.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_REPRESENTATION_H
#define IMPATOM_REPRESENTATION_H

#include <IMP/atom/atom_config.h>
#include "atom_macros.h"
#include "Hierarchy.h"

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

IMPATOMEXPORT extern const double ALL_RESOLUTIONS;

/** Eventually, other types of representation will be supported */
enum RepresentationType {
  BALLS = 0,
  DENSITIES = 1
};

//! A decorator for a representation.
/** It stores a number of copies of its sub hierarchy each with an associated
 * resolution. You can use it to get the representation at a given resolution.
 * \note The particle returned for a given representation should be thought of
 * as replacing this particle (not as a child of it).
 *
 * \note Only one Representation node is allowed in any path up the tree as
 * nesting them does not have a clear meaning.
 */
class IMPATOMEXPORT Representation : public Hierarchy {
  static IntsKey get_types_key();
  static ParticleIndexesKey get_representations_key();
  static FloatsKey get_resolutions_key();
  static FloatKey get_base_resolution_key();

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                double resolution = -1);

  static void do_setup_particle(Model *m, ParticleIndex pi,
                                Representation o){
    do_setup_particle(m,pi,
                      o.get_model()->get_attribute(get_base_resolution_key(),
                                                   o.get_particle_index()));
  }
 public:
  IMP_DECORATOR_SETUP_0(Representation);
  IMP_DECORATOR_SETUP_1(Representation, double, resolution);
  IMP_DECORATOR_SETUP_1(Representation, Representation, other);
  IMP_DECORATOR_METHODS(Representation, Hierarchy);

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_base_resolution_key(), pi);
  }

  //! Get children at the resolution closest to `resolution` of the passed type.
  Hierarchy get_representation(double resolution,
                               RepresentationType type = BALLS);

  //! Return all children at all resolutions.
  Hierarchies get_representations(RepresentationType type = BALLS);

  //! Add the representation for the given resolution.
  /** If the resolution is
      not given it is computed using get_resolution().
      Currently only 'BALLS' and 'DENSITIES' are supported; eventually,
      other types of representation may be supported.
   */
  void add_representation(ParticleIndexAdaptor rep,
                          RepresentationType type = BALLS,
                          double resolution = -1);

  //! Get all resolutions that are available for a specific RepresentationType.
  Floats get_resolutions(RepresentationType type = BALLS) const;

  //! Remove the given representation.
  /** The representation is only removed from the Representation
      decorator; it is not destroyed. For that, use IMP::atom::destroy().
   */
  void remove_representation(ParticleIndexAdaptor rep);

  //! If you've changed the parent, update all the resolutions
  void update_parents();
};

IMP_DECORATORS(Representation, Representations, Hierarchies);

//! Estimate the resolution of the hierarchy as used by Representation.
/** It is currently the inverse average radius of the leaves.
 */
IMPATOMEXPORT double get_resolution(Model *m, ParticleIndex pi);

/** \copydoc get_resolution(Model, ParticleIndex) */
inline double get_resolution(Hierarchy h) {
  return get_resolution(h.get_model(), h.get_particle_index());
}

//! Return the Representation object containing this hierarchy
IMPATOMEXPORT Representation get_representation(Hierarchy h, bool nothrow = false);

//! Traverse through the tree and show atom info, including representations
IMPATOMEXPORT void show_with_representations(Hierarchy h,
                                             std::ostream& out = std::cout);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_REPRESENTATION_H */
