// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2010-2012 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2010 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMP_EIGEN_GLOBAL_FUNCTIONS_H
#define IMP_EIGEN_GLOBAL_FUNCTIONS_H

#define IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(NAME,FUNCTOR) \
  template<typename Derived> \
  inline const IMP_Eigen::CwiseUnaryOp<IMP_Eigen::internal::FUNCTOR<typename Derived::Scalar>, const Derived> \
  NAME(const IMP_Eigen::ArrayBase<Derived>& x) { \
    return x.derived(); \
  }

#define IMP_EIGEN_ARRAY_DECLARE_GLOBAL_EIGEN_UNARY(NAME,FUNCTOR) \
  \
  template<typename Derived> \
  struct NAME##_retval<ArrayBase<Derived> > \
  { \
    typedef const IMP_Eigen::CwiseUnaryOp<IMP_Eigen::internal::FUNCTOR<typename Derived::Scalar>, const Derived> type; \
  }; \
  template<typename Derived> \
  struct NAME##_impl<ArrayBase<Derived> > \
  { \
    static inline typename NAME##_retval<ArrayBase<Derived> >::type run(const IMP_Eigen::ArrayBase<Derived>& x) \
    { \
      return x.derived(); \
    } \
  };


namespace IMP_Eigen
{
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(real,scalar_real_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(imag,scalar_imag_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(conj,scalar_conjugate_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(sin,scalar_sin_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(cos,scalar_cos_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(asin,scalar_asin_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(acos,scalar_acos_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(tan,scalar_tan_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(exp,scalar_exp_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(log,scalar_log_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(abs,scalar_abs_op)
  IMP_EIGEN_ARRAY_DECLARE_GLOBAL_UNARY(sqrt,scalar_sqrt_op)
  
  template<typename Derived>
  inline const IMP_Eigen::CwiseUnaryOp<IMP_Eigen::internal::scalar_pow_op<typename Derived::Scalar>, const Derived>
  pow(const IMP_Eigen::ArrayBase<Derived>& x, const typename Derived::Scalar& exponent) {
    return x.derived().pow(exponent);
  }

  template<typename Derived>
  inline const IMP_Eigen::CwiseBinaryOp<IMP_Eigen::internal::scalar_binary_pow_op<typename Derived::Scalar, typename Derived::Scalar>, const Derived, const Derived>
  pow(const IMP_Eigen::ArrayBase<Derived>& x, const IMP_Eigen::ArrayBase<Derived>& exponents) 
  {
    return IMP_Eigen::CwiseBinaryOp<IMP_Eigen::internal::scalar_binary_pow_op<typename Derived::Scalar, typename Derived::Scalar>, const Derived, const Derived>(
      x.derived(),
      exponents.derived()
    );
  }
  
  /**
  * \brief Component-wise division of a scalar by array elements.
  **/
  template <typename Derived>
  inline const IMP_Eigen::CwiseUnaryOp<IMP_Eigen::internal::scalar_inverse_mult_op<typename Derived::Scalar>, const Derived>
    operator/(const typename Derived::Scalar& s, const IMP_Eigen::ArrayBase<Derived>& a)
  {
    return IMP_Eigen::CwiseUnaryOp<IMP_Eigen::internal::scalar_inverse_mult_op<typename Derived::Scalar>, const Derived>(
      a.derived(),
      IMP_Eigen::internal::scalar_inverse_mult_op<typename Derived::Scalar>(s)  
    );
  }

  namespace internal
  {
    IMP_EIGEN_ARRAY_DECLARE_GLOBAL_EIGEN_UNARY(real,scalar_real_op)
    IMP_EIGEN_ARRAY_DECLARE_GLOBAL_EIGEN_UNARY(imag,scalar_imag_op)
    IMP_EIGEN_ARRAY_DECLARE_GLOBAL_EIGEN_UNARY(abs2,scalar_abs2_op)
  }
}

// TODO: cleanly disable those functions that are not supported on Array (numext::real_ref, internal::random, internal::isApprox...)

#endif // IMP_EIGEN_GLOBAL_FUNCTIONS_H
