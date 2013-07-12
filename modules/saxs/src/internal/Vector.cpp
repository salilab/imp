/**
 * \file Vector class for vector representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#include <IMP/saxs/internal/Vector.h>

#include <cmath>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

Vector::Vector(int m, const double* a) {
  setup2(m,1);
  if (m>0) for (int i=0; i<m; i++) data_[i] = a[i];
}

Vector::Vector(const Diagonal &D) {
  int mm = D.dmin();
  setup2(mm,1);
  for (int i=0;  i<mm; i++) data_[i] = D[i];
}

Vector::Vector(double t1, double t2, double t3, double t4, double t5,
               double t6, double t7, double t8, double t9, double t10) {
  int k=Matrix::countargs(t1,t2,t3,t4,t5,t6,t7,t8,t9,t10);
  setup2(k,1);
  data_[0] = t1;
  if (k>=2)  data_[1] = t2;
  if (k>=3)  data_[2] = t3;
  if (k>=4)  data_[3] = t4;
  if (k>=5)  data_[4] = t5;
  if (k>=6)  data_[5] = t6;
  if (k>=7)  data_[6] = t7;
  if (k>=8)  data_[7] = t8;
  if (k>=9)  data_[8] = t9;
  if (k>=10) data_[9] = t10;
}

Vector Vector::operator=(double x) {
  for (int i=0; i<m_; i++) data_[i] = x;
  return *this;
}

Vector Vector::operator=(const Matrix &A) {
  clear();
  if (A.dim2()>1) Matrix::xerror(2,"Vector=Matrix");
  int m = A.dim1();
  setup2(m,1);
  for (int i=0; i<m; i++) data_[i] = A[i][0];
  return *this;
}

Vector Vector::operator = (const Diagonal &D) {
  clear();
  int m = D.dmin();
  setup2(m,1);
  for (int i=0; i<m; i++) data_[i] = D[i];
  return *this;
}

int Vector::imax() const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::imaxabs");
  double t=data_[0]; int k=0;
  for (int i=1; i<sz; i++) { if (data_[i]>t) { t = data_[i]; k=i; } }
  return k;
}

int Vector::imin() const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::iminabs");
  double t=data_[0]; int k=0;
  for (int i=1; i<sz; i++) { if (data_[i]<t) { t = data_[i]; k=i; } }
  return k;
}

int Vector::imaxabs() const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::imaxabs");
  double t=std::abs(data_[0]); int k=0;
  for (int i=1; i<sz; i++)
    if (std::abs(data_[i])>t) {
      t = std::abs(data_[i]); k=i;
    }
  return k;
}

int Vector::iminabs() const {
  int sz=m_*n_;
  if (sz<1) Matrix::xerror(3,"Matrix::iminabs");
  double t=std::abs(data_[0]); int k=0;
  for (int i=1; i<sz; i++)
    if (std::abs(data_[i])<t) {
      t = std::abs(data_[i]); k=i;
    }
  return k;
}

Vector Vector::range(int i, int j) {
  if (i<0 || j<0 || i>=dim1() || j>=dim1() || j<i)
    Matrix::xerror(4,"Vector::range(,)");
  Vector x(j-i+1);
  for (int k=i; k<=j; k++) x[k-i] = data_[k];
  return x;
}

void Vector::push_front(double value) {
  int m = m_+1;
  Vector B(*this);
  clear();
  setup2(m,1);
  for (int i=1; i<m; i++) data_[i] = B[i-1];
  data_[0] = value;
}

void Vector::pop_front() {
  int m = m_-1;
  Vector B(*this);
  clear();
  setup2(m,1);
  for (int i=0; i<m; i++) data_[i] = B[i+1];
}

void Vector::push_end(double value) {
  int m = m_+1;
  Vector B(*this);
  clear();
  setup2(m,1);
  for (int i=0; i<m-1; i++) data_[i] = B[i];
  data_[m-1] = value;
}

void Vector::pop_end() {
  int m = m_-1;
  Vector B(*this);
  clear();
  setup2(m,1);
  for (int i=0; i<m; i++) data_[i] = B[i];
}

void Vector::sort(double *v, double *w, bool carry,
                  int *p, int *q, int m, int n) {
  int i,j,k;
  double temp;
  int itemp;

  //note that if m==n or the arguments are misordered nothing happens.
  if (n < m+10) {
    //do a bubble sort for short sequences
    for (i=m+1; i<=n; i++)
      for (j=i; j>m; j--) {
        if (v[j]>v[j-1]) break;
        temp=v[j]; v[j]=v[j-1]; v[j-1]=temp;
        if (carry) { itemp=p[j]; p[j]=p[j-1]; p[j-1]=itemp; }
      }
    return;
  }

  //for long sequences split them into two and sort each...
  int mid = (m+n)/2;
  sort(v,w,carry,p,q,m,mid);
  sort(v,w,carry,p,q,mid+1,n);

  //then merge the two parts...
  i=m;
  j=mid+1;
  k=m-1;
  while (i<=mid || j<=n) {
    k++;
    if      (i>mid)      { w[k]=v[j]; if (carry) q[k]=p[j]; j++; }
    else if (j>n)        { w[k]=v[i]; if (carry) q[k]=p[i]; i++; }
    else if (v[i]<=v[j]) { w[k]=v[i]; if (carry) q[k]=p[i]; i++; }
    else                 { w[k]=v[j]; if (carry) q[k]=p[j]; j++; }
  }

  //and copy the result back into original arrays
  for (i=m; i<=n; i++) v[i] = w[i];
  if (carry) for (i=m; i<=n; i++) p[i] = q[i];
}

void Vector::sort() {
  int m=(*this).dim1();
  Vector W(m);
  int *p; p=new int;
  int *q; q=new int;
  sort(data_,W.data_,false,p,q,0,m-1);
}

void Vector::sort(int *p) {
  int m=(*this).dim1();
  Vector W(m);
  int *q; q = new int[m];
  sort(data_,W.data_,true,p,q,0,m-1);
}

double Vector::median() const {
  Vector B=*this;
  B.sort();
  int m=B.dim1();
  if (m<1) return 0.0;
  int h=m/2;
  if (2*h==m)
    return 0.5*(B[h-1] + B[h]);
  else
    return B[h];
}

Vector Vector::moving_average(int w) {
  //compute moving average accurately, but sloe
  if (w<=0 || w>m_) Matrix::xerror(7,"Vector::moving_average()");
  int ln=m_-w+1;
  Vector avg(ln);
  double sum;
  for (int i=0; i<ln; i++) {
    sum=0.0;
    for (int j=i; j<i+w; j++) sum += data_[j];
    avg[i] = sum;
  }
  return avg;
}

Vector Vector::moving_average_fast(int w) {
  //compute fast moving average which will have cumulative round-off error
  //cost is about 1 addition per element of the input vector
  if (w<=0 || w>m_) Matrix::xerror(7,"Vector::moving_average()");
  int ln=m_-w+1;
  Vector avg = Vector(ln);

  //do first sum
  double sum=0.0;
  for (int j=0; j<w; j++) sum += data_[j];
  avg[0] = sum;

  //do subsequent sum adjustments
  for (int i=1; i<ln; i++) {
    sum = sum - data_[i-1] + data_[i+w-1];
    avg[i] = sum;
  }
  return avg;
}

IMPSAXS_END_INTERNAL_NAMESPACE
