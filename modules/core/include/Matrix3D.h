/**
 *  \file Matrix3D.h   \brief 3D matrix class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_MATRIX_3D_H
#define IMPCORE_MATRIX_3D_H

#include "config.h"

#include "IMP/Vector3D.h"
#include <math.h>

IMPCORE_BEGIN_NAMESPACE

//! 3D matrix class.
/** \ingroup helper
*/
class IMPCOREEXPORT Matrix3D
{
public:
  //! Default constructor. All cells of the matrix are 0
  Matrix3D() {
    vrow[0] = vrow[1] = vrow[2] = Vector3D(0, 0, 0);
  }

  //! Initializes a matrix with explicit values
  Matrix3D(Float a11, Float a12, Float a13,
           Float a21, Float a22, Float a23,
           Float a31, Float a32, Float a33) {
    vrow[0] = Vector3D(a11, a12, a13);
    vrow[1] = Vector3D(a21, a22, a23);
    vrow[2] = Vector3D(a31, a32, a33);
  }
  //! Constructs a matrix with d's in the diagonal and 0's else where.
  /** \param[in] d the value of the diagonal cells
   */
  Matrix3D(Float d){
  vrow[0] = Vector3D(d, 0, 0);
  vrow[1] = Vector3D(0, d, 0);
  vrow[2] = Vector3D(0, 0, d);
  }

  Vector3D operator*(const Vector3D &o) const {
    return Vector3D(vrow[0]*o, vrow[1]*o, vrow[2]*o);
  }

  //! Returns the rotation around the x-axis for a rotational matrix assuming
  //! rotations are performed in the order of x-y-z.
  /** \note the output is  meaningless if the matrix is not a rotational matrix.
  */
  Float rotX() const {
    return atan2(element(3,2), element(3,3));
  }
  //! Returns the rotation around the y-axis for a rotational matrix assuming
  //! rotations are performed in the order of x-y-z.
  /** \note the output is  meaningless if the matrix is not a rotational matrix.
  */
  Float rotY() const {
    return atan2(element(3,1),
                 sqrt(element(2,1)*element(2,1)+element(1,1)*element(1,1)));
  }

  //! Returns the rotation around the z-axis for a rotational matrix assuming
  //! rotations are performed in the order of x-y-z.
  /** \note the output is  meaningless if the matrix is not a rotational matrix.
  */
  Float rotZ() const {
    return atan2(element(2,1), element(1,1));
  }

  //! Returns 3 rotation angles assuming
  //! rotations are performed in the order of x-y-z.
  /** \note the output is  meaningless if the matrix is not a rotational matrix.
  */
  Vector3D rot() const {
    return Vector3D(rotX(), rotY(), rotZ());
  }

  void show(std::ostream& out = std::cout) const {
    out <<vrow[0]<<'\n'<<vrow[1]<<'\n'<<vrow[2]<<'\n';
  }

private:
  //! Returns the numerical value at position row, col in the 3X3 matrix
  //! row, col should be within the range of 1..3 or run time errors may occur.
  Float element(const unsigned short r, const unsigned short col) const {
    return vrow[r-1][col-1];
  }
  Vector3D  vrow[3];     // 3 columns of the matrix are held in 3 vectors
};

IMPCORE_END_NAMESPACE
#endif  /* IMPCORE_MATRIX_3D_H */
