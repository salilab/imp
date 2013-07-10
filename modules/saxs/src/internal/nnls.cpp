#include <IMP/saxs/internal/nnls.h>

#include <IMP/saxs/internal/Row.h>

#include <cmath>

namespace {

//***********************************************************************
//Decomposition routines adapted from Template Numerical Toolkit.
//***********************************************************************
//Following matrix decomposition routines for SVD, QR, and Eigenvalue
//are adapted from http://math.nist.gov/tnt/index.html.
//These are public domain routines.
//My thanks to the authors for this nice public domain package.

//decide on the moving average width for Picard condition determination
int decide_width(int p) {
  int w = 1;
  if (p>4) w = 2;
  if (p>10) w = 4;
  if (p>100) w = 6;
  return w;
}

double hypot2(double a, double b) {
  if (std::abs(a)<std::abs(b)) {
    double c = a/b;
    return std::abs(b) * sqrt(1.0 + c*c);
  } else {
    if (a==0.0) return 0.0;
    double c = b/a;
    return std::abs(a) * sqrt(1.0 + c*c);
  }
}

//find sigma given a useable rank, ur
double findsigma(const Matrix &A, const Vector &b, int ur,
                 const Matrix &U, const Diagonal &s, const Matrix &V) {
  int m=A.dim1();
  if (m-ur<1) return 0.0;
  Diagonal ps=pseudoinverse(s);
  int mm=ps.dmin();
  for (int i=ur; i<mm; i++) ps[i] = 0.0;
  Vector xur = V * ps * transpose(U) * b;
  Vector res = b-A*xur;
  return sqrt(res.norm2()/double(m-ur));
}

//for internal use...returns the RMS residual after computing the
//regularized solution with given lambda
double rmslambda(const Matrix &A, const Vector &b, Vector &x, int ur,
                 double lambda,
                 const Matrix &U, const Diagonal &s, const Matrix &V) {
  if (ur<=0) ur=A.dmin();
  if (ur>A.dmin()) ur=A.dmin();

  double lambda2=lambda*lambda;
  Diagonal ps=transpose(s);
  double denom;
  double eps=s.epsilon();
  for (int i=0; i<ur; i++) {
    denom=s[i]*s[i]+lambda2;
    if (denom>eps) ps[i] = s[i]/denom;
    else           ps[i] = 0.0;
  }
  int m=A.dmin();  //max rank of A
  for (int i=ur; i<m; i++) ps[i] = 0.0;  //was for (int i=ur; i<m-1; i++) 2011

  x = V * ps * transpose(U) * b;
  Vector res = b-A*x;
  double r=res.rms();
  return r;
}

//find the lambda that fits this sigma, and the associated solution
Vector discrep(const Matrix &A, const Vector &b, int ur, double sigma,
               double &lambda,
               const Matrix &U, const Diagonal & s, const Matrix &V) {
  if (ur>A.dmin()) ur=A.dmin();
  if (ur<=0) ur=A.dmin();  //default

  Vector x;
  lambda = 0.0;
  double check = rmslambda(A,b,x,ur,lambda,U,s,V);
  if (check>=sigma) return x;  //covers special cases

  double lo=0.;          //no extra residual
  //residual probably too large ... if not, that's all we'll do!
  double hi=0.33*s[0];

  //bisect until hopefully we nail exactly the residual we want...
  //but quit eventually
  for (int k=0; k<50; k++) {
    lambda = (lo+hi)*0.5;
    check = rmslambda(A,b,x,ur,lambda,U,s,V);
    if (std::abs(check-sigma) < 0.0001*sigma) break;
    if (check>sigma) hi=lambda;
    else             lo=lambda;
  }
  return x;
}

//the SVD decomposition----------------------------------------------
static void svd (const Matrix& Arg, Matrix& U, Diagonal& s, Matrix& V) {

  //Singular Value Decomposition.
  //For an m-by-n matrix A with m >= n, the singular value decomposition is
  //an m-by-n orthogonal matrix U, an n-by-n diagonal matrix S, and
  //an n-by-n orthogonal matrix V so that A = U*S*V'.

  //The singular values, sigma[k] = S[k][k], are ordered so that
  //sigma[0] >= sigma[1] >= ... >= sigma[n-1].

  //The singular value decompostion always exists, so the constructor will
  //never fail.  The matrix condition number and the effective numerical
  //rank can be computed from this decomposition.
  //(Adapted from JAMA, a Java Matrix Library, developed by jointly
  //by the Mathworks and NIST; see  http://math.nist.gov/javanumerics/jama).

  //Must have m>=n
  //Sizes must be Arg(m,n)  U(m,n)   s(m+1)   V(n,n)

  int m = Arg.dim1();
  int n = Arg.dim2();
  if (m<n) Matrix::xerror(4,"svd()");
  if (m<n) return;

  Matrix A;
  A=Arg;
  U = Matrix(m,n,0.0);
  V = Matrix(n,n,0.0);
  s = Diagonal(n);

  double *e; e=new double[n];
  double *work; work = new double[m];
  int nu = std::min(m,n);

  //---end of interface changes

  int wantu = 1;  					// boolean
  int wantv = 1;  					// boolean
  int i=0, j=0, k=0;

  // Reduce A to bidiagonal form, storing the diagonal elements
  // in s and the super-diagonal elements in e.
  int nct = std::min(m-1,n);
  int nrt = std::max(0,std::min(n-2,m));
  for (k = 0; k < std::max(nct,nrt); k++) {
    if (k < nct) {
      // Compute the transformation for the k-th column and
      // place the k-th diagonal in s[k].
      // Compute 2-norm of k-th column without under/overflow.
      s[k] = 0.0;
      for (i = k; i < m; i++) s[k] = hypot2(s[k],A[i][k]);
      if (s[k] != 0.0) {
        if (A[k][k] < 0.0) s[k] = -s[k];
        for (i = k; i < m; i++) A[i][k] /= s[k];
        A[k][k] += 1.0;
      }
      s[k] = -s[k];
    }

    for (j = k+1; j < n; j++) {
      if ((k < nct) && (s[k] != 0.0))  {
        // Apply the transformation.
        double t = 0;
        for (i = k; i < m; i++) t += A[i][k]*A[i][j];
        t = -t/A[k][k];
        for (i = k; i < m; i++) A[i][j] += t*A[i][k];
      }

      // Place the k-th row of A into e for the
      // subsequent calculation of the row transformation.
      e[j] = A[k][j];
    }

    if (wantu && (k < nct)) {
      // Place the transformation in U for subsequent back multiplication.
      for (i = k; i < m; i++) U[i][k] = A[i][k];
    }

    if (k < nrt) {
      // Compute the k-th row transformation and place the
      // k-th super-diagonal in e[k].
      // Compute 2-norm without under/overflow.
      e[k] = 0;
      for (i = k+1; i < n; i++) e[k] = hypot2(e[k],e[i]);
      if (e[k] != 0.0) {
        if (e[k+1] < 0.0) e[k] = -e[k];
        for (i = k+1; i < n; i++) e[i] /= e[k];
        e[k+1] += 1.0;
      }
      e[k] = -e[k];
      if ((k+1 < m) && (e[k] != 0.0)) {
        // Apply the transformation.
        for (i = k+1; i < m; i++) work[i] = 0.0;
        for (j = k+1; j < n; j++) {
          for (i = k+1; i < m; i++) {
            work[i] += e[j]*A[i][j];
          }
        }
        for (j = k+1; j < n; j++) {
          double t = -e[j]/e[k+1];
          for (i = k+1; i < m; i++) {
            A[i][j] += t*work[i];
          }
        }
      }
      if (wantv) {
        // Place the transformation in V for subsequent back multiplication.
        for (i = k+1; i < n; i++) V[i][k] = e[i];
      }
    }
  }

  // Set up the final bidiagonal matrix or order p.
  int p = std::min(n,m+1);
  if (nct < n) s[nct] = A[nct][nct];
  if (m < p) s[p-1] = 0.0;
  if (nrt+1 < p) e[nrt] = A[nrt][p-1];
  e[p-1] = 0.0;

  // If required, generate U.
  if (wantu) {
    for (j = nct; j < nu; j++) {
      for (i = 0; i < m; i++) U[i][j] = 0.0;
      U[j][j] = 1.0;
    }
    for (k = nct-1; k >= 0; k--) {
      if (s[k] != 0.0) {
        for (j = k+1; j < nu; j++) {
          double t = 0;
          for (i = k; i < m; i++) t += U[i][k]*U[i][j];
          t = -t/U[k][k];
          for (i = k; i < m; i++) U[i][j] += t*U[i][k];
        }
        for (i = k; i < m; i++ ) U[i][k] = -U[i][k];
        U[k][k] = 1.0 + U[k][k];
        for (i = 0; i < k-1; i++) U[i][k] = 0.0;
      } else {
        for (i = 0; i < m; i++) U[i][k] = 0.0;
        U[k][k] = 1.0;
      }
    }
  }

  // If required, generate V.
  if (wantv) {
    for (k = n-1; k >= 0; k--) {
      if ((k < nrt) && (e[k] != 0.0)) {
        for (j = k+1; j < nu; j++) {
          double t = 0;
          for (i = k+1; i < n; i++) t += V[i][k]*V[i][j];
          t = -t/V[k+1][k];
          for (i = k+1; i < n; i++) V[i][j] += t*V[i][k];
        }
      }
      for (i = 0; i < n; i++) V[i][k] = 0.0;
      V[k][k] = 1.0;
    }
  }

  // Main iteration loop for the singular values.
  int pp = p-1;
  int iter = 0;
  double eps = pow(2.0,-52.0);
  while (p > 0) {
    int k=0;
    int kase=0;

    // Here is where a test for too many iterations would go.

    // This section of the program inspects for
    // negligible elements in the s and e arrays.  On
    // completion the variables kase and k are set as follows.

    // kase = 1     if s(p) and e[k-1] are negligible and k<p
    // kase = 2     if s(k) is negligible and k<p
    // kase = 3     if e[k-1] is negligible, k<p, and
    //              s(k), ..., s(p) are not negligible (qr step).
    // kase = 4     if e(p-1) is negligible (convergence).

    for (k = p-2; k >= -1; k--) {
      if (k == -1) break;
      if (std::abs(e[k]) <= eps*(std::abs(s[k]) + std::abs(s[k+1]))) {
        e[k] = 0.0;
        break;
      }
    }
    if (k == p-2) {
      kase = 4;
    } else {
      int ks;
      for (ks = p-1; ks >= k; ks--) {
        if (ks == k) break;
        double t = (ks != p ? std::abs(e[ks]) : 0.) +
                   (ks != k+1 ? std::abs(e[ks-1]) : 0.);
        if (std::abs(s[ks]) <= eps*t)  {
          s[ks] = 0.0;
          break;
        }
      }
      if (ks == k) {
        kase = 3;
      } else if (ks == p-1) {
        kase = 1;
      } else {
        kase = 2;
        k = ks;
      }
    }
    k++;

    // Perform the task indicated by kase.
    switch (kase) {
      // Deflate negligible s(p).
     case 1: {
       double f = e[p-2];
       e[p-2] = 0.0;
       for (j = p-2; j >= k; j--) {
         double t = hypot2(s[j],f);
         double cs = s[j]/t;
         double sn = f/t;
         s[j] = t;
         if (j != k) {
           f = -sn*e[j-1];
           e[j-1] = cs*e[j-1];
         }
         if (wantv) {
           for (i = 0; i < n; i++) {
             t = cs*V[i][j] + sn*V[i][p-1];
             V[i][p-1] = -sn*V[i][j] + cs*V[i][p-1];
             V[i][j] = t;
           }
         }
       }
     }
       break;

     // Split at negligible s(k).
     case 2: {
       double f = e[k-1];
       e[k-1] = 0.0;
       for (j = k; j < p; j++) {
         double t = hypot2(s[j],f);
         double cs = s[j]/t;
         double sn = f/t;
         s[j] = t;
         f = -sn*e[j];
         e[j] = cs*e[j];
         if (wantu) {
           for (i = 0; i < m; i++) {
             t = cs*U[i][j] + sn*U[i][k-1];
             U[i][k-1] = -sn*U[i][j] + cs*U[i][k-1];
             U[i][j] = t;
           }
         }
       }
     }
       break;

     // Perform one qr step.
     case 3: {
       // Calculate the shift.
       double scale = std::max(std::max(std::max(std::max(
                       std::abs(s[p-1]),std::abs(s[p-2])),std::abs(e[p-2])),
                       std::abs(s[k])),std::abs(e[k]));
       double sp = s[p-1]/scale;
       double spm1 = s[p-2]/scale;
       double epm1 = e[p-2]/scale;
       double sk = s[k]/scale;
       double ek = e[k]/scale;
       double b = ((spm1 + sp)*(spm1 - sp) + epm1*epm1)/2.0;
       double c = (sp*epm1)*(sp*epm1);
       double shift = 0.0;
       if ((b != 0.0) || (c != 0.0)) {
         shift = sqrt(b*b + c);
         if (b < 0.0) {
           shift = -shift;
         }
         shift = c/(b + shift);
       }
       double f = (sk + sp)*(sk - sp) + shift;
       double g = sk*ek;

       // Chase zeros.
       for (j = k; j < p-1; j++) {
         double t = hypot2(f,g);
         double cs = f/t;
         double sn = g/t;
         if (j != k) e[j-1] = t;
         f = cs*s[j] + sn*e[j];
         e[j] = cs*e[j] - sn*s[j];
         g = sn*s[j+1];
         s[j+1] = cs*s[j+1];
         if (wantv) {
           for (i = 0; i < n; i++) {
             t = cs*V[i][j] + sn*V[i][j+1];
             V[i][j+1] = -sn*V[i][j] + cs*V[i][j+1];
             V[i][j] = t;
           }
         }
         t = hypot2(f,g);
         cs = f/t;
         sn = g/t;
         s[j] = t;
         f = cs*e[j] + sn*s[j+1];
         s[j+1] = -sn*e[j] + cs*s[j+1];
         g = sn*e[j+1];
         e[j+1] = cs*e[j+1];
         if (wantu && (j < m-1)) {
           for (i = 0; i < m; i++) {
             t = cs*U[i][j] + sn*U[i][j+1];
             U[i][j+1] = -sn*U[i][j] + cs*U[i][j+1];
             U[i][j] = t;
           }
         }
       }
       e[p-2] = f;
       iter = iter + 1;
     }
       break;

     // Convergence.
     case 4: {
       // Make the singular values positive.
       if (s[k] <= 0.0) {
         s[k] = (s[k] < 0.0 ? -s[k] : 0.0);
         if (wantv) {
           for (i = 0; i <= pp; i++) {
             V[i][k] = -V[i][k];
           }
         }
       }

       // Order the singular values.
       while (k < pp) {
         if (s[k] >= s[k+1]) {
           break;
         }
         double t = s[k];
         s[k] = s[k+1];
         s[k+1] = t;
         if (wantv && (k < n-1)) {
           for (i = 0; i < n; i++) {
             t = V[i][k+1]; V[i][k+1] = V[i][k]; V[i][k] = t;
           }
         }
         if (wantu && (k < m-1)) {
           for (i = 0; i < m; i++) {
             t = U[i][k+1]; U[i][k+1] = U[i][k]; U[i][k] = t;
           }
         }
         k++;
       }
       iter = 0;
       p--;
     }
       break;
    }
  };
}

//Jones auto-regularization with SVD provided
Vector autoregusv(const Matrix &A, const Vector &b, int &ur,  double &sigma,
                  double &lambda, const Matrix &U, const Diagonal &s,
                  const Matrix &V) {
  ur = 0;
  sigma = 0.0;
  lambda = 0.0;

  int m=A.dim1();
  int n=A.dim2();
  int mb=b.dim1();
  if (m != mb) Matrix::xerror(4,"Rmatrix::autoregusv");
  if (m==0 || n==0) Matrix::xerror(3,"Rmatrix::autoregusv");

  Vector beta=transpose(U)*b;
  int p = U.dim2();              //smaller of m and n
  int w = decide_width(p);

  //compute contributions to norm of solution
  Vector g(p);
  int k=0;      //the rank of A
  g[0] = 0.0;
  for (int i=0; i<p; i++) {
    if (s[i]<=0.0) break;
    g[i]=std::abs(beta[i]/s[i]);
    k = i+1;
  }

  if (k<=0 || g[0]==0.0) return Vector(n); //zero system
  ur = k;  //useable rank

  //scale g so overflow does not happen in g^2 calculations
  double mxg = g.maxabs();
  if (mxg>1.0E10) g /= g.maxabs();

  if (w<k) {
    //compute moving average
    double sum;
    Vector avg(k-w+1);
    for (int i=0; i<k-w+1; i++) {
      sum=0.0;
      for (int j=i; j<i+w; j++) sum += g[j]*g[j];
      avg[i] = sum;
    }

    //find low point of moving average
    //normally don't allow min to be at avg[0]
    int start=0; if (w>1) start=1;
    Vector see = avg.range(start,avg.dim1()-1);
    int ilow = see.iminabs() + start;
    double low = avg[ilow];

    //see if the moving average ever gets much larger
    double bad = 15.0*low;
    int ibad = 0;
    for (int i=ilow+1; i<k-w+1; i++) if (avg[i] > bad) { ibad=i; break; }

    if (ibad>0) {
      //find where it gets a little larger
      bad = 3.0*low;
      ibad = ilow+1;
      for (int i=ilow+1; i<k-w+1; i++)
        if (avg[i] > bad) {ibad = i; break;}

      //find largest value in this width
      double worst = g[ibad]*g[ibad];
      int iworst = ibad;
      for (int i=ibad; i<ibad+w; i++) {
        if (g[i]*g[i] > worst) {
          worst = g[i]*g[i];
          iworst = i;
        }
      }
      ur = iworst;

      //then backtrack
      for (int i=ur-1; i>ilow; i--) {
        if (g[i-1]*g[i-1] > g[i]*g[i]) break;
        ur = i;
      }
    }
  }

  if (ur < p) sigma = findsigma(A,b,ur,U,s,V); //desired sigma
  double sigmin = (b-U*beta).rms(); //minimum achievable sigma
  Vector x;
  double cond = condition_number(s);
  if (sigma==0.0 || sigma < sigmin*1.01 || cond<10.0) {
    if (cond<10.0) ur = p;
    x = V * pseudoinverse(s) * transpose(U) * b;
  } else {
    x = discrep(A,b,ur,sigma,lambda,U,s,V);
  }
  return x;
}

//returns a "compact" SVD of A
void mysvd(const Matrix &A, Matrix &U, Diagonal &S, Matrix &V) {
  int m=A.dim1();
  int n=A.dim2();
  if (m*n<1) { U.clear(); S.clear(); V.clear(); return; }

  if (m<n) {
    Matrix AT=transpose(A);
    svd(AT,V,S,U);
  } else {
    svd(A,U,S,V);
  }
}

double estimate_solution_norm(const Matrix &A, const Vector &B) {
  if (A.dim1() != B.dim()) Matrix::xerror(2,"RMatrix::initial_estimate");
  int m = A.dim1();
  int n = A.dim2();
  Vector X(n,0.0);

  double r,factor,x;
  int i,j;

  //each row gives a 1-equation underdetermined estimate of the solution.
  //compute all m such estimates...
  for (i=0; i<m; i++) {
    Row R=A.get_row(i);
    r = R.norm2();
    if (r==0.0 || B[i]==0.0) continue;

    factor = B[i]/r;
    for (j=0; j<n; j++) {
      x = R[j]*factor;
      if (std::abs(x)>std::abs(X[j])) X[j] = x;
    }
  }
  return X.norm();
}

//Tikhonov regularization with given lambda
Vector regularize(const Matrix &A, const Matrix &B, double lambda) {
  int m=A.dim1();
  int n=A.dim2();
  int mb=B.dim1();
  if (m != mb) Matrix::xerror(4,"Rmatrix::regularize");
  if (m==0 || n==0) return Vector(n);

  Matrix U,V;
  Diagonal S;
  mysvd(A,U,S,V);
  Diagonal P=smoothinverse(S,lambda);
  Vector x = V * P * transpose(U) * B;
  return x;
}

}

Vector autoregnn(const Matrix &A, const Vector &b, bool nonneg)
  {
    int m=A.dim1();
    int n=A.dim2();
    int mb=b.dim1();
    if (m != mb) Matrix::xerror(4,"Rmatrix::autoregnn");
    if (m==0 || n==0) { Vector x(n,1); return x; }

    if (m<1 || n<1) return Vector(n);
    Matrix C=A;
    Matrix U,V;
    Diagonal s;
    mysvd(C,U,s,V);

    //initial solution
    int ur;
    double sigma;
    double lambda;
    Vector x=autoregusv(C,b,ur,sigma,lambda,U,s,V);

    if (!nonneg) return x;

    //compute a small negative tolerance
    double tol = -estimate_solution_norm(A,b) * sqrt(Matrix::roundoff());

    //count initial negatives
    int negs=0; for (int j=0; j<n; j++) if (x[j]<0.0) negs++;
    if (negs<=0) return x;
    int sip = int(negs/100); if (sip<1) sip=1;
    Row zeroed(n);

    //iteratively zero some x values
    for (int kount=1; kount<=n; kount++)  { //loop essentially forever
      //count negatives and choose how many to treat
      negs=0; for (int j=0; j<n; j++) if (zeroed[j]<1.0 && x[j]<tol) negs++;
      if (negs<=0) break;
      int gulp = std::max(negs/20,sip);

      //zero the most negative solution values
      for (int k=1; k<=gulp; k++) {
        int p = -1;
        double worst=0.0;
        for (int j=0; j<n; j++)
          if (zeroed[j]<1.0 && x[j]<worst) {
            p=j; worst=x[p];
          }
        if (p<0) break;
        for (int i=0; i<m; i++) C[i][p]=0.0;
        zeroed[p]=9;
      }

      //re-solve
      x=regularize(C,b,lambda);
    }
    for (int j=0; j<n; j++)
      if (x[j]<0.0 && std::abs(x[j])<=std::abs(tol)) x[j]=0.0;
    return x;
}

Vector autoreg(const Matrix &A, const Vector &b) {
  return autoregnn(A,b,false);
}

Vector autoregn(const Matrix &A, const Vector &b) {
  return autoregnn(A,b,true);
}
