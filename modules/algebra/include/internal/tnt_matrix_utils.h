/*
 * \file tnt_matrix_utils.h
 * \brief tnt_matrix_utils.h
 */

#ifndef IMPALGEBRA_TNT_MATRIX_UTILS_H
#define IMPALGEBRA_TNT_MATRIX_UTILS_H

#include <assert.h>
#include "tnt_array1d.h"
#include "tnt_array2d.h"
#include "jama_lu.h"
IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE
namespace TNT
{

template<class T>
T determinant(const Array2D<T> &M)
{
  assert(M.dim1() == M.dim2()); // square matrices only please
  // Compute determinant using LU factors.
  JAMA::LU<T> lu(M);
  return lu.det();
}


template<class T>
Array2D<T> inverse(const Array2D<T> &M)
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

template<class T>
bool is_inversable(const Array2D<T> &M)
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
    if(std::isnan(inv[d1][d2])) {
      return false;
    }
  }
  }
  return true;
}


template<class T>
Array2D<T> transpose(const Array2D<T> &M)
{
  Array2D<T> tran(M.dim2(), M.dim1() );
  for(int r=0; r< M.dim1(); ++r)
    for(int c=0; c< M.dim2(); ++c)
      tran[c][r] = M[r][c];
  return tran;
}

template<class T>
Array2D<T> transpose(const Array1D<T> &V)
{
  Array2D<T> tran(V.dim1(), 1);
  for(int i=0; i< V.dim1(); ++i)
    tran[i][1] = V[i];
  return tran;
}


template<class T>
Array1D<T> multiply(const Array2D<T> &M,const Array1D<T> &V)
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

template<class T>
Array1D<T> multiply(T s,const Array1D<T> &V)
{
  Array1D<T> ans(V.dim1());
  for(int i=0; i< V.dim1(); ++i) {
    ans[i]=V[i]*s;
  }
  return ans;
}

template<class T>
Array1D<T> add(const Array1D<T> &V1,const Array1D<T> &V2)
{
  assert(V1.dim1() == V2.dim1());
  Array1D<T> ans(V1.dim1());
  for(int i=0; i< V1.dim1(); ++i) {
    ans[i]=V1[i]+V2[i];
  }
  return ans;
}

template<class T>
Array1D<T> subtract(const Array1D<T> &V1,const Array1D<T> &V2)
{
  Array1D<T> ans(V1.dim1());
  for(int i=0; i< V1.dim1(); ++i) {
    ans[i]=V1[i]-V2[i];
  }
  return ans;
}

template<class T>
T dot_product(const Array1D<T> &V1,const Array1D<T> &V2)
{
  assert(V1.dim1() == V2.dim1());
  T ans = (T)0;
  for(int i=0; i< V1.dim1(); ++i) {
    ans += V1[i]*V2[i];
  }
  return ans;
}

template<class T>
void set_identity(Array2D<T> &M)
{
  for(int r=0; r< M.dim1(); ++r) {
    for(int c=0; c< M.dim2(); ++c) {
      M[r][c]=0.;
    }
    M[r][r]=1.;
  }
}




template<class T>
void set_row(Array2D<T> &M, const Array1D<T> &v,int i)
{
  assert(i<M.dim1());
  assert(v.dim1() == M.dim2());
  for(int c=0; c< v.dim1(); ++c) {
    M[i][c]=v[c];
  }
}


}//end namespace
IMPALGEBRA_END_INTERNAL_NAMESPACE
#endif /* IMPALGEBRA_TNT_MATRIX_UTILS_H */
