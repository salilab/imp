/**
 * \file Vector class for vector representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#ifndef IMPSAXS_INTERNAL_VECTOR_H
#define IMPSAXS_INTERNAL_VECTOR_H

#include <IMP/saxs/saxs_config.h>
#include "Matrix.h"
#include "Diagonal.h"

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

class IMPSAXSEXPORT Vector : public Matrix {
public:
  //constructors

  //default constructor: 0 length
  Vector() : Matrix(0,0) {}

  //construct a Vector of length m, zero filled
  explicit Vector(int m) : Matrix(m,1) {}

  //construct a Vector of length m, filled with the value x
  explicit Vector(int m, double x) : Matrix(m,1,x) {}

  //construct a Vector of length m, with data from 1D array a
  explicit Vector(int m, const double* a);

  //copy constructor
  Vector(const Vector &V) : Matrix(V) {}

  //construct a Vector from a 1-column Matrix
  Vector(const Matrix &A) : Matrix(A)
  {
    if (A.dim2()>1) Matrix::xerror(2,"Vector(Matrix)");
  }

  //construct a Vector from a Diagonal
  explicit Vector(const Diagonal &D);

  //construct a Vector from a list of values (each less than BIG)
  explicit Vector(double t1, double t2, double t3=BIG2, double t4=BIG2,
                  double t5=BIG2, double t6=BIG2, double t7=BIG2,
                  double t8=BIG2, double t9=BIG2, double t10=BIG2);

  //supports for example R = 3.14;
  Vector operator=(double x);

  //supports V = A, where A is a 1-column matrix
  Vector operator =(const Matrix &A);

  //copy a vector from the diagonal elements of a Diagonal matrix
  Vector operator = (const Diagonal &D);

  //get the primary dimension
  int dim() const { return m_; }
  int size() const { return m_; }

  //index
  double& operator[] (int i) { return data_[i]; }

  inline const double& operator[] (int i) const { return data_[i]; }

  //normalize this vector to unit norm, if possible
  void normalize() { double a=norm(); if (a>0.0) operator*=(1.0/a); }

  //returns the index of the algebraically maximum value in the Vector.
  int imax() const;

  //returns the index of the algebraically minimum value in the Vector.
  int imin() const;

  //returns the index of the maximum absolute value in the Vector.
  int imaxabs() const;

  //returns the index of the minimum absolute value in the Vector.
  int iminabs() const;

  //create a vector from elements i through j of *this: Vector V2=V1.range(3:9);
  Vector range(int i, int j);

  //stack operations
  //Note that these operations are intended as a convenience,
  //not an optimally efficient mechanism.
  //Every push and pop requires a resize.

  //add a value to the front of the vector (becomes the new (*this)[0])
  void push_front(double value);

  //remove the first value in the vector (remove (*this)[0])
  void pop_front();

  //add a value to the end of the vector
  void push_end(double value);

  //remove the fast value in the vector
  void pop_end();

  //sort the vector into increasing order
  void sort();

  //sort the Vector into increasing order and carry along the integer array p
  void sort(int* p);

  //return the median value;
  //for even lengths, return average of the two in the middle
  double median() const;

  //return the moving average of the elements of the vector
  //(slow/accurate method)
  Vector moving_average(int w);

  //return the moving average of the elements of the vector
  //(fast/inaccurate method)
  Vector moving_average_fast(int w);

  //limitations
  void resize(int m) { Matrix::resize(m,1); }
  void resize(int m, int n)
  { if (n==1) Matrix::resize(m,n);
    else Matrix::xerror(9,"Vector::resize to Matrix");}
  void append_rows(const Matrix &B )
  { if (B.dim2()==1) Matrix::append_rows(B);
    else Matrix::xerror(2,"Vector::append_rows()"); }
  void prepend_rows(const Matrix &B)
  { if (B.dim2()==1) Matrix::prepend_rows(B);
    else Matrix::xerror(2,"Vector::prepend_rows()"); }

  //prohibitions
private:
  void t()                              { Matrix::xerror(9,"Vector::t()"); }
  void del_column(int r)       { Matrix::xerror(9,"Vector::del_rcolumn()"); }
  void add_columns(int m)      { Matrix::xerror(9,"Vector::add_columns()"); }
  void append_columns(const Matrix &B )
  { Matrix::xerror(9,"Vector::append_columns()"); }
  void prepend_columns(const Matrix &B)
  { Matrix::xerror(9,"Vector::prepend_columns()"); }

private:
  //sort elements v[m] to v[n] in increasing order.
  //w must be a work array at least as large as v.
  //If carry is true then the integer array p is carried along.
  //The array p must be the same length as v, and
  //the integer work array q must be at least as large as p.
  static void sort(double *v, double *w, bool carry, int *p,int *q,int m,int n);
};

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_VECTOR_H */
