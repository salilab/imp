/**
 * \file Row class for matrix row representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#ifndef IMPSAXS_INTERNAL_ROW_H
#define IMPSAXS_INTERNAL_ROW_H

#include <IMP/saxs/saxs_config.h>
#include "Matrix.h"
#include "Diagonal.h"

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

class Row : public Matrix {
public:
  //constructors

  //default constructor: 0 length row
  Row() : Matrix(0,0) {}

  //construct a row of length m, zero filled
  explicit Row(int m) : Matrix(1,m) {}

  //construct a row of length m, with data from array a
  explicit Row(int m, double x) : Matrix(1,m,x) {}

  //construct a row of length m, with data from 1D array a
  explicit Row(int m, const double* a);

  //copy constructor
  Row (const Row &R) : Matrix(R) {}

  //construct a Row from a 1-row Matrix
  Row (const Matrix &A) : Matrix(A)
  { if (A.dim1()>1) Matrix::xerror(2,"Row(Matrix)"); }

  //construct a Row from a Diagonal
  explicit Row(const Diagonal &D);

  //construct a row from a list of at least 2 values (must be less than BIG)
  explicit Row(double t1, double t2, double t3=BIG2, double t4=BIG2,
               double t5=BIG2, double t6=BIG2, double t7=BIG2,
               double t8=BIG2, double t9=BIG2, double t10=BIG2);

  //supports for example R = 3.14;
  Row operator=(double x);

  //assignment ... R = A, where A is a 1-row matrix
  Row operator=(const Matrix &A);

  //get the primary dimension
  int dim() const { return n_; }
  int size() const { return n_; }
  //index
  double& operator[] (int i) { return data_[i]; }

  inline const double& operator[] (int i) const { return data_[i]; }

  //limitations
  void resize(int n) { Matrix::resize(1,n); }
  void resize(int m, int n)
  { if (m==1) Matrix::resize(m,n);
    else Matrix::xerror(9,"Row::resize to Matrix");}
  void append_columns(const Matrix &B )
  { if (B.dim1()==1) Matrix::append_columns(B);
    else Matrix::xerror(2,"Vector::append_columns()"); }
  void prepend_columns(const Matrix &B)
  { if (B.dim1()==1) Matrix::prepend_columns(B);
    else Matrix::xerror(2,"Vector::prepend_columns()"); }

  //prohibitions
private:
  void t() { Matrix::xerror(9,"Row::t()"); }
  void del_row(int r) { Matrix::xerror(9,"Row::del_row()"); }
  void add_rows(int m) { Matrix::xerror(9,"Row::add_row()"); }
  void append_rows(const Matrix &B) { Matrix::xerror(9,"Row::append_rows()"); }
  void prepend_rows(const Matrix &B){ Matrix::xerror(9,"Row::prepend_rows()"); }

};

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_ROW_H */
