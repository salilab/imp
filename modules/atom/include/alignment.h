/**
 *  \file IMP/atom/alignment.h
 *  \brief alignment of structures
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_ALIGNMENT_H
#define IMPATOM_ALIGNMENT_H

#include <IMP/atom/atom_config.h>
#include <IMP/core/XYZ.h>
#include "Selection.h"
#include <IMP/algebra.h>

IMPATOM_BEGIN_NAMESPACE
/**
   Get the transformation to align two selections
 */
IMP::algebra::Transformation3D get_transformation_aligning_first_to_second(const IMP::atom::Selection &s1,
                                      const IMP::atom::Selection &s2);

IMP::algebra::Transformation3D get_transformation_aligning_first_to_second(const IMP::atom::Selection &s1,
                                      const IMP::atom::Selection &s2){

	IMP::Particles ps1=s1.get_selected_particles();
	IMP::Particles ps2=s2.get_selected_particles();

	IMP::core::XYZs ds1(ps1.size());
	IMP::core::XYZs ds2(ps2.size());

	for (unsigned int i=0; i<ps1.size(); ++i){ds1[i]=IMP::core::XYZ(ps1[i]);}
	for (unsigned int i=0; i<ps2.size(); ++i){ds2[i]=IMP::core::XYZ(ps2[i]);}

	return IMP::algebra::get_transformation_aligning_first_to_second(ds1,ds2);
}

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ALIGNMENT_H */
