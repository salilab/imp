/**
 * \file Row class for matrix row representation for NNLS computation
 *
 * Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
 * see nnls.h for details
 *
 */

#include <IMP/saxs/internal/Row.h>

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

Row::Row(int m, const double* a) {
  setup2(1,m);
  for (int i=0; i<m; i++) data_[i] = a[i];
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

IMPSAXS_END_INTERNAL_NAMESPACE
