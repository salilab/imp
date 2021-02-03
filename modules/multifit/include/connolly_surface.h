/**
 *  \file IMP/multifit/connolly_surface.h
 *  \brief Generate surface for a set of atoms
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_CONNOLLY_SURFACE_H
#define IMPMULTIFIT_CONNOLLY_SURFACE_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/file.h>
#include <IMP/atom/Atom.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Write the Connolly surface for a set of atoms to a file.
void IMPMULTIFITEXPORT
    write_connolly_surface(atom::Atoms atoms, TextOutput fn,
                           float density, float probe_radius);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_CONNOLLY_SURFACE_H */
