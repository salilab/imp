#include <IMP/saxs/internal/Row.h>

Row::Row(int m, const double* a) {
  setup2(1,m);
  for (int i=0; i<m; i++) data_[i] = a[i];
}

Row::Row(double t1, double t2, double t3, double t4, double t5,
         double t6, double t7, double t8, double t9, double t10) {
  int k=Matrix::countargs(t1,t2,t3,t4,t5,t6,t7,t8,t9,t10);
  setup2(1,k);
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

Row::Row(const Diagonal &D) {
  int mm = D.dmin();
  setup2(1,mm);
  for (int i=0; i<mm; i++) data_[i] = D[i];
}

Row Row::operator=(double x) {
  for (int j=0; j<n_; j++) data_[j] = x;
  return *this;
}

Row Row::operator=(const Matrix &A) {
  clear();
  if (A.dim1()>1) Matrix::xerror(2,"Row=Matrix");
  int n=A.dim2();
  setup2(1,n);
  for (int j=0; j<n; j++) data_[j] = A[0][j];
  return *this;
}
