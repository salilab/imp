/**
 *  \file IMP/algebra/io.h
 *  \brief Classes to write entities in algebra to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
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
    geometric primitive. Each line has the form \quote{x y z} for points or
    \quote{x y z r} for spheres. We can easily add general dimension support
    if requested..
    Lines beginning with "#" are treated as comments.

    @{
 */
//! Write a set of 3D vectors to a file
/** \see read_pts
    \see VectorD
*/
IMPALGEBRAEXPORT void write_pts(const Vector3Ds &vs, TextOutput out);

//! Read a set of 3D vectors from a file
/** \see write_pts
    \see VectorD
*/
IMPALGEBRAEXPORT Vector3Ds read_pts(TextInput input);

//! Write a set of 3D spheres to a file
/** \see read_pts
    \see SphereD
*/
IMPALGEBRAEXPORT void write_spheres(const Sphere3Ds &vs, TextOutput out);

//! Read a set of 3D spheres from a file
/** \see write_pts
    \see SphereD
*/
IMPALGEBRAEXPORT Sphere3Ds read_spheres(TextInput input);

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_IO_H */
