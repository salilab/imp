/**
 *  \file IMP/cnmultifit/symmetric_multifit.h
 *  \brief Build cyclic symmetric complexes.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_SYMMETRIC_MULTIFIT_H
#define IMPCNMULTIFIT_SYMMETRIC_MULTIFIT_H

#include <IMP/cnmultifit/cnmultifit_config.h>
#include <string>

IMPCNMULTIFIT_BEGIN_NAMESPACE

//! High level interface to build cyclic symmetric complexes.
/** Given a parameter file, this function does all the work to build a number
    of fits of the monomer into a density map of the symmetric complex.

    \param param_filename Name of a parameter file.
    \param chimera_filename If not empty, the name of a file that will be
                            generated containing Chimera-compatible fit
                            information.
 */
void IMPCNMULTIFITEXPORT
    do_all_fitting(const std::string param_filename,
                   const std::string chimera_filename = "");

IMPCNMULTIFIT_END_NAMESPACE

#endif /* IMPCNMULTIFIT_SYMMETRIC_MULTIFIT_H */
