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
    These functions write geometry to text files, one line per
    geometric primitive. Each line has the form \quoted{x y z} for points or
    \quoted{x y z r} for spheres. We can easily add general dimension support
    if requested..
    Lines beginning with "#" are treated as comments.

    @{
 */
//! Write a set of 3D vectors to a file
/** \see read_pts
    \relatesalso VectorD
*/
IMPALGEBRAEXPORT void write_pts(const std::vector<VectorD<3> > &vs,
                                TextOutput out);

//! Read a set of 3D vectors from a file
/** \see write_pts
    \relatesalso VectorD
*/
IMPALGEBRAEXPORT std::vector<VectorD<3> > read_pts(TextInput in);

//! Write a set of 3d spheres to a file
/** \see read_pts
    \relatesalso SphereD
*/
IMPALGEBRAEXPORT void write_spheres(const std::vector<VectorD<3> > &vs,
                                    TextOutput out);

//! Read a set of 3d spheres from a file
/** \see write_pts
    \relatesalso SphereD
*/
IMPALGEBRAEXPORT std::vector<SphereD<3> > read_spheres(TextInput in);

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_IO_H */
