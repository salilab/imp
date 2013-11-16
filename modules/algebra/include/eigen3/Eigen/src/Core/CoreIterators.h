// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMP_EIGEN_COREITERATORS_H
#define IMP_EIGEN_COREITERATORS_H

namespace IMP_Eigen { 

/* This file contains the respective InnerIterator definition of the expressions defined in Eigen/Core
 */

/** \ingroup SparseCore_Module
  * \class InnerIterator
  * \brief An InnerIterator allows to loop over the element of a sparse (or dense) matrix or expression
  *
  * todo
  */

// generic version for dense matrix and expressions
template<typename Derived> class DenseBase<Derived>::InnerIterator
{
  protected:
    typedef typename Derived::Scalar Scalar;
    typedef typename Derived::Index Index;

    enum { IsRowMajor = (Derived::Flags&RowMajorBit)==RowMajorBit };
  public:
    IMP_EIGEN_STRONG_INLINE InnerIterator(const Derived& expr, Index outer)
      : m_expression(expr), m_inner(0), m_outer(outer), m_end(expr.innerSize())
    {}

    IMP_EIGEN_STRONG_INLINE Scalar value() const
    {
      return (IsRowMajor) ? m_expression.coeff(m_outer, m_inner)
                          : m_expression.coeff(m_inner, m_outer);
    }

    IMP_EIGEN_STRONG_INLINE InnerIterator& operator++() { m_inner++; return *this; }

    IMP_EIGEN_STRONG_INLINE Index index() const { return m_inner; }
    inline Index row() const { return IsRowMajor ? m_outer : index(); }
    inline Index col() const { return IsRowMajor ? index() : m_outer; }

    IMP_EIGEN_STRONG_INLINE operator bool() const { return m_inner < m_end && m_inner>=0; }

  protected:
    const Derived& m_expression;
    Index m_inner;
    const Index m_outer;
    const Index m_end;
};

} // end namespace IMP_Eigen

#endif // IMP_EIGEN_COREITERATORS_H
