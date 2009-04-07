/**
 *  \file io.h   \brief Classes to write entities in algebra to files.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_IO_H
#define IMPALGEBRA_IO_H

#include "Vector3D.h"
#include "Sphere3D.h"

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
IMPALGEBRAEXPORT void write_pts(const Vector3Ds &vs, std::ostream &out);

//! Read a set of vector3Ds from a file
/** \see write_pts
*/
IMPALGEBRAEXPORT Vector3Ds read_pts(std::istream &in);

//! Write a set of Sphere3Ds to a file
/** \see read_pts
*/
IMPALGEBRAEXPORT void write_spheres(const Vector3Ds &vs, std::ostream &out);

//! Read a set of Sphere3Ds from a file
/** \see write_pts
*/
IMPALGEBRAEXPORT Sphere3Ds read_spheres(std::istream &in);

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_IO_H */
