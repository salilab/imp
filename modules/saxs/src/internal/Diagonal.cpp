/**
 * \file Diagonal class for Diagonal matrix representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#include <IMP/saxs/internal/Diagonal.h>

#include <cmath>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

void Diagonal::setupd(int m, int n) {
  checkdim(m,n);
  m_ = m;
  n_ = n;
  mm_ = m_; if (n_<m_) mm_ = n_;
  if (m_==0 || n_==0) return;

  data_ = new double[mm_];
  for (int i=0; i<mm_; i++) data_[i] = 0.0;
}

Diagonal::Diagonal(int m, int n, double x) {
  setupd(m,n);
  for (int i=0; i<mm_; i++) data_[i] = x;
}

Diagonal::Diagonal(int m, int n, const double *a) {
  setupd(m,n);
  for (int i=0; i<mm_; i++) data_[i] = a[i];
}

Diagonal::Diagonal(const Diagonal &D) {
  int m = D.dim1();
  int n = D.dim2();
  setupd(m,n);
  for (int i=0; i<mm_; i++) data_[i] = D[i];
}

Diagonal::Diagonal(int m, int n, const Diagonal &D) {
  setupd(m,n);
  int mm = std::min(mm_ , D.mm_);
  for (int i=0; i<mm; i++) data_[i] = D[i];
}

Diagonal::Diagonal(const Matrix &A) {
  int m=A.dim1();
  int n=A.dim2();
  if (m==1) {
    setupd(n,n);
    for (int j=0; j<n; j++) data_[j] = A[0][j];
  } else if (n==1) {
    setupd(m,m);
    for (int i=0; i<m; i++) data_[i] = A[i][0];
  } else {
    setupd(m,n);
    for (int i=0; i<mm_; i++) data_[i] = A[i][i];
  }
}

Diagonal Diagonal::operator=(const double x) {
  for (int i=0; i<mm_; i++) data_[i] = x;
  return *this;
}

Diagonal Diagonal::operator=(const Diagonal &D) {
  clear();
  int m = D.dim1();
  int n = D.dim2();
  setupd(m,n);
  for (int i=0; i<mm_; i++) data_[i] = D[i];
  return *this;
}

void Diagonal::resize(int m,int n) {
  Diagonal P=*this;
  setupd(m,n);
  int mm = std::min(mm_ , P.mm_);
  for (int i=0; i<mm; i++) data_[i] = P[i];
}

Diagonal Diagonal::operator=(const Matrix &A) {
  Diagonal E(A);
  *this = E;
  return *this;
}

bool Diagonal::operator==(const Diagonal &D) const {
  if (m_ != D.m_ || n_ != D.n_ ) return false;
  for (int i=0; i<mm_; i++)
    { if (data_[i] != D.data_[i]) return false; }
  return true;
}

bool Diagonal::approximate(const Diagonal &D,
                           double tolerance) const {
  if (m_ != D.m_ || n_ != D.n_ ) return false;
  for (int i=0; i<mm_; i++)
    { if (std::abs(data_[i]-D.data_[i])>tolerance) return false; }
  return true;
}

Diagonal Diagonal::operator += (const Diagonal &D) {
  if (m_!=D.m_ || n_!=D.n_) Matrix::xerror(2,"Diagonal+=Diagonal");
  for (int i=0; i<mm_; i++) data_[i]+=D[i];
  return *this;
}

Diagonal Diagonal::operator -= (const Diagonal &D) {
  if (m_!=D.m_ || n_!=D.n_) Matrix::xerror(2,"Diagonal-=Diagonal");
  for (int i=0; i<mm_; i++) data_[i]-=D[i];
  return *this;
}

Diagonal Diagonal::operator - () const {
  Diagonal D(*this);
  for (int i=0; i<mm_; i++) D[i] = -D[i];
  return D;
}

Diagonal Diagonal::operator + (const Diagonal &D) const {
  Diagonal E(*this);
  E+=D;
  return E;
}

Diagonal Diagonal::operator - (const Diagonal &D) const {
  Diagonal E(*this);
  E-=D;
  return E;
}

Diagonal Diagonal::operator * (const Diagonal &D) const {
  if (n_ != D.m_) Matrix::xerror(2,"Diagonal*Diagonal");
  Diagonal E(m_ , D.n_);
  if (m_==0 || n_==0 || D.m_==0 || D.n_==0) return E;

  int m=std::min(mm_,D.mm_);  //smallest of all 4 dimensions! 2011
  for (int i=0; i<m; i++) E[i] = data_[i]*D[i];
  return E;
}

void Diagonal::mabs()
{ for (int i=0; i<mm_; i++) data_[i]=std::abs(data_[i]); }

void Diagonal::msqrt()
{ for (int i=0; i<mm_; i++) data_[i]=sqrt(std::abs(data_[i])); }

void Diagonal::msquare()
{ for (int i=0; i<mm_; i++) data_[i]=data_[i]*data_[i]; }

void Diagonal::mlog10() {
  double tiny = log10((*this).maxabs()) - 30.0;
  for (int i=0; i<mm_; i++)
    if (data_[i]!=0.0) data_[i]=log10(std::abs(data_[i]));
    else               data_[i]=tiny;
}

void Diagonal::mpow10() {
  double scale=log(10.0);
  for (int i=0; i<mm_; i++) data_[i]=exp(scale*data_[i]);
}

void Diagonal::at_least(double x)
{ for (int i=0; i<mm_; i++) if (data_[i]<x) data_[i]=x; }

double Diagonal::maxabs() const {
  if (mm_<1) return 0.0;
  double t=std::abs(data_[0]);
  for(int i=1; i<mm_; i++) { if (std::abs(data_[i])>t) t = std::abs(data_[i]); }
  return t;
}

double Diagonal::minabs() const {
  if (mm_<1) return 0.0;
  double t=std::abs(data_[0]);
  for(int i=1; i<mm_; i++) { if (std::abs(data_[i])<t) t = std::abs(data_[i]); }
  return t;
}

int Diagonal::imaxabs() const {
  if (mm_<1) Matrix::xerror(3,"Diagonal::imaxabs");
  double t=std::abs(data_[0]); int k=0;
  for (int i=1; i<mm_; i++)
    if (std::abs(data_[i])>t) {
      t = std::abs(data_[i]); k=i;
    }
  return k;
}

int Diagonal::iminabs() const {
  if (mm_<1) Matrix::xerror(3,"Diagonal::iminabs");
  double t=std::abs(data_[0]); int k=0;
  for (int i=1; i<mm_; i++)
    if (std::abs(data_[i])<t) {
      t = std::abs(data_[i]); k=i;
    }
  return k;
}

int Diagonal::ilastnz() const {
  if (mm_<1) Matrix::xerror(3,"Diagonal::iminabs");
  int k=-1;
  for (int i=0; i<mm_; i++) { if (data_[i]!=0.0) k=i; }
  return k;
}

double Diagonal::trace() const {
  double t=0.0;
  for (int i=0; i<mm_; i++) t += data_[i];
  return t;
}

double Diagonal::epsilon() const { return maxabs()*8.0*Matrix::roundoff(); }

int Diagonal::plateau() const {
  double eps = 10.0*epsilon();
  double seps = sqrt(eps);

  int p=0;
  for (int i=mm_-1; i>0; i--) {
    if (data_[i-1]>eps && data_[i-1]>(1.0+seps)*data_[i]) break;
    p++;
  }
  if (p>=mm_-1) p=0;  //for a constant set of singular values.
  return p;
}

void Diagonal::zeros()
{ for (int i=0; i<mm_; i++) data_[i]=0.0; }

void Diagonal::ones()
{ for (int i=0; i<mm_; i++) data_[i]=1.0; }

void Diagonal::identity()
{ for (int i=0; i<mm_; i++) data_[i]=1.0; }

void Diagonal::iota()
{ for (int i=0; i<mm_; i++) data_[i]=i+1; }

void Diagonal::random()
{ for (int i=0; i<mm_; i++) data_[i]=Matrix::myrandom(); }

void Diagonal::gauss()
{ for (int i=0; i<mm_; i++) data_[i]=Matrix::mygauss(); }

std::ostream& operator<<(std::ostream& os, const Diagonal &D) {
  int m=D.dim1();
  int n=D.dim2();
  int mm=D.dmin();
  os << "Matrix is Diagonal of size " << m << " rows by "
     << n << " columns:" << std::endl;
  for (int i=0; i<mm; i++) {
    for (int j=0; j<i; j++) os << " ";
    os << D[i] << " " << std::endl;
  }
  for (int i=mm; i<m; i++) {
    for (int j=0; j<i; j++) os << " ";
    os << "..." << std::endl;
  }
  os << std::endl;
  return os;
}

void Diagonal::print() const { std::cout << *this; }

//Free functions for Diagonal-------------------------------

//supports 2.0+D for example...returns element[i][j] = 2.0+D[i][j]
Diagonal operator + (double x, const Diagonal &D)
{ Diagonal B(D); B+=x; return B; }

//supports 2.0*D for example...returns element[i][j] = 2.0*D[i][j]
Diagonal operator * (double x, const Diagonal &D)
{ Diagonal B(D); B*=x; return B; }

//supports 2.0-D for example...returns element[i][j] = 2.0-D[i][j]
Diagonal operator - (double x, const Diagonal &D) {
  Diagonal B(D.dim1() , D.dim2() , x);
  B-=D;
  return B;
}

//returns a Diagonal which is the transpose of the argument.
//The argument is not changed.
Diagonal transpose (const Diagonal &D) {
  Diagonal B(D);
  B.t();
  return B;
}

//returns the pseudo-inverse of a Diagonal matrix D.
//Values below or near roundoff times the largest (magnitude) diagonal
//element are considered to be zero
Diagonal pseudoinverse(const Diagonal &D) {
  int mm=D.dmin();
  if (mm<1) Matrix::xerror(3,"pseudoinverse(diagonal)");
  Diagonal S=transpose(D);
  double eps = D.epsilon();
  for (int i=0; i<mm; i++)
    if (S[i] > eps) S[i] = 1.0/S[i];
    else            S[i] = 0.0;
  return S;
}

//returns the smoothed (regularized) pseudo-inverse of a Diagonal matrix D.
//Values below or near roundoff times the largest (magnitude) diagonal
//element are considered to be zero
Diagonal smoothinverse(const Diagonal &S, double lambda) {
  int m=S.dmin();
  if (m<1) Matrix::xerror(3,"smoothinverse(diagonal)");

  if (lambda==0.0) return pseudoinverse(S);
  double lambda2 = lambda*lambda;
  Diagonal P=transpose(S);
  for (int i=0; i<m; i++) P[i] = S[i]/(S[i]*S[i] + lambda2);

  double eps = S.epsilon();
  double large = 1.0/eps;
  for (int i=0; i<m; i++) if (P[i]>large) P[i] = 0.0;

  return P;
}

//Tikhonov regularize the Matrix A with given lambda.
//Seldom necessary. Use the above when possible.
Diagonal regularize(const Diagonal &S, double lambda) {
  int m=S.dim1();
  int n=S.dim2();
  if (m==0 || n==0) return Diagonal(m,n);
  if (lambda<=0.0) return S;

  double lambda2=lambda*lambda;
  double eps = S.epsilon();
  Diagonal P=S;
  for (int i=0; i<P.dmin(); i++) {
    if (S[i]>eps) P[i] = (S[i]*S[i] + lambda2)/S[i];
    else          P[i] = 0.0;
  }
  return P;
}

//returns the condition number of a Diagonal matrix D.
double condition_number(const Diagonal &D) {
  int mm=D.dmin();
  if (mm<1) Matrix::xerror(3,"condition_number(diagonal)");
  double a=std::abs(D[0]);
  double big = a;
  double small = a;
  for (int i=1; i<mm; i++) {
    a=std::abs(D[i]);
    if (a>big) big=a;
    if (a<small) small=a;
  }
  if (big==0.0) return 1.0/Matrix::roundoff();
  if (small==0.0) return 1.0/D.epsilon();
  return big/small;
}

//returns the condition number of a Diagonal matrix ignoring zero
// or near-zero values
double condition_number_nonzero(const Diagonal &D) {
  int mm=D.dmin();
  if (mm<1) Matrix::xerror(3,"condition_number(diagonal)");
  double eps=D.epsilon();
  double a=std::abs(D[0]);
  double big = a;
  double small = a;
  for (int i=1; i<mm; i++) {
    a=std::abs(D[i]);
    if (a<=eps) continue;
    if (a>big) big=a;
    if (a<small) small=a;
  }
  if (big==0.0) return 1.0/Matrix::roundoff();
  if (small==0.0) return 1.0/D.epsilon();
  return big/small;
}

//Matrix * Diagonal
Matrix operator * (const Matrix &A, const Diagonal &D) {
  int am = A.dim1();
  int an = A.dim2();

  int mm = D.dmin();
  int bm = D.dim1();
  int bn = D.dim2();
  if (an != bm) Matrix::xerror(2,"Matrix*Diagonal");

  Matrix C(am , bn);
  for (int j=0; j<mm; j++)
    for (int i=0; i<am; i++)
      C[i][j]=A[i][j] * D[j];
  //any extra columns remain at 0.0
  return C;
}

//Diagonal * Matrix
Matrix operator * (const Diagonal &D , const Matrix &B) {
  int am = D.dim1();
  int an = D.dim2();
  int mm = D.dmin();

  int bm = B.dim1();
  int bn = B.dim2();
  if (an != bm) Matrix::xerror(2,"Diagonal*Matrix");

  Matrix C(am , bn);
  for (int i=0; i<mm; i++)
    for (int j=0; j<bn; j++)
      C[i][j] = D[i] * B[i][j];
  //any extra rows remain at 0.0
  return C;
}

//create a full Matrix of the same shape as D
//with diagonal elements taken from D and zeros otherwise
Matrix full(const Diagonal &D) {
  int m=D.dim1();
  int n=D.dim2();
  Matrix A(m,n);
  int mm=D.dmin();
  for (int i=0; i<mm; i++) A[i][i] = D[i];
  return A;
}

IMPSAXS_END_INTERNAL_NAMESPACE
