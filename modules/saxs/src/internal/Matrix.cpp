/**
 * \file Matrix class for matrix representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#include <IMP/saxs/internal/Matrix.h>

#include <IMP/constants.h>

#include <iostream>
#include <iomanip>
#include <cmath>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

void Matrix::setup2(int m, int n) {
  checkdim(m,n);
  m_ = m;
  n_ = n;
  if (m_==0 || n_==0) return;
  int sz = m_*n_;
  data_ = new double[sz];
  for (int i=0; i<sz; i++) data_[i] = 0.0;

  v_ = new double*[m_];
  double* p = &(data_[0]);
  for (int i=0; i<m_; i++) { v_[i] = p; p += n_; }
}

Matrix::Matrix(int m, int n, double x) : m_(m), n_(n) {
  setup();
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = x;
}

Matrix::Matrix(int m, int n, int ndim, const double *a) : m_(m), n_(n) {
  setup();
  for (int i=0; i<m; i++)
    for (int j=0; j<n; j++)
      v_[i][j] = a[i*ndim + j];
}

Matrix::Matrix(const Matrix &A) : m_(A.m_), n_(A.n_) {
  setup();
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = A.v_[i][j];
}

Matrix Matrix::operator=(const double x) {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = x;
  return *this;
}

Matrix Matrix::operator=(const Matrix &A) {
  clear();
  setup2(A.m_ , A.n_);
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = A[i][j];
  return *this;
}

bool Matrix::operator==(const Matrix &B) const {
  if (m_ != B.m_ || n_ != B.n_ ) return false;
  if (m_ == 0    || n_ == 0    ) return true;
  int sz=m_*n_;
  for (int i=0; i<sz; i++)
    { if (data_[i] != B.data_[i]) return false; }
  return true;
}

bool Matrix::approximate(const Matrix &B, double tolerance) const {
  if (m_ != B.m_ || n_ != B.n_ ) return false;
  if (m_ == 0    || n_ == 0    ) return true;
  int sz=m_*n_;
  for (int i=0; i<sz; i++)
    { if (std::abs(data_[i]-B.data_[i])>tolerance) return false; }
  return true;
}

bool Matrix::operator!=(const Matrix &B) const {
  return !((*this)==B);
}

Matrix Matrix::operator- () {
  Matrix C(m_,n_);
  int sz=m_*n_;
  for (int i=0; i<sz; i++) C.data_[i] = -data_[i];
  return C;
}

Matrix Matrix::operator+= (const Matrix &B) {
  if (m_!=B.m_ || n_!=B.n_) Matrix::xerror(2,"Matrix+=Matrix");
  int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]+=B.data_[i];
  return *this;
}

Matrix Matrix::operator-= (const Matrix &B) {
  if (m_!=B.m_ || n_!=B.n_) Matrix::xerror(2,"Matrix-=Matrix");
  int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]-=B.data_[i];
  return *this;
}

Matrix Matrix::operator*= (const Matrix &B) {
  if (m_!=B.m_ || n_!=B.n_) Matrix::xerror(2,"Matrix*=Matrix");
  int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]*=B.data_[i];
  return *this;
}

Matrix Matrix::operator+ (const Matrix &B) const {
  if (m_!=B.m_ || n_!=B.n_) Matrix::xerror(2,"Matrix+Matrix");
  Matrix C(*this);
  return C+=B;
}

Matrix Matrix::operator- (const Matrix &B) const {
  if (m_!=B.m_ || n_!=B.n_) Matrix::xerror(2,"Matrix-Matrix");
  Matrix C(*this);
  return C-=B;
}

Matrix Matrix::operator* (const Matrix &B) const {
  if (n_ != B.m_) Matrix::xerror(2,"Matrix*Matrix");
  Matrix C(m_,B.n_);
  if (m_==0 || n_==0 || B.m_==0 || B.n_==0) return C;
  double sum;
  for (int i=0; i<m_; i++)
    for (int j=0; j<B.n_; j++) {
      sum = 0.0;
      for (int k=0; k<n_; k++) sum=sum+v_[i][k]*B[k][j];
      C[i][j]=sum;
    }
  return C;
}

void Matrix::normalize_them(Matrix &B, Matrix &E, int i, double rownorm) {
  if (rownorm>0.0) { //some rows may be zero
    double scale = 1/rownorm;
    for (int j=0; j<n_; j++)
      v_[i][j] = v_[i][j]*scale;
    for (int j=0; j<B.n_; j++)
      B(i,j) = B(i,j)*scale;
    for (int j=0; j<E.n_; j++)
      E(i,j) = E(i,j)*scale;
  }
  //else what to do with B and E if row is zero??
}

void Matrix::normalize_rows(Matrix &B, Matrix &E) {
  if (m_ != B.m_  || m_ != E.m_) Matrix::xerror(2,"normalize_rows(...)");
  for (int i=0; i<m_; i++) {
    Matrix R=this->get_row(i);
    normalize_them(B,E,i,R.norm());
  }
}

void Matrix::normalize_rows_max1(Matrix &B, Matrix &E) {
  if (m_ != B.m_  || m_ != E.m_) Matrix::xerror(2,"normalize_rows_max1(...)");
  for (int i=0; i<m_; i++) {
    Matrix R=this->get_row(i);
    normalize_them(B,E,i,R.maxabs());
  }
}

void Matrix::mabs() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) data_[i]=std::abs(data_[i]);
}

void Matrix::msqrt() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) data_[i]=sqrt(std::abs(data_[i]));
}

void Matrix::msquare() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) data_[i]=data_[i]*data_[i];
}

void Matrix::mlog10() {
  int sz=m_*n_;
  double tiny = log10((*this).maxabs()) - 30.0;
  for (int i=0; i<sz; i++)
    if (data_[i]!=0.0) data_[i]=log10(std::abs(data_[i]));
    else               data_[i] = tiny;
}

void Matrix::mpow10() {
  int sz=m_*n_;
  double scale=log(10.0);
  for (int i=0; i<sz; i++) data_[i]=exp(scale*data_[i]);
}

void Matrix::at_least(double x) {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) if (data_[i]<x) data_[i]=x;
}

void Matrix::keep_digits(int n) {
  int p=1;
  for (int i=1; i<n; i++) p*=10;
  int sz=m_*n_;
  for (int i=0; i<sz; i++) {
    if (data_[i]==0.0) continue;
    double x=std::abs(data_[i]);
    double q=1;
    while (x>10*p) { x/=10; q*=10; }
    while (x<p)    { x*=10; q/=10; }
    int val=x+0.5;
    x = val;
    x *= q;
    if (data_[i]>0.0) data_[i] = x; else data_[i] = -x;
  }
}

void Matrix::trunc() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) {
    if (data_[i]<0.0) data_[i] = -int(std::abs(data_[i]));
    else              data_[i] =  int(     data_[i] );
  }
}

void Matrix::round() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) {
    if (data_[i]<0.0) data_[i] = -int(std::abs(data_[i])+0.5);
    else              data_[i] =  int(     data_[i] +0.5);
  }
}

void Matrix::ceil() {
  int sz=m_*n_;
  int k;
  double d;
  for (int i=0; i<sz; i++) {
    k = int(data_[i]);
    d = double(k);
    data_[i] = d<data_[i] ? d+1 : d;
  }
}

void Matrix::signum() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) {
    if (data_[i]< 0.0) data_[i] = -1.0;
    else               data_[i] =  1.0;
  }
}

void Matrix::trinity() {
  int sz=m_*n_;
  for (int i=0; i<sz; i++) {
    if (data_[i]< 0.0) data_[i] = -1.0;
    if (data_[i]> 0.0) data_[i] =  1.0;
  }
}

void Matrix::to_percentages() {
  for (int k=0; k<n_; k++) {
    Matrix obs=get_column(k);
    double sum=obs.sumabs();
    if (sum>0.0)obs*=100.0/sum;
    set_column(k,obs);
  }
}

double Matrix::maxval() const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::max()");
  double t=data_[0];
  for (int i=1; i<sz; i++) { if (data_[i]>t) t = data_[i]; }
  return t;
}

double Matrix::minval() const {
  int sz=m_*n_;
  if (sz<1) return 0.0;
  double t=data_[0];
  for (int i=1; i<sz; i++) { if (data_[i]<t) t = data_[i]; }
  return t;
}

double Matrix::maxabs() const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::maxabs()");
  double t=std::abs(data_[0]);
  for (int i=1; i<sz; i++) { if (std::abs(data_[i])>t) t = std::abs(data_[i]); }
  return t;
}

double Matrix::minabs() const {
  int sz=m_*n_;
  if (sz<1) return 0.0;
  double t=std::abs(data_[0]);
  for (int i=1; i<sz; i++) { if (std::abs(data_[i])<t) t = std::abs(data_[i]); }
  return t;
}

double Matrix::minpos() const {
  int sz=m_*n_;
  if (sz<1) return 0.0;
  double t=maxabs();
  for (int i=0; i<sz; i++) { if (data_[i]>0.0 && data_[i]<t) t = data_[i]; }
  return t;
}

void Matrix::ijmaxabs(int &imax, int &jmax) const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::ijmaxabs()");
  double t=std::abs(data_[0]);
  imax=0; jmax=0;
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      if (std::abs(v_[i][j])>t) { t=std::abs(v_[i][j]); imax=i; jmax=j; }
}

double Matrix::sum() const {
  int sz=m_*n_;
  if (sz<1) return 0.0;
  double t=0.0;
  for (int i=0; i<sz; i++) { t=t+data_[i]; }
  return t;
}

double Matrix::sumabs() const {
  int sz=m_*n_;
  if (sz<1) return 0.0;
  double t=0.0;
  for (int i=0; i<sz; i++) { t=t+std::abs(data_[i]); }
  return t;
}

int Matrix::num_non_zero() const {
  int sz=m_*n_;
  if (sz<1) return 0;
  int k=0;
  for (int i=0; i<sz; i++) { if (data_[i]!=0.0) k++; }
  return k;
}

int Matrix::num_non_negative() const {
  int sz=m_*n_;
  if (sz<1) return 0;
  int k=0;
  for (int i=0; i<sz; i++) { if (data_[i]>=0.0) k++; }
  return k;
}

double Matrix::norm2_as_vector() const {
  int mn=m_*n_;
  double sum=0.0;
  for (int i=0; i<mn; i++) { sum += data_[i]*data_[i]; }
  return sum;
}

double Matrix::norm2() const {
  if (m_!=1 && n_!=1) Matrix::xerror(5,"Matrix::norm2()");
  return norm2_as_vector();
}

double Matrix::rowdot(int i,int k) const {
  double sum=0.;
  for (int j=0; j<n_; j++) sum += v_[i][j]*v_[k][j];
  return sum;
}

Matrix Matrix::t() {
  Matrix B(*this);
  clear();
  setup2(B.n_,B.m_);
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = B[j][i];
  return *this;
}

double Matrix::dot (Matrix &B) const {
  if ( (m_!=1 && n_!=1) || (B.m_!=1 && B.n_!=1) )
    Matrix::xerror(2,"Matrix::dot(Matrix)");
  int amn = m_*n_;
  int bmn = B.m_ * B.n_;
  if (amn!=bmn) Matrix::xerror(2,"Matrix.dot(Matrix)");
  double sum=0.0;
  for (int i=0; i<amn; i++) sum += data_[i]*B.data_[i];
  return sum;
}

Matrix Matrix::get_row (int i) const {
  Matrix r(1,n_);
  for (int j=0; j<n_; j++) r[0][j] = data_[i*n_ + j];
  return r;
}

Matrix Matrix::get_column (int j) const {
  Matrix c(m_,1);
  for (int i=0; i<m_; i++) c[i][0] = data_[i*n_ + j];
  return c; }

void Matrix::set_row (int i, double val) {
  if (i<0 || i>=m_) Matrix::xerror(1,"Matrix::set_row(i,val)");
  for (int j=0; j<n_; j++) data_[i*n_ + j] = val;
}

void Matrix::set_column (int j, double val) {
  if (j<0 || j>=n_) Matrix::xerror(1,"Matrix::set_column(j,val)");
  for (int i=0; i<m_; i++) data_[i*n_ + j] = val;
}

void Matrix::set_row (int i, Matrix A) {
  if (n_ != A.n_ || A.m_!=1) Matrix::xerror(1,"Matrix::set_row(i,A)");
  if (i<0 || i>=m_) Matrix::xerror(1,"set_row");
  for (int j=0; j<n_; j++) data_[i*n_ + j] = A[0][j];
}

void Matrix::set_column (int j, Matrix A) {
  if (m_ != A.m_ || A.n_!=1) Matrix::xerror(1,"Matrix::set_column(i,A)");
  if (j<0 || j>=n_) Matrix::xerror(1,"set_column");
  for (int i=0; i<m_; i++) data_[i*n_ + j] = A[i][0];
}

void Matrix::set_row_zero(int i) {
  if (i<0 || i>=m_) Matrix::xerror(1,"Matrix::set_row_zero");
  for (int j=0; j<n_; j++) data_[i*n_ + j] = 0.0;
}

void Matrix::set_column_zero(int j) {
  if (j<0 || j>=n_) Matrix::xerror(1,"Matrix::set_column_zero");
  for (int i=0; i<m_; i++) data_[i*n_ + j] = 0.0;
}

void Matrix::resize(int m,int n) {
  if (m==m_ && n==n_) return;  //nothing to do
  checkdim(m,n);
  Matrix B(*this); //save a copy
  clear();         //discard current contents
  setup2(m,n);     //reconfigure
  if (m_>0 && n_>0) {
    //copy whatever can be copied
    int mm=std::min(m_,B.m_);
    int nn=std::min(n_,B.n_);
    for (int i=0; i<mm; i++)
      for (int j=0; j<nn; j++)
        v_[i][j] = B[i][j];
  }
}

void Matrix::del_row(int r) {
  //shift later rows up one
  for (int i=r; i<m_-1; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = v_[i+1][j];
  resize(m_-1,n_);
}

void Matrix::del_column(int c) {
  //shift later columns left one
  for (int i=0; i<m_; i++)
    for (int j=c; j<n_-1; j++)
      v_[i][j] = v_[i][j+1];
  resize(m_,n_-1);
}

void Matrix::append_rows(const Matrix &B) {
  if (m_ == 0 || n_ == 0) { *this = B; return; }  //append to null matrix
  if (n_ != B.n_) Matrix::xerror(2,"Matrix::append_rows");
  int mm=m_;  //save current row size of *this
  resize(m_ + B.m_ , n_);
  for (int i=0; i<B.m_; i++)
    for (int j=0; j<n_; j++)
      v_[mm+i][j] = B[i][j];
}

void Matrix::prepend_rows(const Matrix &B) {
  if (m_ == 0 || n_ == 0) { *this = B; return; }
  if (n_ != B.n_) Matrix::xerror(2,"Matrix::prepend_rows");
  Matrix C=B;
  C.append_rows(*this);
  *this = C;
}


void Matrix::append_columns(const Matrix &B) {
  if (m_ == 0 || n_ == 0) { *this = B; return; }
  if (m_ != B.m_) Matrix::xerror(2,"Matrix::append_columns");
  int nn=n_;  //save current column size of *this
  resize(m_ , n_ + B.n_);
  for (int i=0; i<m_; i++)
    for (int j=0; j<B.n_; j++)
      v_[i][nn+j] = B[i][j];
}

void Matrix::prepend_columns(const Matrix &B) {
  if (m_ == 0 || n_ == 0) { *this = B; return; }
  if (m_ != B.m_) Matrix::xerror(2,"Matrix::prepend_columns");
  Matrix C=B;
  C.append_columns(*this);
  *this = C;
}

void Matrix::zeros() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = 0.0;
}

void Matrix::ones() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = 1.0;
}

void Matrix::identity() {
  Matrix::zeros();
  int mn = std::min(m_,n_);
  for (int i=0; i<mn; i++) v_[i][i]=1.0;
}

void Matrix::iota() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = i+j+1;
}

void Matrix::iotazero() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = i+j;
}

void Matrix::random() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = Matrix::myrandom();
}

void Matrix::gauss() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = Matrix::mygauss();
}

void Matrix::hilbert() {
  for (int i=0; i<m_; i++)
    for (int j=0; j<n_; j++)
      v_[i][j] = 1.0/double(i+j+1);
}

void Matrix::heat() {
  //a suggested problem size is 11 x 51
  if (m_ == 0 || n_ == 0) return;
  double xx,tt;
  for (int i=0; i<m_; i++) {
    xx = 1.5*double(i)/double(m_-1);
    for (int j=0; j<n_; j++) {
      tt = 1.5*double(j)/double(n_-1);
      v_[i][j] = exp(-(xx-tt)*(xx-tt));
    }
  }
}

void Matrix::laplace() {
  //a suggested problem size is 19 x 21
  if (m_ == 0 || n_ == 0) return;
  double s,t;
  for (int i=0; i<m_; i++) {
    s = 0.5 + 5.0*double(i)/double(m_);
    for (int j=0; j<n_; j++) {
      t = 5.0*double(j)/double(n_);
      v_[i][j] = exp(-s*t);
    }
  }
}

void Matrix::cusp() {
  if (m_ == 0 || n_ == 0) return;
  int mn=m_*n_;  //normally should be a Row or Vector... otherwise punt
  for (int i=0; i<mn; i++) data_[i] = sin(IMP::PI*double(i)/double(m_-1));
}

std::ostream& operator<<(std::ostream& os, const Matrix &A) {
  //A.print();
  int m=A.dim1();
  int n=A.dim2();
  os << "Matrix is " << m << " rows by " << n << " columns:" << std::endl;
  for (int i=0; i<m; i++) {
    for (int j=0; j<n; j++)
      os << std::setw(12) << std::setprecision(6) << A(i,j) << " ";
    os << std::endl;
  }
  os << std::endl;
  return os;
}

void Matrix::print() const {
  std::cout << *this;
}

void Matrix::print_by_row() const {
  std::cout << "Matrix is " << m_ << " rows by " << n_ << " columns:"
            << std::endl;
  for (int i=0; i<m_; i++) {
    for (int j=0; j<n_; j++) {
      if (j%5==0) {
        if (j==0) std::cout         << "Row " << std::setw(4) << i;
        else      std::cout << std::endl << "        ";
      }
      std::cout << std::setw(12) << std::setprecision(6)
                << data_[i*n_+j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Matrix::print_by_column() const {
  std::cout << "Matrix is " << m_ << " rows by " << n_ << " columns:"
            << std::endl;
  for (int j=0; j<n_; j++) {
    for (int i=0; i<m_; i++) {
      if (i%5==0) {
        if (i==0) std::cout         << "Col " << std::setw(4) << j;
        else      std::cout << std::endl << "        ";
      }
      std::cout << std::setw(12) << std::setprecision(6)
                << data_[i*n_+j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Matrix::printA() const {
  if (dmin()==0) { std::cout << "(matrix is empty)" << std::endl; return; }
  double t;
  double small=0.00001;
  int m=dim1();
  int n=dim2();
  std::cout << "Matrix is " << m << " rows by " << n << " columns:"
            << std::endl;
  int mx=m; if (mx>15) mx=15;
  int nn=n; if (nn>6) nn=6;
  for (int i=0; i<mx; i++) {
    std::cout << std::setw(2) << i << ":";
    for (int j=0; j<nn; j++) {
      t=v_[i][j];
      if (std::abs(t)<small) t=0.0;
      std::cout << std::setw(10) << std::setprecision(4) <<  t    << " ";
    }
    if (n>nn) std::cout << "...";
    std::cout << std::endl;
  }
  if (mx<m) std::cout << "          ..." << std::endl;
  std::cout << std::endl;
}

void Matrix::printAb(const Matrix &b) const {  //revise like following routine?
  if (dmin()==0) { std::cout << "(matrix is empty)" << std::endl; return; }
  double t;
  double small=0.00001;
  int m=dim1();
  int n=dim2();
  if (dim1() != b.dim1()) Matrix::xerror(2,"Matrix::printAb");

  std::cout << "Matrix is " << m << " rows by " << n << " columns:"
            << std::endl;
  int mx=m; if (mx>15) mx=15;
  int nn=n; if (nn>5) nn=5;
  for (int i=0; i<mx; i++) {
    std::cout << std::setw(2) << i << ":";
    for (int j=0; j<nn; j++) {
      t=v_[i][j];
      if (std::abs(t)<small) t=0.0;
      std::cout << std::setw(10) << std::setprecision(4) <<  t  << " ";
    }
    if (n>nn) std::cout << "...";

    double bb = b[i][0];
    if (std::abs(bb)<small) bb=0.0;
    std::cout << " = " << std::setw(10) << std::setprecision(4) << bb;
    std::cout << std::endl;
  }

  if (mx<m) std::cout << "          ..." << std::setw(58) << " " << "..."
                      << std::endl;
  std::cout << std::setprecision(0) << std::endl;
}

void Matrix::printAbe(const Matrix &b, const Matrix &e)const {
  if (dmin()==0) { std::cout << "(matrix is empty)" << std::endl; return; }
  double t;
  int m=dim1();
  int n=dim2();
  if (dim1() != b.dim1()) Matrix::xerror(2,"Matrix::printAbe");
  if (dim1() != e.dim1()) Matrix::xerror(2,"Matrix::printAbe");

  std::cout << "Matrix is " << m << " rows by " << n << " columns:"
            << std::endl;

  //format is... (i is in first 2 columns)
  //0000000111111111122222222223333333333444444444455555555556666666666777777777
  //3456789012345678901234567890123456789012345678901234567890123456789012345678
  //:-1.234E-15 -2.345E-14 xxxxxxxxxx yyyyyyyyyyy ... = rrrrrrrrrr +- eeeeeeeeee

  int mx=m; if (mx>15) mx=15;
  int nn=n; if (nn>4) nn=4;
  for (int i=0; i<mx; i++) {
    std::cout << std::setw(2) << i << ":";
    for (int j=0; j<nn; j++) {
      t=v_[i][j];
      std::cout << std::setw(10) << std::setprecision(4) << t << " ";
    }
    if (n>nn) std::cout << "... ";

    double bb = b[i][0];
    std::cout << "= " << std::setw(10) << std::setprecision(4) << bb << " ";

    double ee = e[i][0];
    std::cout << "+- " << std::setw(10) << std::setprecision(4) << ee;
    std::cout << std::endl;
  }

  if (mx<m) {
    for (int j=0; j<nn; j++) std::cout << "    ...    ";
    if (n>nn) std::cout << "... ";
    std::cout << "=    ...     ";
    std::cout << "+-    ...";
    std::cout << std::endl;
  }
  std::cout << std::setprecision(0) << std::endl;
}

void Matrix::printAxb(const Matrix &x, const Matrix &b, int maxrows) const {
  if (dmin()==0) { std::cout << "(matrix is empty)" << std::endl; return; }
  double t;
  double small=0.00001;
  int m=dim1();
  int n=dim2();
  if (dim2() != x.dim1()) Matrix::xerror(2,"Matrix::printAxb");
  if (dim1() != b.dim1()) Matrix::xerror(2,"Matrix::printAxb");

  std::cout << "Matrix is " << m << " rows by " << n << " columns:"
            << std::endl;
  int mx=m; if (n>m) mx=n; if (mx>maxrows) mx=maxrows;
  int nn=n; if (nn>4) nn=4;
  for (int i=0; i<mx; i++) {
    std::cout << std::setw(2) << i << ":";
    if (i<m) {
      for (int j=0; j<nn; j++) {
        t=v_[i][j];
        if (std::abs(t)<small) t=0.0;
        std::cout << std::setw(10) << std::setprecision(4) <<  t    << " ";
      }
      if (n>nn) std::cout << "...";
    } else {
      for (int k=0; k<nn; k++) std::cout << std::setw(10) << "  "  << " ";
      if (n>nn) std::cout << "   ";
    }

    double xx = i<n ? x[i][0] : 0.0;
    if (std::abs(xx)<small) xx=0.0;
    if (i<n)  std::cout << " x" << std::setw(10) << std::setprecision(4) << xx;
    else      std::cout << "  " << std::setw(10) << " ";

    double bb = i<m ? b[i][0] : 0.0;
    if (std::abs(bb)<small) bb=0.0;
    if (i<m) std::cout << " = " << std::setw(10) << std::setprecision(4) << bb;
    std::cout << std::endl;
  }

  if (mx<m) std::cout << "          ...";
  else      std::cout << "             ";
  if (mx<n) std::cout << std::setw(46) << " " << "...      ";
  else      std::cout << std::setw(55) << " ";
  if (mx<m) std::cout << "    ... ";
  std::cout << std::endl;

  std::cout << std::setprecision(0) << std::endl;
  //int junk; std::cout << "OK? "; cin >> junk;
}

Matrix Matrix::compute_star_magnitudes() const {
  Matrix mag(*this);
  if (dmin()==0) return mag;
  mag.mabs();            //absolute values
  double mx = mag.maxabs();   //largest
  if (mx==0.0) mx = 1.0; //handle all-zeroes case
  mag = mag/mx;          //largest is now 1.0
  mag.mlog10();          //logs range from  0.0 to -15 or so
  mag-=1.0;              //logs range from -1.0 to -16 or so
  mag.mabs();            //now all are flipped positive...ranging from 1.0 to 16
  return mag;
}

void Matrix::print_star_magnitudes() const {
  if (dmin()==0) { std::cout << "(matrix is empty)" << std::endl; return; }
  int m=dim1();
  int n=dim2();
  std::cout << "Printing star magnitudes with rows= "
            << m << "  columns= " << n << std::endl;
  double scale = this->maxabs();
  if (scale==0.0) scale = 1.0;
  std::cout << "Magnitude 1 is " << scale << " to >" << scale/10.0 << std::endl;
  Matrix B = this->compute_star_magnitudes();
  int k;

  std::cout << " "; for (int j=0; j<n; j++) std::cout << "-" ;
  std::cout << std::endl;
  for (int i=0; i<m; i++) {
    std::cout << "|";
    for (int j=0; j<n; j++) {
      k = int(B[i][j]);
      if (k<=9) std::cout << std::setw(1) << k;
      else      std::cout << " ";
    }
    std::cout << "|" << std::endl;
  }
  std::cout << " "; for (int j=0; j<n; j++) std::cout << "-" ;
  std::cout << std::endl;
};

//Free functions for Matrix

//supports 2.0+A for example...returns element[i][j] = 2.0+A[i][j]
Matrix operator + (double x, const Matrix &A) { Matrix B(A); B+=x; return B; }

//supports 2.0*A for example...returns element[i][j] = 2.0*A[i][j]
Matrix operator * (double x, const Matrix &A) { Matrix B(A); B*=x; return B; }

//supports 2.0-A for example...returns element[i][j] = 2.0-A[i][j]
Matrix operator - (double x, const Matrix &A) {
  Matrix B(A.dim1() , A.dim2() , x);
  B-=A;
  return B;
}

//returns the transpose of the object.
//Note that the object itself is NOT modified.  To modify the object, use A.t()
Matrix transpose (const Matrix& A) {
  int mm=A.dim1();
  int nn=A.dim2();
  Matrix B(nn,mm);
  for (int i=0; i<nn; i++)
    for (int j=0; j<mm; j++)
      B[i][j] = A[j][i];
  return B;
}

IMPSAXS_END_INTERNAL_NAMESPACE
