/**
 *  \file io.h   \brief Classes to write entities in algebra to files.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_IO_H
#define IMPALGEBRA_IO_H

#include "Vector3D.h"
#include "Sphere3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Write a set of vector3Ds to a file
/** The file format is one vector per line, spaces between coordinates
    lines with # are a comment.
    \see read_pts
*/
void write_pts(const Vector3Ds &vs, std::ostream &out);

//! Read a set of vector3Ds from a file
/** The file format is one vector per line, spaces between coordinates
    lines with # are a comment.
    \see write_pts
*/
Vector3Ds read_pts(std::istream &in);

//! Write a set of Sphere3Ds to a file
/** The file format is one vector per line, spaces between coordinates
    lines with # are a comment.
    \see read_pts
*/
void write_spheres(const Vector3Ds &vs, std::ostream &out);

//! Read a set of Sphere3Ds from a file
/** The file format is one vector per line, spaces between coordinates
    lines with # are a comment.
    \see write_pts
*/
Sphere3Ds read_spheres(std::istream &in);

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_IO_H */
