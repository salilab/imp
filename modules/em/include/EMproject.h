/**
 *  \file EMproject.h
 *  \brief Project operation for EM volumes
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_EMPROJECT_H
#define IMPEM_EMPROJECT_H

#include "em_exports.h"
#include "IMP/algebra/Matrix3D.h"

IMPEM_BEGIN_NAMESPACE

//! Projects a given 3D matrix into a 2D matrix given an rotation and an shift
//! vector.
/**
 * The rotation that ultimately describes the projection plane is described
 * itself by 3 Euler angles (in radians) following the ZYZ convention,
 * right hand side. This rotation builds the coordinate system for the
 * projection.
 * The projection plane is the XY plane (Z=0) of this new coordinate system.
 * The additional shift vector is applied in the coordinate system for the
 * projection.
 * The projection model is:
 *               p = R * r + v
 * where:
 *   p - coordinates of a point in the projection coordinate system
 *   R - Rotation matrix describing the projection plane
 *   r - coordinates of the same point in the system employed for the Matrix3D
 *   v - shift vector.
 *
 * \param[in] m3 A class containing a 3D matrix of values to project.
 * \param[out] m2  A class containing a Matrix2D of Floats to store the
 *  projection.
 * \param[in] Ydim size in rows for the Matrix2D
 * \param[in] Xdim size in columns for the Matrix2D
 * \param[in] Rot. First Euler angle (radians) defining the rotation
 * \param[in] Tilt. Second Euler angle (radians) defining the rotation
 * \param[in] Psi. Third Euler angle (radians) defining the rotation
 * \param[in] v shift vector
 */
template<typename T>
IMPEMEXPORT void project(algebra::Matrix3D<T>& m3,algebra::Matrix2D<Float>& m2,
                         Int Ydim, Int Xdim, Float Rot, Float Tilt, Float Psi,
               algebra::Vector3D v);



//! Projects a given 3D matrix into a 2D matrix given an rotation and an offset.
/**
 * The axis of projection is defined by a point and a direction of projection.
 * Additional shift vector and in-plane rotation angle for the resulting
 * projected image can be specified, and the new projection axis is computed
 * according to them.
 *
 * \param[in] m3 A class containing a 3D matrix of values.
 * \param[out] m2  A class containing a Matrix2D of Floats to store the
 *   projection.
 * \param[in] Ydim size in rows for the Matrix2D
 * \param[in] Xdim size in columns for the Matrix2D
 * \param[in] Rot. First Euler angle (ZYZ convention, right hand side)
 *   defining the direction of the axis of rotation.
 * \param[in] Tilt. Second Euler angle (ZYZ convention, right hand side)
 *   defining the direction of the axis of rotation.
 * \param[in] Psi. Third Euler angle (ZYZ convention, right hand side)
 * \
 */


IMPEM_END_NAMESPACE
#endif  /* IMPEM_EMPROJECT_H */
