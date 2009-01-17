/*
*
* Template Numerical Toolkit (TNT)
*
* Mathematical and Computational Sciences Division
* National Institute of Technology,
* Gaithersburg, MD USA
*
*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
*
*/


#ifndef IMPALGEBRA_TNT_ARRAY_2D_UTILS_H
#define IMPALGEBRA_TNT_ARRAY_2D_UTILS_H

#include "../config.h"
#include <cstdlib>
#include <cassert>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
namespace TNT
{
using namespace std;


template <class T>
std::ostream& operator<<(std::ostream &s, const Array2D<T> &A)
{
    int M=A.dim1();
    int N=A.dim2();

    s << M << " " << N << "\n";

    for (int i=0; i<M; i++)
    {
        for (int j=0; j<N; j++)
        {
            s << A[i][j] << " ";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Array2D<T> &A)
{

    int M, N;

    s >> M >> N;

  Array2D<T> B(M,N);

    for (int i=0; i<M; i++)
        for (int j=0; j<N; j++)
        {
            s >>  B[i][j];
        }

  A = B;
    return s;
}


template <class T>
Array2D<T> operator+(const Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() != m ||  B.dim2() != n )
    return Array2D<T>();

  else
  {
    Array2D<T> C(m,n);

    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        C[i][j] = A[i][j] + B[i][j];
    }
    return C;
  }
}

template <class T>
Array2D<T> operator-(const Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() != m ||  B.dim2() != n )
    return Array2D<T>();

  else
  {
    Array2D<T> C(m,n);

    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        C[i][j] = A[i][j] - B[i][j];
    }
    return C;
  }
}


template <class T>
Array2D<T> operator*(const Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() != m ||  B.dim2() != n )
    return Array2D<T>();

  else
  {
    Array2D<T> C(m,n);

    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        C[i][j] = A[i][j] * B[i][j];
    }
    return C;
  }
}




template <class T>
Array2D<T> operator/(const Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() != m ||  B.dim2() != n )
    return Array2D<T>();

  else
  {
    Array2D<T> C(m,n);

    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        C[i][j] = A[i][j] / B[i][j];
    }
    return C;
  }
}





template <class T>
Array2D<T>&  operator+=(Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() == m ||  B.dim2() == n )
  {
    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        A[i][j] += B[i][j];
    }
  }
  return A;
}



template <class T>
Array2D<T>&  operator-=(Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() == m ||  B.dim2() == n )
  {
    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        A[i][j] -= B[i][j];
    }
  }
  return A;
}



template <class T>
Array2D<T>&  operator*=(Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() == m ||  B.dim2() == n )
  {
    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        A[i][j] *= B[i][j];
    }
  }
  return A;
}





template <class T>
Array2D<T>&  operator/=(Array2D<T> &A, const Array2D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();

  if (B.dim1() == m ||  B.dim2() == n )
  {
    for (int i=0; i<m; i++)
    {
      for (int j=0; j<n; j++)
        A[i][j] /= B[i][j];
    }
  }
  return A;
}

/**
    Matrix Multiply:  compute C = A*B, where C[i][j]
    is the dot-product of row i of A and column j of B.


    @param A an (m x n) array
    @param B an (n x k) array
    @return the (m x k) array A*B, or a null array (0x0)
        if the matrices are non-conformant (i.e. the number
        of columns of A are different than the number of rows of B.)


*/
template <class T>
Array2D<T> matmult(const Array2D<T> &A, const Array2D<T> &B)
{
    if (A.dim2() != B.dim1())
        return Array2D<T>();

    int M = A.dim1();
    int N = A.dim2();
    int K = B.dim2();

    Array2D<T> C(M,K);

    for (int i=0; i<M; i++)
        for (int j=0; j<K; j++)
        {
            T sum = 0;

            for (int k=0; k<N; k++)
                sum += A[i][k] * B [k][j];

            C[i][j] = sum;
        }

    return C;

}


/** Added by Daniel */
template <class T>
Array2D<T> transpose(const Array2D<T> &A)
{
  Array2D<T> ret(A.dim2(), A.dim1());
  for (unsigned int i=0; i < A.dim1(); ++i) {
    for (unsigned int j=0; j < A.dim2(); ++j) {
      ret[j][i]=A[i][j];
    }
  }
  return ret;
}


/** Added by Daniel */
template <class T>
T determinant(const Array2D<T> &A)
{
  assert(A.dim1() == A.dim2());
  assert(A.dim1() == 3);
  const T m01 = A[0][0]*A[1][1] - A[1][0]*A[0][1];
  const T m02 = A[0][0]*A[2][1] - A[2][0]*A[0][1];
  const T m12 = A[1][0]*A[2][1] - A[2][0]*A[1][1];
  const T m012 = m01*A[2][2] - m02*A[1][2] + m12*A[0][2];
  return m012;
}

} // namespace TNT

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_TNT_ARRAY_2D_UTILS_H */
