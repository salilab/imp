/**
 *  \file internal/pdb.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_PDB_H
#define IMPKERNEL_INTERNAL_PDB_H

#include <IMP/kernel/kernel_config.h>
#include "../Model.h"
#include "../container_base.h"
#include <IMP/base/file.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

/** Create balls from ATOM lines in PDB file. Ignores most everything,
    and gives all atoms the same radius. This is useful for making quick
    benchmarks that don't require the atom module for input, yet can use
    pdb files to make shapes.
*/
IMPKERNELEXPORT ParticlesTemp create_particles_from_pdb(base::TextInput fname,
                                                  Model *m);


IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_PDB_H */
