/**
   This code for non-negative least squares (NNLS) is taken from:
   http://www.rejonesconsulting.com/Overview.htm
   Version 2.15 January 2012.

   The rejtrix.h file was split into Matrix, Diagonal, Vector, and Row classes
   The autoregnn that solves the non-negative least squares (NNLS)
   problem is here.

   Matrix.h contains a library of matrix creation and manipulation
   routines for any shape matrix, and a suite of standard solvers
   based on a pseudo-inverse (a.k.a. truncated SVD) treatment when needed.
   It also contains slightly modified versions of decomposition routines
   provided by Mathworks, NIST, and the Template Numerical Toolkit web site.

   All these packages are freely distributed from the author's web site at
   www.rejonesconsulting.com.
   See that site for usage instructions and tutorials.

   Class diagram...

    [-----Matrix------]  [---Diagonal---]     (base classes)
       ^      ^    ^          ^    ^
       |      .    |          .    .           dashed lines mean "derives from"
       |      .    |          .    .           dotted lines mean "references"
       |      .    |          .    .
    [-Row-]   .  [---Vector-----]  .          (limited to one row or column)
              .         ^          .
              .         .          .

  ------Licensing Notice---------------------------------------
  Copyright (c) 2006, 2008, 2011 Rondall E. Jones, Albuquerque NM, USA.
  As of January 1, 2012, we grant completely free usage of all these
  software packages to any users and all purposes.
  (The included NIST decomposition routines are unlimited release.)
  We do ask that you not remove the author's identification and notes above,
  and when appropriate, reference the author's web site in publications.

  ------Warning Notice-----------------------------------------
  Warning: This software is provided AS IS!
  THERE IS NO WARRANTY OF ANY SORT WHATSOEVER!
  The algorithms contained herein are partly heuristic and/or experimental.
  Inevitably this package contains bugs/errors which are as yet unknown,
  as well as known deficiencies.
  The user must determine if the results of any calculation using this package
  are appropriate for the user's purposes.

  ------Technical note on coding style------------------------
  In matrix.h and rejtrix.h we have emphasized code re-use over efficiency
  in lower level utility routines. This is an attempt to minimize errors,
  maximize the utility of code testing, and improve reliability of the code.

  Specifically, Row and Vector inherit many operations from Matrix,
  at less than optimum efficiency, in order to reuse that one implementation.
  Or, see how Matrix::operator+() uses Matrix::operator+=()
  or how Matrix::prepend_columns() leverages off Matrix::append_columns()
  to make a trivial implementation of prepend_columns at a cost in efficiency.

  Efficiency of the higher level algorithm usually far exceeds in impact
  any such minor issue of efficiency in low level routines.
*/

#ifndef IMPSAXS_INTERNAL_NNLS_H
#define IMPSAXS_INTERNAL_NNLS_H

#include <IMP/saxs/saxs_config.h>

#include "Matrix.h"
#include "Vector.h"

IMPSAXS_BEGIN_INTERNAL_NAMESPACE

//Jones auto-regularization (easier call)
IMPSAXSEXPORT Vector autoreg(const Matrix &A, const Vector &b);

//Jones auto-regularization with nonnegativity (easier call)
IMPSAXSEXPORT Vector autoregn(const Matrix &A, const Vector &b);

//Jones auto-regularization plus non-negativity constraint
IMPSAXSEXPORT Vector autoregnn(const Matrix &A, const Vector &b, bool nonneg);

IMPSAXS_END_INTERNAL_NAMESPACE

#endif /* IMPSAXS_INTERNAL_NNLS_H */
