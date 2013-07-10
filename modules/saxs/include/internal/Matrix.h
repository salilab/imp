#ifndef IMP_MATRIX_H
#define IMP_MATRIX_H

#include <iostream>
#include <algorithm>

#include <math.h>

static const double BIG  =1.0E30;
static const double BIG2 =2.0E30;

class Matrix {
private:
  int m_;
  int n_;
  double* data_;
  double** v_;

  friend class Row;
  friend class Vector;

  //internal use only
  void checkdim(int m, int n) { if (m<0 || n<0) Matrix::xerror(4,"Matrix::checkdim"); return; }
  void setup2(int m, int n);
  void setup() { setup2(m_,n_); }

  void normalize_them(Matrix &B, Matrix &E, int i, double rownorm);

public:
  //static utilities

  //error reporting routine
  static void xerror(int m, const char* who) {
    std::cerr << "Error in routine " << who << std::endl;
    if (m==1)  std::cerr << "Reference Out-of-Bounds!" << std::endl;
    else if (m==2)  std::cerr << "Dimensions do not match!" << std::endl;
    else if (m==3)  std::cerr << "Operation on an empty matrix!" << std::endl;
    else if (m==4)  std::cerr << "Invalid dimensions!" << std::endl;
    else if (m==5)  std::cerr << "Taking vector norm of non-vector! Use matrix norm?" << std::endl;
    else if (m==6)  std::cerr << "Divide by zero!" << std::endl;
    else if (m==7)  std::cerr << "Invalid input parameter" << std::endl;
    else if (m==8)  std::cerr << "Algorithm error" << std::endl;
    else if (m==9)  std::cerr << "Prohibited operation for Rows and Vectors!" << std::endl;
    else if (m==10) std::cerr << "Given row is too long for matrix!" << std::endl;
    else if (m==11) std::cerr << "Invalid argument vector!" << std::endl;
    else if (m==12) std::cerr << "Problem is too large for current limits!" << std::endl;
    else            std::cerr << "Miscellaneous error: " << m << std::endl;
    //prompt();
    exit(1);
  }

  //find the value of roundoff versus 1.0
  static double roundoff() {
    static bool ok=false;
    static double round=1.0e-9;
    if (ok) return round;

    int j=0;
    double one=1.0;
    double two=1.0;
    double three;
    for (int i=0; i<=100; i++)
      { j=i; three=one+two; if (three==one) break; two/=2.0; }
    round = two*2.0; //went one too far
    if (j>=100) round = 1.0E-9;
    ok = true;
    return round;
  }

  //generate a random value between zero and one
  static double myrandom(int reset=0) {
    static int seed=13*13*13;
    if (reset!=0) seed=reset%16384;
    seed=seed*13;       //scramble
    seed=seed%16384;    //chop to 16 bits
    return double(seed)/16384.0;
  }

  //generate an approximately Gaussian random value, mean 0, sigma 1
  static double mygauss() {
    double sum=0.0;
    //for (int i=0; i<9; i++)  sum += (myrandom()-0.5)*2.0*1.732;
    //rms is about 7% too large often... why???  so reduce it...
    for (int i=0; i<9; i++)  sum += (myrandom()-0.5)*2.0*1.62;
    return sum/3.0;
  }

  //count the number of arguments less than BIG
  static int countargs(double t1, double t2, double t3, double t4, double t5, double t6, double t7, double t8, double t9, double t10) {
    if (t2>BIG) return 1;
    if (t3>BIG) return 2;
    if (t4>BIG) return 3;
    if (t5>BIG) return 4;
    if (t6>BIG) return 5;
    if (t7>BIG) return 6;
    if (t8>BIG) return 7;
    if (t9>BIG) return 8;
    if (t10>BIG) return 9;
    return 10;
  }

  //constructors---------------------------------------------------
  //default constructor: 0 by 0 matrix
  Matrix () : m_(0), n_(0) {}

  //construct an m by n matrix (zero filled)
  explicit Matrix (int m, int n) { setup2(m,n); }

  //construct an m by n matrix filled with the value x
  explicit Matrix (int m, int n, double x);

  //construct an m by n matrix; copy the contents from the array a[m][ndim]
  Matrix (int m, int n, int ndim, const double *a);

  //copy constructor
  Matrix (const Matrix &A);

  //destructors----------------------------------------------------

  //delete all data and set size to 0 by 0
  void clear() {
    if (m_>0 && n_>0) { delete [] data_; delete [] v_; }
    m_ = 0;
    n_ = 0;
  }

  ~Matrix () { clear(); }

  //assignment-----------------------------------------------------

  //supports for example B = 3.14;
  Matrix operator=(const double x);

  //supports for example B = A;
  Matrix operator=(const Matrix &A);

  //accessors------------------------------------------------------

  //get the row dimension
  int dim1() const { return m_; }

  //get the column dimension
  int dim2() const { return n_; }

  //get the smaller dimension
  int dmin() const { return m_ < n_ ? m_ : n_ ; }

  //get the larger dimension
  int dmax() const { return m_ > n_ ? m_ : n_ ; }

  //get the 2-D size
  int dsize() const { return m_ * n_ ; }

  //get the 2-dimensional array representing the matrix
  void get_array(double* A) { int sz=m_*n_; for (int i=0; i<sz; i++) A[i]=data_[i]; }

  //see if the two matrices have matching dimensions ... A.matching(B)
  bool matches(const Matrix &B) const
  { return m_==B.m_ && n_==B.n_; }

  //index----------------------------------------------------------

  inline double* operator[] (int i) {
    if (i < 0 || i >= m_) { Matrix::xerror(1,"operator[]"); }; //DELETE for no debug
    return v_[i];
  }

  inline const double* operator[] (int i) const {
    if (i < 0 || i >= m_) { Matrix::xerror(1,"operator[]"); }; //DELETE for no debug
    return v_[i];
  }

  //Alternative index form... A(i,j) rather than A[i][j].
  //This checks both the indices for proper range.
  //(The A[i][j] form can only check the first index.)

  double& operator() (int i, int j) {
    if (i < 0 || i >= m_ || j < 0 || j >= n_) Matrix::xerror(1,"operator(,)");
    return v_[i][j];
  }

  const double& operator() (int i, int j) const {
    if (i < 0 || i >= m_ || j < 0 || j >= n_) Matrix::xerror(1,"operator(,)");
    return v_[i][j];
  }

  //equivalence operations-----------------------------------------

  //supports A==B
  bool operator==(const Matrix &B) const;

  //supports A!=B
  bool operator!=(const Matrix &B) const;

  //approximately equal test.
  //Two values are considered approx. equal if they differ by
  //less than tolerance.
  //So this is an absolute test, not relative.
  bool approximate(const Matrix &B, double tolerance) const;

  //element-wise Matrix operations---------------------------------

  //these operations support for example, A+=2.0, A-=2.0, A*=2.0, A/=2.0
  Matrix operator+= (double x)
  {int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]+=x; return *this; }

  Matrix operator-= (double x)
  {int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]-=x; return *this; }
  Matrix operator*= (double x)
  {int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]*=x; return *this; }
  Matrix operator/= (double x)
  {int sz=m_*n_; for (int i=0; i<sz; i++) data_[i]/=x; return *this; }

  //these operations support for example, A+2.0, A-2.0, A*2.0, A/2.0
  Matrix operator+  (double x) const { Matrix C(*this); C+=x; return C; }
  Matrix operator-  (double x) const { Matrix C(*this); C-=x; return C; }
  Matrix operator*  (double x) const { Matrix C(*this); C*=x; return C; }
  Matrix operator/  (double x) const { Matrix C(*this); C/=x; return C; }

  //unary minus--- for B = -A; for example
  Matrix operator- ();

  //these operations support A+=B, A-=B, A*=B, A/=B, which are all element-wise.
  //A and B must have exactly the same shape.
  Matrix operator+= (const Matrix &B);
  Matrix operator-= (const Matrix &B);
  Matrix operator*= (const Matrix &B);

  //these operations support A+B  and A-B, which are all element-wise.
  //A and B must have exactly the same shape.
  Matrix operator+ (const Matrix &B) const;
  Matrix operator- (const Matrix &B) const;

  //the following provides the matrix product A*B,
  //where A's second dimension must equal B's first dimension
  Matrix operator* (const Matrix &B) const;

  //--------

  //the following scales each row of the matrix to unit norm,
  //and carries along B and E. (B usually the RHS; E the error est.)
  void normalize_rows(Matrix &B, Matrix &E);

  //the following scales each row of the matrix to unit norm,
  //and carries along B
  void normalize_rows(Matrix &B)
  { Matrix E(m_,1); normalize_rows(B,E); }

  //the following scales each row of the matrix to unit norm
  void normalize_rows()
  { Matrix B(m_,1); Matrix E(m_,1); normalize_rows(B,E); }

  //--------

  //the following scales each row of the matrix to max element of 1.0
  //and carries along B and E. (B usually the RHS; E the error est.)
  void normalize_rows_max1(Matrix &B, Matrix &E);

  //the following scales each row of the matrix to max element of 1.0,
  //and carries along B
  void normalize_rows_max1(Matrix &B)
  { Matrix E(m_,1); normalize_rows_max1(B,E); }

  //the following scales each row of the matrix to max element of 1.0
  void normalize_rows_max1()
  { Matrix B(m_,1); Matrix E(m_,1); normalize_rows_max1(B,E); }

  //element-wise operations----------------------------------------

  //replaces each element with its absolute value
  void mabs();

  //replaces each element with the square root of its absolute value
  void msqrt();

  //replaces each element with its square
  void msquare();

  //Replaces each element with the base 10 log of its absolute value
  //log(A.maxabs())-30.0 is used for zero elements.
  void mlog10();

  //Replaces each element a with 10^a.
  //That is, with the antilog10 of a.
  void mpow10();

  //makes each element at least x
  void at_least(double x);

  //truncates each number to n digits
  void keep_digits(int n);

  //In the following integer utilities, the values are computed as integers,
  //but the TYPE remains floating point.

  //truncates each element to integer                  -2.6 --> -2.0    2.6 --> 2.0
  void trunc();

  //rounds each element to the nearest integer         -2.6 --> -3.0    2.6 --> 3.0
  void round();

  //rounds each element toward +infinity               -2.6 --> -2.0    2.6 --> 3.0
  void ceil();

  //rounds each element to +1 or -1; zero goes to +1   -2.6 --> -1.0    2.6 --> 1.0
  void signum();

  //rounds each element to +1 or -1; zero stays as 0   -2.6 --> -1.0    2.6 --> 1.0
  void trinity();

  //convert each column to percentages based on the sum of the
  //(absolute values of the) elements of the column
  void to_percentages();

  //min/max/sum functions------------------------------------------------

  //returns the element which is algebraically largest
  double maxval() const;

  //returns the element which is algebraically smallest
  double minval() const;

  //returns the (absolute value of the) element which is largest in absolute value
  double maxabs() const;

  //returns the (absolute value of the) element which is smallest in absolute value
  double minabs() const;

  //returns the smallest element greater than zero
  double minpos() const;

  //returns (imax,jmax) position of element with largest abs value
  void ijmaxabs(int &imax, int &jmax) const;

  //returns the sum of the elements
  double sum() const;

  //returns the sum of the absolute values of the elements
  double sumabs() const;

  //returns the average of the values of the elements
  double average() const { return sum()/(m_*n_); }

  //returns the average of the absolute values of the elements
  double averageabs() const { return sumabs()/(m_*n_); }

  //find a neglible value for *this
  double epsilon() const { return maxabs()*8.0*Matrix::roundoff(); }

  //count the number of non-zero elements
  int num_non_zero() const ;

  //std::cout the number of non-negative elements
  int num_non_negative() const;

  //1-D norms------------------------------------------------------
  //These methods require that the object be 1-dimensional.
  //That is, a Row, a Vector, or a Matrix of size 1 by n, or m by 1.
  //For a row v, norm(v) = sqrt(v * v').

  //returns square(norm(*this))
  double norm2() const;

  //returns norm(*this)
  double norm () const { return sqrt(norm2()); }

  //returns root-mean-square(*this)
  double rms  () const { return sqrt(norm2()/double(std::max(m_,std::max(n_,1)))); }

  //returns the population standard deviation
  double popstddev() const { double a=average(); Matrix d=*this - a; return d.rms(); }

  //returns the sample standard deviation
  double samstddev() const
  { if (m_<2) return 0.0; return popstddev()*sqrt(double(n_)/double(n_-1)); }

  //norms of the elements of the matrix as if it were 1-D ---------
  //These methods NO NOdouble require that the object be 1-dimensional.

  //returns the sum of the squares of all the elements
  double norm2_as_vector() const;

  //returns the square root of the sum of the squares of the elements
  double norm_as_vector() const { return sqrt(norm2_as_vector()); }

  //Frobenius norm is another name for our norm_as_vector
  double Frobenius() const { return sqrt(norm2_as_vector()); }

  //returns root-mean-square of the matrix elements
  double rms_as_vector() const { return sqrt(norm2_as_vector()/double(m_*n_)); }

  //row/column operations-------------------------------------------

  //returns the dot product of two rows of *this
  double rowdot(int i,int k) const;

  //dot product of two equal-length 1-dimensional matrices.
  //dot tolerates any two equal length 1-D matrices: row.row, row.col, col.col
  double dot (Matrix &B) const;

  //returns a row of *this
  Matrix get_row (int i) const;

  //returns a column of *this
  Matrix get_column (int j) const;

  //sets all values in row i to val
  void set_row (int i, double val);

  //sets all values in column j to val
  void set_column (int j, double val);

  //sets row i from a row matrix.  The row sizes must match.
  void set_row (int i, Matrix A);

  //sets column i from a column matrix.  The columns sizes must match.
  void set_column (int j, Matrix A);

  //sets all values in row i to zero
  void set_row_zero(int i);

  //sets all values in column j to zero
  void set_column_zero(int j);

  //matrix shape operations----------------------------------------

  //transposes *this
  Matrix t();

  //resize to smaller or larger
  //keeps upper left content as far as possible; fills with zero
  void resize(int m,int n);

  //deletes row r; decreases matrix size!
  void del_row(int r);

  //deletes column c; decreases matrix size!
  void del_column(int c);

  //add m new rows at the bottom of *this; zero filled
  void add_rows(int m) { resize(m_ + m, n_); }

  //append Matrix B to bottom of *this
  void append_rows(const Matrix &B );

  //prepend the Matrix B to the top of *this
  void prepend_rows(const Matrix &B);

  //add n new columns at the right side of *this
  void add_columns(int n) { resize(m_, n_ + n); };

  //append Matrix B to right side of *this
  void append_columns(const Matrix &B );

  //prepend the Matrix B to the left of *this
  void prepend_columns(const Matrix &B);

  //common matrices------------------------------------------------

  void zeros();   //set *this to all zeros
  void ones();    //set *this to all ones
  void identity();//set *this to identity matrix
  void iota();    //set *this[i][j] = i + j + 1.  In a row that's 1, 2, 3, ...
  void iotazero();//set *this[i][j] = i + j.  In a row that's 0, 1, 2, ...
  void random();  //set *this to random values in (0,1)
  void gauss();   //set *this to random Gaussian, mean 0, standard deviation 1
  void hilbert(); //set *this[i][j] = 1/(i+j+1)
  void heat();    //set to an example heat equation style kernel
  void laplace(); //set to an example inverse Laplace style kernel
  void cusp();    //set to one positive cusp of a sine function

  //following static methods create a Matrix of the given size and
  //call the appropriate routine above to define the elements of the Matrix.

  static Matrix zeros   (int m, int n) { Matrix A(m,n); A.zeros();    return A; }
  static Matrix ones    (int m, int n) { Matrix A(m,n); A.ones();     return A; }
  static Matrix identity(int m, int n) { Matrix A(m,n); A.identity(); return A; }
  static Matrix iota    (int m, int n) { Matrix A(m,n); A.iota();     return A; }
  static Matrix iotazero(int m, int n) { Matrix A(m,n); A.iotazero(); return A; }
  static Matrix random  (int m, int n) { Matrix A(m,n); A.random();   return A; }
  static Matrix gauss   (int m, int n) { Matrix A(m,n); A.gauss();    return A; }
  static Matrix hilbert (int m, int n) { Matrix A(m,n); A.hilbert();  return A; }
  static Matrix heat    (int m, int n) { Matrix A(m,n); A.heat();     return A; }
  static Matrix laplace (int m, int n) { Matrix A(m,n); A.laplace();  return A; }

  //displays-------------------------------------------------------

  //print a rectangular layout with default width
  void print() const;

  //print by row in narrow (less than 80 column) format
  void print_by_row() const;

  //print by column in narrow format
  void print_by_column() const;

  //print a glimpse of the matrix, in an 80-column wide format
  void printA() const;

  //print a glimpse of the matrix and the right hand side vector, b,
  //in an 80-column wide format
  void printAb(const Matrix &b) const;

  //print a glimpse of the matrix and the right hand side vector, b,
  //and an error estimate vector e, in an 80-column wide format
  void printAbe(const Matrix &b, const Matrix &e) const;

  //print a glimpse of the matrix, the solution, x, and the right hand side vector, b,
  //in an 80-column wide format.
  //x and b must be single column or single row matrices, or a Row or Vector.
  //By default, up to 25 rows will be printed.
  void printAxb(const Matrix &x, const Matrix &b, int maxrows=25) const;

  //Compute for each element an Order of Magnitude of 1 to 16 or so.
  //This arrangement follows the way stars are classified:
  //magnitude 1 is from largest to about 1/10th that;
  //magnitude 2 is from 1/10th of the largest to about 1/100th of the largest;
  //etc
  Matrix compute_star_magnitudes() const;

  //show each element as Order of Magnitude 1 to 9 or blank for smaller than 9th magnitude
  void print_star_magnitudes() const;
};

std::ostream& operator<<(std::ostream& os, const Matrix &A);

Matrix operator + (double x, const Matrix &A);

Matrix operator * (double x, const Matrix &A);

Matrix operator - (double x, const Matrix &A);

Matrix transpose (const Matrix& A);

#endif /* IMP_MATRIX_H */
