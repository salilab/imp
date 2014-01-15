/**
 *  \file IMP/atom/hierarchy_tools.h
 *  \brief A set of useful functionality on IMP::atom::Hierarchy decorators
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_HIERARCHY_TOOLS_H
#define IMPATOM_HIERARCHY_TOOLS_H

#include <IMP/atom/atom_config.h>
#include <IMP/algebra/algebra_config.h>
#include "Hierarchy.h"
#include "Residue.h"
#include "Atom.h"
#include <IMP/core/Typed.h>
#include <IMP/core/XYZR.h>
#include "Selection.h"
#include <boost/graph/adjacency_list.hpp>

IMPATOM_BEGIN_NAMESPACE

//! Create a coarse grained molecule
/** The coarse grained model is created with a number of spheres
    based on the resolution and the volume. If the volume is not provided
    it is estimated based on the number of residues. The protein is
    created as a molecular hierarchy rooted at p. The leaves are Domain
    particles with appropriate residue indexes stored and are XYZR
    particles.

    Volume is, as usual, in cubic anstroms.

    Currently the function creates a set of balls with radii no greater
    than resolution which overlap by 20% and have a volume of their
    union equal to the passed volume.

    The coordinates of the balls defining the protein are optimized
    by default, and have garbage coordinate values.
    \untested{create_protein}
    \unstable{create_protein}
    See Hierarchy
 */
IMPATOMEXPORT Hierarchy
    create_protein(kernel::Model *m, std::string name, double resolution,
                   int number_of_residues, int first_residue_index = 0,
                   double volume = -1
#ifndef IMP_DOXYGEN
                   ,
                   bool ismol = true
#endif
                   );
/** Like the former create_protein(), but it enforces domain splits
    at the provide domain boundairs. The domain boundaries should be
    the start of the first domain, any boundies, and then one past
    the end of the last domain.
 */
IMPATOMEXPORT Hierarchy create_protein(kernel::Model *m, std::string name,
                                       double resolution,
                                       const Ints domain_boundaries);

/** \name Simplification along backbone

    These two methods create a simplified version of a molecule by
    merging residues sequentially. In one case every n residues are
    merged, in the other, the intervals are passed manually. The
    resulting molecule is not optimized by default and has no
    restraints automatically created.

    At the moment, the calls only support unmodified hierarchies loaded
    by read_pdb() which have only protein or DNA members.

    They return Hierarchy() if the input chain is empty.

    If keep_detailed is true, then the original high resolution structure
    particles are added as children of the simplified structure.
    @{
*/
/** Simplify every num_res into one particle.*/
IMPATOMEXPORT Hierarchy
    create_simplified_along_backbone(Hierarchy input, int num_res,
                                     bool keep_detailed = false);
/** Simplify by breaking at the boundaries provided.*/
IMPATOMEXPORT Hierarchy
    create_simplified_along_backbone(Chain input,
                                     const IntRanges &residue_segments,
                                     bool keep_detailed = false);
/** @} */

/** \name Finding information
    Get the attribute of the given particle or throw a ValueException
    if it is not applicable. The particle with the given information
    must be above the passed node.
    @{
*/
IMPATOMEXPORT Ints get_residue_indexes(Hierarchy h);
IMPATOMEXPORT ResidueType get_residue_type(Hierarchy h);
/** \deprecated_at{2.2} Use the string version. */
IMPATOM_DEPRECATED_FUNCTION_DECL(2.2)
inline char get_chain_id_char(Hierarchy h) {
  IMPATOM_DEPRECATED_FUNCTION_DEF(2.2, "Use string version");
  return get_chain_id(h)[0];
}
IMPATOMEXPORT AtomType get_atom_type(Hierarchy h);
IMPATOMEXPORT std::string get_domain_name(Hierarchy h);
/** @} */

/** Create an excluded volume restraint for the included molecules. If a
    value is provided for resolution, then something less than the full
    resolution representation will be used.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
    See Hierarchy
 */
IMPATOMEXPORT kernel::Restraint *create_excluded_volume_restraint(
    const Hierarchies &hs, double resolution = -1);

/** Set the mass, radius, residues, and coordinates to approximate the passed
    particles.
 */
IMPATOMEXPORT void setup_as_approximation(kernel::Particle *h,
                                          const kernel::ParticlesTemp &other
#ifndef IMP_DOXYGEN
                                          ,
                                          double resolution = -1
#endif
                                          );

/** Set the mass, radius, residues, and coordinates to approximate the passed
    particle based on the leaves of h.
    See Hierarchy
 */
IMPATOMEXPORT void setup_as_approximation(Hierarchy h
#ifndef IMP_DOXYGEN
                                          ,
                                          double resolution = -1
#endif
                                          );

/** Transform a hierarchy. This is aware of rigid bodies.
 */
IMPATOMEXPORT void transform(Hierarchy h, const algebra::Transformation3D &tr);

/** A graph for representing a Hierarchy so you can view it
    nicely.
*/
IMP_GRAPH(HierarchyTree, bidirectional, Hierarchy, int,
          vertex.show(out, "\\n"));
/** Get a graph for the passed Hierarchy. This can be used,
    for example, to graphically display the hierarchy in 2D.
    See Hierarchy
*/
IMPATOMEXPORT HierarchyTree get_hierarchy_tree(Hierarchy h);

/** \class HierarchyGeometry
    \brief Display an IMP::atom::Hierarchy particle as balls.

    \class HierarchiesGeometry
    \brief Display an IMP::SingletonContainer of IMP::atom::Hierarchy particles
    as balls.
*/
class HierarchyGeometry : public display::SingletonGeometry {
  double res_;
  mutable IMP::base::map<kernel::Particle *, base::Pointer<display::Geometry> >
      components_;

 public:
  HierarchyGeometry(core::Hierarchy d, double resolution = 0)
      : SingletonGeometry(d), res_(resolution) {}
  display::Geometries get_components() const {
    display::Geometries ret;
    atom::Hierarchy d(get_particle());
    atom::Selection sel(d);
    sel.set_resolution(res_);
    kernel::ParticlesTemp ps = sel.get_selected_particles();
    for (unsigned int i = 0; i < ps.size(); ++i) {
      if (components_.find(ps[i]) == components_.end()) {
        IMP_NEW(core::XYZRGeometry, g, (core::XYZR(ps[i])));
        components_[ps[i]] = g;
      }
      ret.push_back(components_.find(ps[i])->second);
    }
    return ret;
  }
  IMP_OBJECT_METHODS(HierarchyGeometry);
};
class HierarchiesGeometry : public display::SingletonsGeometry {
  double res_;
  mutable IMP::base::map<kernel::ParticleIndex,
                         base::Pointer<display::Geometry> > components_;

 public:
  HierarchiesGeometry(SingletonContainer *sc, double resolution = -1)
      : SingletonsGeometry(sc), res_(resolution) {}
  display::Geometries get_components() const {
    display::Geometries ret;
    IMP_FOREACH(kernel::ParticleIndex pi, get_container()->get_contents()) {
      kernel::Model *m = get_container()->get_model();
      if (components_.find(pi) == components_.end()) {
        IMP_NEW(HierarchyGeometry, g, (atom::Hierarchy(m, pi), res_));
        components_[pi] = g;
      }
      ret.push_back(components_.find(pi)->second);
    }
    return ret;
  }
  IMP_OBJECT_METHODS(HierarchiesGeometry);
};

/** Transform a hierarchy, being aware of rigid bodies and intermediate nodes
 * with coordinates. Rigid bodies must either be completely contained within the
 * hierarchy or completely disjoint (no rigid bodies that contain particles in
 * the hierarchy and other particles not in it are allowed). */
IMPATOMEXPORT void transform(atom::Hierarchy h,
                             const algebra::Transformation3D &tr);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_HIERARCHY_TOOLS_H */
