/**
 *  \file hierarchy_tools.h
 *  \brief A set of useful functionality on IMP::atom::Hierarchy decorators
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_HIERARCHY_TOOLS_H
#define IMPATOM_HIERARCHY_TOOLS_H

#include "atom_config.h"
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
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy create_protein(Model *m,
                                       std::string name,
                                       double resolution,
                                       int number_of_residues,
                                       int first_residue_index=0,
                                       double volume=-1
#ifndef IMP_DOXYGEN
                                       , bool ismol=true
#endif
);
/** Like the former create_protein(), but it enforces domain splits
    at the provide domain boundairs. The domain boundaries should be
    the start of the first domain, any boundies, and then one past
    the end of the last domain.
 */
IMPATOMEXPORT Hierarchy create_protein(Model *m,
                                       std::string name,
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
    @{
*/
/** Simplify every num_res into one particle.*/
IMPATOMEXPORT Hierarchy create_simplified_along_backbone(Chain in,
                                                         int num_res);
/** Simplify by breaking at the boundaries provided.*/
IMPATOMEXPORT Hierarchy create_simplified_along_backbone(Chain in,
           const IntRanges& residue_segments);
/** @} */


/** \name Finding information
    Get the attribute of the given particle or throw a ValueException
    if it is not applicable. The particle with the given information
    must be above the passed node.
    @{
*/
IMPATOMEXPORT std::string get_molecule_name(Hierarchy h);
IMPATOMEXPORT Ints get_residue_indexes(Hierarchy h);
IMPATOMEXPORT ResidueType get_residue_type(Hierarchy h);
IMPATOMEXPORT int get_chain_id(Hierarchy h);
IMPATOMEXPORT AtomType get_atom_type(Hierarchy h);
IMPATOMEXPORT std::string get_domain_name(Hierarchy h);
IMPATOMEXPORT int get_copy_index(Hierarchy h);
/** @} */



/** Create an excluded volume restraint for the included molecules. If a
    value is provided for resolution, then something less than the full
    resolution representation will be used.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Restraint* create_excluded_volume_restraint(const Hierarchies &hs,
                                                          double resolution=-1);




/** Set the mass, radius, residues, and coordinates to approximate the passed
    particles.
 */
IMPATOMEXPORT void setup_as_approximation(Particle *h,
                                          const ParticlesTemp &other
#ifndef IMP_DOXYGEN
                                          ,
                                          double resolution=-1
#endif
);

/** Set the mass, radius, residues, and coordinates to approximate the passed
    particle based on the leaves of h.
    \relatesalso Hierarchy
 */
IMPATOMEXPORT void setup_as_approximation(Hierarchy h
#ifndef IMP_DOXYGEN
                                          ,
                                          double resolution =-1
#endif
);

/** Transform a hierarchy. This is aware of rigid bodies.
 */
IMPATOMEXPORT void transform(Hierarchy h, const algebra::Transformation3D &tr);



/** A graph for representing a Hierarchy so you can view it
    nicely.
*/
IMP_GRAPH(HierarchyTree, bidirectional, Hierarchy, int);
/** Get a graph for the passed Hierarchy. This can be used,
    for example, to graphically display the hierarchy in 2D.
    \relatesalso Hierarchy
*/
IMPATOMEXPORT HierarchyTree get_hierarchy_tree(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_HIERARCHY_TOOLS_H */
