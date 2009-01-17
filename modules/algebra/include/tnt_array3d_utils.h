/*
 * \file tnt_array3d_utils.h
 * \brief tnt_array3d_utils.h
 */



#ifndef IMPALGEBRA_TNT_ARRAY_3D_UTILS_H
#define IMPALGEBRA_TNT_ARRAY_3D_UTILS_H

#include "config.h"
#include <cstdlib>
#include <cassert>

IMPALGEBRA_BEGIN_NAMESPACE
namespace TNT
{
using namespace std;


template <class T>
std::ostream& operator<<(std::ostream &s, const Array3D<T> &A)
{
    int M=A.dim1();
    int N=A.dim2();
    int K=A.dim3();

    s << M << " " << N << " " << K << "\n";

    for (int i=0; i<M; i++)
    {
        for (int j=0; j<N; j++)
        {
      for (int k=0; k<K; k++)
              s << A[i][j][k] << " ";
      s << "\n";
        }
        s << "\n";
    }


    return s;
}

template <class T>
std::istream& operator>>(std::istream &s, Array3D<T> &A)
{

    int M, N, K;

    s >> M >> N >> K;

  Array3D<T> B(M,N,K);

    for (int i=0; i<M; i++)
        for (int j=0; j<N; j++)
      for (int k=0; k<K; k++)
              s >>  B[i][j][k];

  A = B;
    return s;
}



template <class T>
Array3D<T> operator+(const Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
    return Array3D<T>();

  else
  {
    Array3D<T> C(m,n,p);

    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
        C[i][j][k] = A[i][j][k] + B[i][j][k];

    return C;
  }
}


template <class T>
Array3D<T> operator-(const Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
    return Array3D<T>();

  else
  {
    Array3D<T> C(m,n,p);

    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
        C[i][j][k] = A[i][j][k] - B[i][j][k];

    return C;
  }
}




template <class T>
Array3D<T> operator*(const Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
    return Array3D<T>();

  else
  {
    Array3D<T> C(m,n,p);

    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
        C[i][j][k] = A[i][j][k] * B[i][j][k];

    return C;
  }
}


template <class T>
Array3D<T> operator/(const Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() != m ||  B.dim2() != n || B.dim3() != p )
    return Array3D<T>();

  else
  {
    Array3D<T> C(m,n,p);

    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
        C[i][j][k] = A[i][j][k] / B[i][j][k];

    return C;
  }
}



template <class T>
Array3D<T>& operator+=(Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
  {
    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
          A[i][j][k] += B[i][j][k];
  }

  return A;
}

template <class T>
Array3D<T>& operator-=(Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
  {
    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
          A[i][j][k] -= B[i][j][k];
  }

  return A;
}

template <class T>
Array3D<T>& operator*=(Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
  {
    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
          A[i][j][k] *= B[i][j][k];
  }

  return A;
}


template <class T>
Array3D<T>& operator/=(Array3D<T> &A, const Array3D<T> &B)
{
  int m = A.dim1();
  int n = A.dim2();
  int p = A.dim3();

  if (B.dim1() == m &&  B.dim2() == n && B.dim3() == p )
  {
    for (int i=0; i<m; i++)
      for (int j=0; j<n; j++)
        for (int k=0; k<p; k++)
          A[i][j][k] /= B[i][j][k];
  }

  return A;
}





} // namespace TNT

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_TNT_ARRAY_3D_UTILS_H */
