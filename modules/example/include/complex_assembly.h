/**
 *  \file IMP/example/complex_assembly.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_COMPLEX_ASSEMBLY_H
#define IMPEXAMPLE_COMPLEX_ASSEMBLY_H

#include <IMP/example/example_config.h>
#include <IMP/base_types.h>
#include <IMP/algebra/BoundingBoxD.h>

IMPEXAMPLE_BEGIN_NAMESPACE

/** Provide an example of a more involved protocol for assembly a complex. The
    protocol adds the particles one at a time based on how well connected they
    are to the already added particles (in the interaction graph). After each
    addition, the assembly is optimized. The protocol seems to work at
    assembling the residues of a protein from a truncated distance matrix.
*/
IMPEXAMPLEEXPORT void optimize_assembly(
    kernel::Model *m, const kernel::ParticlesTemp &components,
    const kernel::RestraintsTemp &interactions,
    const kernel::RestraintsTemp &other_restraints,
    const algebra::BoundingBox3D &bb, PairScore *ev, double cutoff,
    const PairPredicates &excluded = PairPredicates());

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_COMPLEX_ASSEMBLY_H */
