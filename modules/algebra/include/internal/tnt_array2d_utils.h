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

#include <IMP/algebra/algebra_config.h>
#include <cstdlib>
#include <cassert>
#include <IMP/base/utility.h>
#include <IMP/base/exception.h>
#include "jama_lu.h"

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
IMP_CLANG_PRAGMA(diagnostic push)
IMP_CLANG_PRAGMA(diagnostic ignored "-Wshadow")
namespace TNT
{
using namespace std;


template <class T>
inline std::ostream& operator<<(std::ostream &s, const Array2D<T> &A)
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
inline std::istream& operator>>(std::istream &s, Array2D<T> &A)
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
inline Array2D<T> operator+(const Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T> operator-(const Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T> operator*(const Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T> operator/(const Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T>&  operator+=(Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T>&  operator-=(Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T>&  operator*=(Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T>&  operator/=(Array2D<T> &A, const Array2D<T> &B)
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
inline Array2D<T> matmult(const Array2D<T> &A, const Array2D<T> &B)
{
  if (A.dim2() != B.dim1()) {
    IMP_THROW("Bad dimensions", base::ValueException);
  }
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
inline Array2D<T> transpose(const Array2D<T> &A)
{
  Array2D<T> ret(A.dim2(), A.dim1());
  for (int i=0; i < A.dim1(); ++i) {
    for (int j=0; j < A.dim2(); ++j) {
      ret[j][i]=A[i][j];
    }
  }
  return ret;
}


/** Added by Keren */
template <class T>
inline T determinant(const Array2D<T> &M)
{
  assert(M.dim1() == M.dim2()); // square matrices only please
  // Compute determinant using LU factors.
  JAMA::LU<T> lu(M);
  return lu.det();
}

/** Added by Keren */
template<class T>
inline Array2D<T> inverse(const Array2D<T> &M)
{
  assert(M.dim1() == M.dim2()); // square matrices only please
  // solve for inverse with LU decomposition
  JAMA::LU<T> lu(M);
  // create identity matrix
  Array2D<T> id(M.dim1(), M.dim2(), (T)0);
  for (int i = 0; i < M.dim1(); i++) id[i][i] = 1;
  // solves A * A_inv = Identity
  return lu.solve(id);
}

/** Added by Keren */
template<class T>
inline bool is_inversable(const Array2D<T> &M)
{
  if (M.dim1() != M.dim2()) {
    return false;
  }
  // solve for inverse with LU decomposition
  JAMA::LU<T> lu(M);
  // create identity matrix
  Array2D<T> id(M.dim1(), M.dim2(), (T)0);
  for (int i = 0; i < M.dim1(); i++) id[i][i] = 1;
  // solves A * A_inv = Identity
  Array2D<T> inv = lu.solve(id);
  //check if the values of inv are all numbers
  for(int d1=0;d1<inv.dim1();d1++){
  for(int d2=0;d2<inv.dim2();d2++){
    if(IMP::base::isnan(inv[d1][d2])) {
      return false;
    }
  }
  }
  return true;
}

/** Added by Keren */
template<class T>
inline Array1D<T> multiply(const Array2D<T> &M,const Array1D<T> &V)
{
  assert(M.dim2() == V.dim1());
  Array1D<T> ans(V.dim1());
  for(int r=0; r< M.dim1(); ++r) {
    ans[r]=0.;
    for(int c=0; c< M.dim2(); ++c) {
      ans[r] += M[r][c]*V[c];
    }
  }
  return ans;
}

/** Added by Keren */
template<class T>
inline Array1D<T> multiply(T s,const Array1D<T> &V)
{
  Array1D<T> ans(V.dim1());
  for(int i=0; i< V.dim1(); ++i) {
    ans[i]=V[i]*s;
  }
  return ans;
}

/** Added by Keren */
template<class T>
inline Array2D<T> multiply(T s,const Array2D<T> &V)
{
  Array2D<T> ans(V.dim1(), V.dim2());
  for(int i=0; i< V.dim1(); ++i) {
    for(int j=0; j< V.dim2(); ++j) {
      ans[i][j]=V[i][j]*s;
    }
  }
  return ans;
}


/** Added by Keren */
template<class T>
inline Array1D<T> add(const Array1D<T> &V1,const Array1D<T> &V2)
{
  assert(V1.dim1() == V2.dim1());
  Array1D<T> ans(V1.dim1());
  for(int i=0; i< V1.dim1(); ++i) {
    ans[i]=V1[i]+V2[i];
  }
  return ans;
}

/** Added by Keren */
template<class T>
inline Array1D<T> subtract(const Array1D<T> &V1,const Array1D<T> &V2)
{
  Array1D<T> ans(V1.dim1());
  for(int i=0; i< V1.dim1(); ++i) {
    ans[i]=V1[i]-V2[i];
  }
  return ans;
}

/** Added by Keren */
template<class T>
inline T dot_product(const Array1D<T> &V1,const Array1D<T> &V2)
{
  assert(V1.dim1() == V2.dim1());
  T ans = (T)0;
  for(int i=0; i< V1.dim1(); ++i) {
    ans += V1[i]*V2[i];
  }
  return ans;
}

/** Added by Keren */
template<class T>
inline void set_identity(Array2D<T> &M)
{
  for(int r=0; r< M.dim1(); ++r) {
    for(int c=0; c< M.dim2(); ++c) {
      M[r][c]=0.;
    }
    M[r][r]=1.;
  }
}

/** Added by Keren */
template<class T>
inline void set_row(Array2D<T> &M, const Array1D<T> &v,int i)
{
  assert(i<M.dim1());
  assert(v.dim1() == M.dim2());
  for(int c=0; c< v.dim1(); ++c) {
    M[i][c]=v[c];
  }
}

} // namespace TNT
IMP_CLANG_PRAGMA(diagnostic pop)
IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_TNT_ARRAY_2D_UTILS_H */
