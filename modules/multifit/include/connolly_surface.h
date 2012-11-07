/**
 *  \file connolly_surface.h     \brief Generate surface for a set of atoms
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_CONNOLLY_SURFACE_H
#define IMPMULTIFIT_CONNOLLY_SURFACE_H

#include "multifit_config.h"
#include <IMP/base/file.h>
#include <IMP/atom/Atom.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Write the Connolly surface for a set of atoms to a file.
/** The algorithm is taken from Connolly's original MS program, which is
    freely distributable and Copyright 1983, Michael Connolly.

    M.L. Connolly, "Solvent-accessible surfaces of proteins and nucleic acids",
    Science, 221, p709-713 (1983).

    M.L. Connolly, "Analytical molecular surface calculation",
    J. Appl. Cryst. 16, p548-558 (1983).
 */
void IMPMULTIFITEXPORT write_connolly_surface(
           atom::Atoms as, base::TextOutput fn,
           float density, float probe_radius);

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_CONNOLLY_SURFACE_H */
