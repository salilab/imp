/**
 * \file Diagonal class for Diagonal matrix representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#ifndef IMPSAXS_INTERNAL_DIAGONAL_H
#define IMPSAXS_INTERNAL_DIAGONAL_H

#include <IMP/saxs/saxs_config.h>
#include "Matrix.h"

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

//The Diagonal Matrix class.
//This is coded completely separately to avoid any possible
//mix of Matrix functionality with Diagonal functionality.
class IMPSAXSEXPORT Diagonal {
private:

  int m_;
  int n_;
  int mm_;
  double* data_;

  void checkdim(int m, int n) {
    if (m<0 || n<0) Matrix::xerror(4,"Diagonal::checkdim"); return; }
  void setupd(int m, int n);

public:

  //constructors

  //default constructor: 0 length diagonal
  Diagonal() { setupd(0,0); }

  //construct a Diagonal of size m by m, zero filled
  explicit Diagonal(int m) { setupd(m,m); }

  //construct a Diagonal of size m by n, zero filled
  explicit Diagonal(int m, int n) { setupd(m,n); }

  //construct a Diagonal of size m by n, with every diagonal element set to x
  Diagonal(int m, int n, double x);

  //construct a Diagonal of size m by n, with diagonal from array a
  Diagonal (int m, int n, const double *a);

  //copy constructor
  Diagonal(const Diagonal &D);

  //copy constructor from D with shape change to smaller or larger
  Diagonal(int m, int n, const Diagonal &D);

  //construct a Diagonal:
  // 1. from a row, with D[i] = R[i]
  // 2. from a column, with D[i] = C[i]
  // 3. of the same shape as matrix A, with D[i] = A[i][i]
  explicit Diagonal(const Matrix &A);

  //destructors

  //delete all data and set size to 0 by 0
  void clear()
  {
    if (m_>0 && n_>0) { delete [] data_; }
    m_ = n_ = mm_ = 0;
  }

  ~Diagonal() { clear(); }

  //assignment-----------------------------------------------------

  //supports D = x
  Diagonal operator=(const double x);

  //supports D = D
  Diagonal operator=(const Diagonal &D);

  //resize to smaller or larger
  //keeps upper left content as far as possible; fills with zero
  void resize(int m,int n);

  //create a Diagonal of the same shape as A,
  //with diagonal elements taken from A's diagonal: D[i] = A[i][i]
  Diagonal operator=(const Matrix &A);

  //accessors------------------------------------------------------

  //get the row dimension
  inline int dim1() const { return m_; }

  //get the column dimension
  inline int dim2() const { return n_; }

  //get the smaller dimension
  inline int dmin() const { return m_ < n_ ? m_ : n_ ; }

  //get the larger dimension
  inline int dmax() const { return m_ > n_ ? m_ : n_ ; }

  //index----------------------------------------------------------
  inline double& operator[] (int i)
  {
    if (i < 0 || i >= mm_) { Matrix::xerror(1,"Diagonal::operator[]"); };
    return data_[i];
  }

  inline const double& operator[] (int i) const
  {
    if (i < 0 || i >= mm_) { Matrix::xerror(1,"Diagonal::operator[]"); };
    return data_[i];
  }

  //alternative index form... D(i,i) rather than D[i].
  //This checks the indices for proper range
  inline double& operator() (int i, int j)
  {
    if (i < 0 || i >= mm_ || i!=j ) Matrix::xerror(1,"Diagonal::operator(,)");
    return data_[i];
  }

  //equivalence operations-----------------------------------------

  //supports D1==D2
  bool operator ==(const Diagonal &D) const;

  //supports D1!=D2
  bool operator !=(const Diagonal &D) const { return !((*this)==D); }

  //approximate equality, called as D.approximate(D2,0.00000001);
  //Any absolute difference greater than the given scalar causes
  //a return of false.
  bool approximate(const Diagonal &D, double tolerance) const;

  //element-wise operations----------------------------------------

  //these operations support D+=2.0  D-=2.0   D*=2.0  D/=2.0  for example
  Diagonal operator += (double x) {
    for (int i=0; i<mm_; i++) data_[i]+=x;
    return *this;
  }

  Diagonal operator -= (double x) {
    for (int i=0; i<mm_; i++) data_[i]-=x;
    return *this;
  }

  Diagonal operator *= (double x) {
    for (int i=0; i<mm_; i++) data_[i]*=x;
    return *this;
  }

  Diagonal operator /= (double x) {
    for (int i=0; i<mm_; i++) data_[i]/=x;
    return *this;
  }

  //these operations support D+2.0  D-2.0   D*2.0  D/2.0  for example
  Diagonal operator +  (double x) const { Diagonal C(*this); C+=x; return C; }
  Diagonal operator -  (double x) const { Diagonal C(*this); C-=x; return C; }
  Diagonal operator *  (double x) const { Diagonal C(*this); C*=x; return C; }
  Diagonal operator /  (double x) const { Diagonal C(*this); C/=x; return C; }

  //operate and assign
  Diagonal operator += (const Diagonal &D);
  Diagonal operator -= (const Diagonal &D);

  //unary minus--- for B = -A; for example
  Diagonal operator - () const;

  //these operations support A+B  and A-B.
  //A and B must have exactly the same shape
  Diagonal operator + (const Diagonal &D) const;
  Diagonal operator - (const Diagonal &D) const;

  //the following allows A*B, where A's second dimension
  //must equal B's first dimension
  Diagonal operator * (const Diagonal &D) const;

  //replaces each element with its absolute value
  void mabs();

  //replaces each element with the square root of its absolute value
  void msqrt();

  //replaces each element with its square
  void msquare();

  //Replaces each element with the base 10 log of its absolute value.
  //log(A.maxabs())-30.0 is used for zero elements.
  void mlog10();

  //Replaces each element a with 10^a.
  //That is, with the antilog10 of a.
  void mpow10();

  //makes each element at least x
  void at_least(double x);

  //min / max -----------------------------------------------------

  //returns the (absolute value of the) element which is largest
  //in absolute value
  double maxabs() const;

  //returns the (absolute value of the) element which is smallest
  //in absolute value
  double minabs() const;

  //returns the index of the maximum absolute value in the Diagonal
  int imaxabs() const;

  //returns the index of the minimum absolute value in the Diagonal
  int iminabs() const;

  //returns the index of the last non-zero in the Diagonal
  int ilastnz() const;

  //returns the trace of the matrix, which is the sum of the diagonal elements.
  double trace() const;

  //find a neglible value for *this
  double epsilon() const;

  //Determine the number of approximately equal singular
  //values at the end of the list of singular values,
  //not counting the first one at that level.
  //In other words, compute p in Algorithm 12.3.1 See Golub and Van Loan,2nd Ed.
  //The point is that S(k-p-1,k-p-1) should be signifcantly larger than
  //S(k-p,k-p) where k=number of singular values in S.
  //Ideally, p should be 0.
  int plateau() const;

  //transposes *this
  void t() { int t = n_; n_ = m_; m_ = t; }

  //builders
  void zeros();   //set *this to all zeros
  void ones();    //set *this to all ones
  void identity();//set *this to identity matrix
  void iota();    //set *this[i][j] = i + j + 1.  In a row that's 1, 2, 3, ...
  void random();  //set *this to random values in (0,1)
  void gauss();   //set *this to random Gaussian, mean 0, standard deviation 1

  void print() const;
  friend std::ostream& operator<<(std::ostream& os, const Diagonal &D);

};

Diagonal operator + (double x, const Diagonal &D);

Diagonal operator * (double x, const Diagonal &D);

Diagonal operator - (double x, const Diagonal &D);

Diagonal transpose (const Diagonal &D);

Diagonal pseudoinverse(const Diagonal &D);

Diagonal smoothinverse(const Diagonal &S, double lambda);

Diagonal regularize(const Diagonal &S, double lambda);

double condition_number(const Diagonal &D);

double condition_number_nonzero(const Diagonal &D);

Matrix operator * (const Matrix &A, const Diagonal &D);

Matrix operator * (const Diagonal &D , const Matrix &B);

Matrix full(const Diagonal &D);

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_DIAGONAL_H */
