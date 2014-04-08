// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMP_EIGEN_ARRAY_H
#define IMP_EIGEN_ARRAY_H

namespace IMP_Eigen {

/** \class Array 
  * \ingroup Core_Module
  *
  * \brief General-purpose arrays with easy API for coefficient-wise operations
  *
  * The %Array class is very similar to the Matrix class. It provides
  * general-purpose one- and two-dimensional arrays. The difference between the
  * %Array and the %Matrix class is primarily in the API: the API for the
  * %Array class provides easy access to coefficient-wise operations, while the
  * API for the %Matrix class provides easy access to linear-algebra
  * operations.
  *
  * This class can be extended with the help of the plugin mechanism described on the page
  * \ref TopicCustomizingEigen by defining the preprocessor symbol \c IMP_EIGEN_ARRAY_PLUGIN.
  *
  * \sa \ref TutorialArrayClass, \ref TopicClassHierarchy
  */
namespace internal {
template<typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
struct traits<Array<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> > : traits<Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> >
{
  typedef ArrayXpr XprKind;
  typedef ArrayBase<Array<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> > XprBase;
};
}

template<typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
class Array
  : public PlainObjectBase<Array<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols> >
{
  public:

    typedef PlainObjectBase<Array> Base;
    IMP_EIGEN_DENSE_PUBLIC_INTERFACE(Array)

    enum { Options = _Options };
    typedef typename Base::PlainObject PlainObject;

  protected:
    template <typename Derived, typename OtherDerived, bool IsVector>
    friend struct internal::conservative_resize_like_impl;

    using Base::m_storage;

  public:

    using Base::base;
    using Base::coeff;
    using Base::coeffRef;

    /**
      * The usage of
      *   using Base::operator=;
      * fails on MSVC. Since the code below is working with GCC and MSVC, we skipped
      * the usage of 'using'. This should be done only for operator=.
      */
    template<typename OtherDerived>
    IMP_EIGEN_STRONG_INLINE Array& operator=(const EigenBase<OtherDerived> &other)
    {
      return Base::operator=(other);
    }

    /** Copies the value of the expression \a other into \c *this with automatic resizing.
      *
      * *this might be resized to match the dimensions of \a other. If *this was a null matrix (not already initialized),
      * it will be initialized.
      *
      * Note that copying a row-vector into a vector (and conversely) is allowed.
      * The resizing, if any, is then done in the appropriate way so that row-vectors
      * remain row-vectors and vectors remain vectors.
      */
    template<typename OtherDerived>
    IMP_EIGEN_STRONG_INLINE Array& operator=(const ArrayBase<OtherDerived>& other)
    {
      return Base::_set(other);
    }

    /** This is a special case of the templated operator=. Its purpose is to
      * prevent a default operator= from hiding the templated operator=.
      */
    IMP_EIGEN_STRONG_INLINE Array& operator=(const Array& other)
    {
      return Base::_set(other);
    }

    /** Default constructor.
      *
      * For fixed-size matrices, does nothing.
      *
      * For dynamic-size matrices, creates an empty matrix of size 0. Does not allocate any array. Such a matrix
      * is called a null matrix. This constructor is the unique way to create null matrices: resizing
      * a matrix to 0 is not supported.
      *
      * \sa resize(Index,Index)
      */
    IMP_EIGEN_STRONG_INLINE Array() : Base()
    {
      Base::_check_template_params();
      IMP_EIGEN_INITIALIZE_COEFFS_IF_THAT_OPTION_IS_ENABLED
    }

#ifndef IMP_EIGEN_PARSED_BY_DOXYGEN
    // FIXME is it still needed ??
    /** \internal */
    Array(internal::constructor_without_unaligned_array_assert)
      : Base(internal::constructor_without_unaligned_array_assert())
    {
      Base::_check_template_params();
      IMP_EIGEN_INITIALIZE_COEFFS_IF_THAT_OPTION_IS_ENABLED
    }
#endif

    /** Constructs a vector or row-vector with given dimension. \only_for_vectors
      *
      * Note that this is only useful for dynamic-size vectors. For fixed-size vectors,
      * it is redundant to pass the dimension here, so it makes more sense to use the default
      * constructor Matrix() instead.
      */
    IMP_EIGEN_STRONG_INLINE explicit Array(Index dim)
      : Base(dim, RowsAtCompileTime == 1 ? 1 : dim, ColsAtCompileTime == 1 ? 1 : dim)
    {
      Base::_check_template_params();
      IMP_EIGEN_STATIC_ASSERT_VECTOR_ONLY(Array)
      imp_eigen_assert(dim >= 0);
      imp_eigen_assert(SizeAtCompileTime == Dynamic || SizeAtCompileTime == dim);
      IMP_EIGEN_INITIALIZE_COEFFS_IF_THAT_OPTION_IS_ENABLED
    }

    #ifndef IMP_EIGEN_PARSED_BY_DOXYGEN
    template<typename T0, typename T1>
    IMP_EIGEN_STRONG_INLINE Array(const T0& val0, const T1& val1)
    {
      Base::_check_template_params();
      this->template _init2<T0,T1>(val0, val1);
    }
    #else
    /** constructs an uninitialized matrix with \a rows rows and \a cols columns.
      *
      * This is useful for dynamic-size matrices. For fixed-size matrices,
      * it is redundant to pass these parameters, so one should use the default constructor
      * Matrix() instead. */
    Array(Index rows, Index cols);
    /** constructs an initialized 2D vector with given coefficients */
    Array(const Scalar& val0, const Scalar& val1);
    #endif

    /** constructs an initialized 3D vector with given coefficients */
    IMP_EIGEN_STRONG_INLINE Array(const Scalar& val0, const Scalar& val1, const Scalar& val2)
    {
      Base::_check_template_params();
      IMP_EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Array, 3)
      m_storage.data()[0] = val0;
      m_storage.data()[1] = val1;
      m_storage.data()[2] = val2;
    }
    /** constructs an initialized 4D vector with given coefficients */
    IMP_EIGEN_STRONG_INLINE Array(const Scalar& val0, const Scalar& val1, const Scalar& val2, const Scalar& val3)
    {
      Base::_check_template_params();
      IMP_EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Array, 4)
      m_storage.data()[0] = val0;
      m_storage.data()[1] = val1;
      m_storage.data()[2] = val2;
      m_storage.data()[3] = val3;
    }

    explicit Array(const Scalar *data);

    /** Constructor copying the value of the expression \a other */
    template<typename OtherDerived>
    IMP_EIGEN_STRONG_INLINE Array(const ArrayBase<OtherDerived>& other)
             : Base(other.rows() * other.cols(), other.rows(), other.cols())
    {
      Base::_check_template_params();
      Base::_set_noalias(other);
    }
    /** Copy constructor */
    IMP_EIGEN_STRONG_INLINE Array(const Array& other)
            : Base(other.rows() * other.cols(), other.rows(), other.cols())
    {
      Base::_check_template_params();
      Base::_set_noalias(other);
    }
    /** Copy constructor with in-place evaluation */
    template<typename OtherDerived>
    IMP_EIGEN_STRONG_INLINE Array(const ReturnByValue<OtherDerived>& other)
    {
      Base::_check_template_params();
      Base::resize(other.rows(), other.cols());
      other.evalTo(*this);
    }

    /** \sa MatrixBase::operator=(const EigenBase<OtherDerived>&) */
    template<typename OtherDerived>
    IMP_EIGEN_STRONG_INLINE Array(const EigenBase<OtherDerived> &other)
      : Base(other.derived().rows() * other.derived().cols(), other.derived().rows(), other.derived().cols())
    {
      Base::_check_template_params();
      Base::resize(other.rows(), other.cols());
      *this = other;
    }

    /** Override MatrixBase::swap() since for dynamic-sized matrices of same type it is enough to swap the
      * data pointers.
      */
    template<typename OtherDerived>
    void swap(ArrayBase<OtherDerived> const & other)
    { this->_swap(other.derived()); }

    inline Index innerStride() const { return 1; }
    inline Index outerStride() const { return this->innerSize(); }

    #ifdef IMP_EIGEN_ARRAY_PLUGIN
    #include IMP_EIGEN_ARRAY_PLUGIN
    #endif

  private:

    template<typename MatrixType, typename OtherDerived, bool SwapPointers>
    friend struct internal::matrix_swap_impl;
};

/** \defgroup arraytypedefs Global array typedefs
  * \ingroup Core_Module
  *
  * Eigen defines several typedef shortcuts for most common 1D and 2D array types.
  *
  * The general patterns are the following:
  *
  * \c ArrayRowsColsType where \c Rows and \c Cols can be \c 2,\c 3,\c 4 for fixed size square matrices or \c X for dynamic size,
  * and where \c Type can be \c i for integer, \c f for float, \c d for double, \c cf for complex float, \c cd
  * for complex double.
  *
  * For example, \c Array33d is a fixed-size 3x3 array type of doubles, and \c ArrayXXf is a dynamic-size matrix of floats.
  *
  * There are also \c ArraySizeType which are self-explanatory. For example, \c Array4cf is
  * a fixed-size 1D array of 4 complex floats.
  *
  * \sa class Array
  */

#define IMP_EIGEN_MAKE_ARRAY_TYPEDEFS(Type, TypeSuffix, Size, SizeSuffix)   \
/** \ingroup arraytypedefs */                                    \
typedef Array<Type, Size, Size> Array##SizeSuffix##SizeSuffix##TypeSuffix;  \
/** \ingroup arraytypedefs */                                    \
typedef Array<Type, Size, 1>    Array##SizeSuffix##TypeSuffix;

#define IMP_EIGEN_MAKE_ARRAY_FIXED_TYPEDEFS(Type, TypeSuffix, Size)         \
/** \ingroup arraytypedefs */                                    \
typedef Array<Type, Size, Dynamic> Array##Size##X##TypeSuffix;  \
/** \ingroup arraytypedefs */                                    \
typedef Array<Type, Dynamic, Size> Array##X##Size##TypeSuffix;

#define IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES(Type, TypeSuffix) \
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS(Type, TypeSuffix, 2, 2) \
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS(Type, TypeSuffix, 3, 3) \
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS(Type, TypeSuffix, 4, 4) \
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS(Type, TypeSuffix, Dynamic, X) \
IMP_EIGEN_MAKE_ARRAY_FIXED_TYPEDEFS(Type, TypeSuffix, 2) \
IMP_EIGEN_MAKE_ARRAY_FIXED_TYPEDEFS(Type, TypeSuffix, 3) \
IMP_EIGEN_MAKE_ARRAY_FIXED_TYPEDEFS(Type, TypeSuffix, 4)

IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES(int,                  i)
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES(float,                f)
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES(double,               d)
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES(std::complex<float>,  cf)
IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES(std::complex<double>, cd)

#undef IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_ALL_SIZES
#undef IMP_EIGEN_MAKE_ARRAY_TYPEDEFS

#undef IMP_EIGEN_MAKE_ARRAY_TYPEDEFS_LARGE

#define IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE_AND_SIZE(TypeSuffix, SizeSuffix) \
using IMP_Eigen::Matrix##SizeSuffix##TypeSuffix; \
using IMP_Eigen::Vector##SizeSuffix##TypeSuffix; \
using IMP_Eigen::RowVector##SizeSuffix##TypeSuffix;

#define IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE(TypeSuffix) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE_AND_SIZE(TypeSuffix, 2) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE_AND_SIZE(TypeSuffix, 3) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE_AND_SIZE(TypeSuffix, 4) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE_AND_SIZE(TypeSuffix, X) \

#define IMP_EIGEN_USING_ARRAY_TYPEDEFS \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE(i) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE(f) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE(d) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE(cf) \
IMP_EIGEN_USING_ARRAY_TYPEDEFS_FOR_TYPE(cd)

} // end namespace IMP_Eigen

#endif // IMP_EIGEN_ARRAY_H
