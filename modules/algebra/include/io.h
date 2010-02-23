/**
 *  \file algebra/io.h   \brief Classes to write entities in algebra to files.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_IO_H
#define IMPALGEBRA_IO_H

#include "Vector3D.h"
#include "Sphere3D.h"
#include <IMP/file.h>

IMPALGEBRA_BEGIN_NAMESPACE


/** @name Simple geometric IO
    These functions write geometry to text files as one line per
    object where the line is "x y z [r]" (r is there for spheres).
    Lines beginning with "#" are comments.

    @{
 */
//! Write a set of vector3Ds to a file
/** \see read_pts
*/
IMPALGEBRAEXPORT void write_pts(const std::vector<VectorD<3> > &vs,
                                TextOutput out);

//! Read a set of vector3Ds from a file
/** \see write_pts
*/
IMPALGEBRAEXPORT std::vector<VectorD<3> > read_pts(TextInput in);

//! Write a set of std::vector<SphereD<3> > to a file
/** \see read_pts
*/
IMPALGEBRAEXPORT void write_spheres(const std::vector<VectorD<3> > &vs,
                                    TextOutput out);

//! Read a set of std::vector<SphereD<3> > from a file
/** \see write_pts
*/
IMPALGEBRAEXPORT std::vector<SphereD<3> > read_spheres(TextInput in);

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_IO_H */
