%define IMP_SWIG_ALGEBRA_VALUE_D_NOKD(Namespace, Namebase)
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##1D, Namebase##D, Namebase##1Ds);
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##2D, Namebase##D, Namebase##2Ds);
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##3D, Namebase##D, Namebase##3Ds);
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##4D, Namebase##D, Namebase##4Ds);
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##5D, Namebase##D, Namebase##5Ds);
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##6D, Namebase##D, Namebase##6Ds);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<1>, Namebase##D, Namebase##test##1,Namebase##1Ds);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<2>, Namebase##D, Namebase##test##2,Namebase##2Ds);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<3>, Namebase##D, Namebase##test##3,Namebase##3Ds);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<4>, Namebase##D, Namebase##test##4,Namebase##4Ds);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<5>, Namebase##D, Namebase##test##5,Namebase##5Ds);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<6>, Namebase##D, Namebase##test##6,Namebase##6Ds);

IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<1>, IMP::base::Vector<Namespace::Namebase##D< 1 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<1>, IMP::base::Vector<Namespace::Namebase##D< 1 > >,const&);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<2>, IMP::base::Vector<Namespace::Namebase##D< 2 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<2>, IMP::base::Vector<Namespace::Namebase##D< 2 > >,const&);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<3>, IMP::base::Vector<Namespace::Namebase##D< 3 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<3>, IMP::base::Vector<Namespace::Namebase##D< 3 > >,const&);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<4>, IMP::base::Vector<Namespace::Namebase##D< 4 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<4>, IMP::base::Vector<Namespace::Namebase##D< 4 > >,const&);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<5>, IMP::base::Vector<Namespace::Namebase##D< 5 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<5>, IMP::base::Vector<Namespace::Namebase##D< 5 > >,const&);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<6>, IMP::base::Vector<Namespace::Namebase##D< 6 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<6>, IMP::base::Vector<Namespace::Namebase##D< 6 > >,const&);
IMP_SWIG_VALUE_TEMPLATE(Namespace, Namebase##D);
%extend Namespace::Namebase##D {
  int __cmp__(const Namebase##D<D> &) const {
    IMP_UNUSED(self);
    IMP_THROW("Geometric primitives cannot be compared",
              IMP::base::ValueException);
  }
}
%enddef


%define IMP_SWIG_ALGEBRA_VALUE_D(Namespace, Namebase)
IMP_SWIG_ALGEBRA_VALUE_D_NOKD(Namespace, Namebase);
IMP_SWIG_VALUE_INSTANCE(Namespace, Namebase##KD, Namebase##D, Namebase##KDs);
IMP_SWIG_VALUE_IMPL(Namespace, Namebase##D<-1>, Namebase##D, Namebase##test##k, Namebase##KDs);

IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<-1>, IMP::base::Vector<Namespace::Namebase##D< -1 > >,);
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Namespace::Namebase##D<-1>, IMP::base::Vector<Namespace::Namebase##D< -1 > >,const&);
%enddef

%define IMP_SWIG_ALGEBRA_VALUE(Namespace, Name, PluralName)
IMP_SWIG_VALUE(Namespace, Name, PluralName);
%extend Namespace::Name {
  int __cmp__(const Name &) const {
    IMP_UNUSED(self);
    IMP_THROW("Geometric primitives cannot be compared",
              IMP::ValueException);
  }
}
%enddef


%define IMP_SWIG_ALGEBRA_OBJECT_D(Namespace, Namebase)
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##1D, Namebase##1D, Namebase##1Ds);
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##2D, Namebase##2D, Namebase##2Ds);
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##3D, Namebase##3D, Namebase##3Ds);
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##4D, Namebase##4D, Namebase##4Ds);
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##5D, Namebase##5D, Namebase##5Ds);
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##6D, Namebase##6D, Namebase##6Ds);
IMP_SWIG_OBJECT_INSTANCE(Namespace, Namebase##KD, Namebase##KD, Namebase##KDs);
IMP_SWIG_OBJECT_TEMPLATE(Namespace, Namebase##D);
%enddef

%define IMP_SWIG_ALGEBRA_TEMPLATE_D_NOKD(Namespace, Namebase)
%template(Namebase##1D) Namespace::Namebase##D<1>;
%template(Namebase##2D) Namespace::Namebase##D<2>;
%template(Namebase##3D) Namespace::Namebase##D<3>;
%template(Namebase##4D) Namespace::Namebase##D<4>;
%template(Namebase##5D) Namespace::Namebase##D<5>;
%template(Namebase##6D) Namespace::Namebase##D<6>;
%enddef


%define IMP_SWIG_ALGEBRA_TEMPLATE_D(Namespace, Namebase)
IMP_SWIG_ALGEBRA_TEMPLATE_D_NOKD(Namespace, Namebase);
%template(Namebase##KD) Namespace::Namebase##D<-1>;
%enddef

%define IMP_SWIG_ALGEBRA_PRIVATE_TEMPLATE_D(Namespace, Namebase)
%template(_Namebase##1D) Namespace::Namebase##D<1>;
%template(_Namebase##2D) Namespace::Namebase##D<2>;
%template(_Namebase##3D) Namespace::Namebase##D<3>;
%template(_Namebase##4D) Namespace::Namebase##D<4>;
%template(_Namebase##5D) Namespace::Namebase##D<5>;
%template(_Namebase##6D) Namespace::Namebase##D<6>;
%template(_Namebase##KD) Namespace::Namebase##D<-1>;
%enddef

%define IMP_SWIG_ALGEBRA_TEMPLATE_OBJECT_D(Namespace, Namebase)
%template(Namebase##1D) Namespace::Namebase##D<1>;
%template(Namebase##2D) Namespace::Namebase##D<2>;
%template(Namebase##3D) Namespace::Namebase##D<3>;
%template(Namebase##4D) Namespace::Namebase##D<4>;
%template(Namebase##5D) Namespace::Namebase##D<5>;
%template(Namebase##6D) Namespace::Namebase##D<6>;
%template(Namebase##KD) Namespace::Namebase##D<-1>;
%enddef


%define IMP_SWIG_ALGEBRA_FUNCTION_D_D(ReturnType, function_name, Argument0)
%inline %{
  namespace IMP {
    namespace algebra {
  ReturnType##1D function_name(const Argument0##1D& a) {
  return function_name<1>(a);
}
  ReturnType##2D function_name(const Argument0##2D& a) {
  return function_name<2>(a);
}
ReturnType##3D function_name(const Argument0##3D& a) {
  return function_name<3>(a);
}
ReturnType##4D function_name(const Argument0##4D& a) {
  return function_name<4>(a);
}
ReturnType##5D function_name(const Argument0##5D& a) {
  return function_name<5>(a);
}
ReturnType##6D function_name(const Argument0##6D& a) {
  return function_name<6>(a);
}
ReturnType##KD function_name(const Argument0##KD& a) {
  return function_name<-1>(a);
}
    }
  }
%}
%enddef

%define IMP_SWIG_ALGEBRA_FUNCTION_DS_D(ReturnType, function_name, Argument0)
%inline %{
  namespace IMP {
    namespace algebra {
      base::Vector<ReturnType##1D> function_name(const Argument0##1D& a) {
  return function_name<1>(a);
}
  base::Vector<ReturnType##2D> function_name(const Argument0##2D& a) {
  return function_name<2>(a);
}
base::Vector<ReturnType##3D> function_name(const Argument0##3D& a) {
  return function_name<3>(a);
}
base::Vector<ReturnType##4D> function_name(const Argument0##4D& a) {
  return function_name<4>(a);
}
base::Vector<ReturnType##5D> function_name(const Argument0##5D& a) {
  return function_name<5>(a);
}
base::Vector<ReturnType##6D> function_name(const Argument0##6D& a) {
  return function_name<6>(a);
}
base::Vector<ReturnType##KD> function_name(const Argument0##KD& a) {
  return function_name<-1>(a);
}
    }
  }
%}
%enddef

%define IMP_SWIG_ALGEBRA_FUNCTION_D_DS(ReturnType, function_name, Argument0)
%inline %{
  namespace IMP {
    namespace algebra {
  ReturnType##1D function_name(const IMP::base::Vector<Argument0##1D >& a) {
  return function_name<1>(a);
}
  ReturnType##2D function_name(const IMP::base::Vector<Argument0##2D >& a) {
  return function_name<2>(a);
}
ReturnType##3D function_name(const IMP::base::Vector<Argument0##3D >& a) {
  return function_name<3>(a);
}
ReturnType##4D function_name(const IMP::base::Vector<Argument0##4D >& a) {
  return function_name<4>(a);
}
ReturnType##5D function_name(const IMP::base::Vector<Argument0##5D >& a) {
  return function_name<5>(a);
}
ReturnType##6D function_name(const IMP::base::Vector<Argument0##6D >& a) {
  return function_name<6>(a);
}
ReturnType##KD function_name(const IMP::base::Vector<Argument0##KD >& a) {
  return function_name<-1>(a);
}
    }
  }
%}
%enddef

%define IMP_SWIG_ALGEBRA_FUNCTION_D_DD(ReturnType, function_name, Argument0, Argument1)
%inline %{
namespace IMP {
namespace algebra {
ReturnType##1D function_name(const Argument0##1D& a, const Argument1##1D& b) {
  return function_name<1>(a,b);
}
ReturnType##2D function_name(const Argument0##2D& a, const Argument1##2D& b) {
  return function_name<2>(a,b);
}
ReturnType##3D function_name(const Argument0##3D& a, const Argument1##3D& b) {
  return function_name<3>(a,b);
}
ReturnType##4D function_name(const Argument0##4D& a, const Argument1##4D& b) {
  return function_name<4>(a,b);
}
ReturnType##5D function_name(const Argument0##5D& a, const Argument1##5D& b) {
  return function_name<5>(a,b);
}
ReturnType##6D function_name(const Argument0##6D& a, const Argument1##6D& b) {
  return function_name<6>(a,b);
}
ReturnType##KD function_name(const Argument0##KD& a, const Argument1##KD& b) {
  return function_name<-1>(a,b);
}
}
}
%}
%enddef


%define IMP_SWIG_ALGEBRA_FUNCTION_N_DD(ReturnType, function_name, Argument0, Argument1)
%inline %{
namespace IMP {
namespace algebra {
ReturnType function_name(const Argument0##1D& a, const Argument1##1D& b) {
  return function_name<1>(a,b);
}
ReturnType function_name(const Argument0##2D& a, const Argument1##2D& b) {
  return function_name<2>(a,b);
}
ReturnType function_name(const Argument0##3D& a, const Argument1##3D& b) {
  return function_name<3>(a,b);
}
ReturnType function_name(const Argument0##4D& a, const Argument1##4D& b) {
  return function_name<4>(a,b);
}
ReturnType function_name(const Argument0##5D& a, const Argument1##5D& b) {
  return function_name<5>(a,b);
}
ReturnType function_name(const Argument0##6D& a, const Argument1##6D& b) {
  return function_name<6>(a,b);
}
ReturnType function_name(const Argument0##KD& a, const Argument1##KD& b) {
  return function_name<-1>(a,b);
}
}
}
%}
%enddef

%define IMP_SWIG_ALGEBRA_FUNCTION_DS_DN(ReturnType, function_name, Argument0, Argument1)
%inline %{
  namespace IMP {
    namespace algebra {
ReturnType##1Ds function_name(const Argument0##1D& a, const Argument1& b) {
  return function_name<1>(a,b);
}
ReturnType##2Ds function_name(const Argument0##2D& a, const Argument1& b) {
  return function_name<2>(a,b);
}
ReturnType##3Ds function_name(const Argument0##3D& a, const Argument1& b) {
  return function_name<3>(a,b);
}
ReturnType##4Ds function_name(const Argument0##4D& a, const Argument1& b) {
  return function_name<4>(a,b);
}
ReturnType##5Ds function_name(const Argument0##5D& a, const Argument1& b) {
  return function_name<5>(a,b);
}
ReturnType##6Ds function_name(const Argument0##6D& a, const Argument1& b) {
  return function_name<6>(a,b);
}
ReturnType##KDs function_name(const Argument0##KD& a, const Argument1& b) {
  return function_name<-1>(a,b);
}
    }
  }
%}
%enddef


%define IMP_SWIG_ALGEBRA_FUNCTION_TEMPLATE_D(function_name)
namespace IMP {
namespace algebra {
%template(function_name##_1d) function_name##_d<1>;
%template(function_name##_2d) function_name##_d<2>;
%template(function_name##_3d) function_name##_d<3>;
%template(function_name##_4d) function_name##_d<4>;
%template(function_name##_5d) function_name##_d<5>;
%template(function_name##_6d) function_name##_d<6>;
}
}
%enddef

%{
template <class M>
struct ConvertEigenMatrix {
  static std::pair<int, int> get_dimensions(PyObject* o) {
    int outer = PySequence_Length(o);
    PyReceivePointer inner(PySequence_GetItem(o, 0));
    return std::make_pair(outer, PySequence_Length(inner));
  }

  template <class SwigData>
  static M get_cpp_object(PyObject* o, SwigData st) {
    if (!get_is_cpp_object(o, st)) {
      IMP_THROW("Argument not of correct type", ValueException);
    }
    std::pair<int,int> dim= get_dimensions(o);
    M ret(dim.first, dim.second);
    for (unsigned int i = 0; i < dim.first; ++i) {
      PyReceivePointer inner(PySequence_GetItem(o, i));
      for (unsigned int j = 0; j < dim.second; ++j) {
        PyReceivePointer item(PySequence_GetItem(inner, j));
        ret(i, j) = PyFloat_AsDouble(item);
      }
    }
    return ret;
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st) {
    if (!in || !PySequence_Check(in)) {
      return false;
    }
    int inner = -1;
    for (unsigned int i = 0; i < PySequence_Length(in); ++i) {
      PyReceivePointer o(PySequence_GetItem(in, i));
      if (!o || !PySequence_Check(o)) {
        return false;
      }
      if (inner == -1) {
        inner = PySequence_Length(o);
      } else {
        if (PySequence_Length(o) != inner) return false;
      }
    }
    return true;
  }
  template <class SwigData>
  static PyObject* create_python_object(const M& t, SwigData st, int OWN) {
    PyReceivePointer ret(PyList_New(t.rows()));
    for (unsigned int i = 0; i < t.rows(); ++i) {
      PyReceivePointer inner(PyList_New(t.cols()));
      for (unsigned int j = 0; j < t.cols(); ++j) {
        PyReceivePointer o(PyFloat_FromDouble(t(i,j)));
        // this does not increment the ref count
        IMP_PYTHON_CALL(PyList_SetItem(inner, j, o.release()));
      }
      IMP_PYTHON_CALL(PyList_SetItem(ret, i, inner.release()));
    }
    return ret.release();
  }
};

template <class M>
struct ConvertEigenVector {
  static int get_dimension(PyObject* o) {
    return PySequence_Length(o);
  }

  template <class SwigData>
  static M get_cpp_object(PyObject* o, SwigData st) {
    if (!get_is_cpp_object(o, st)) {
      IMP_THROW("Argument not of correct type", ValueException);
    }
    int dim= get_dimension(o);
    M ret(dim);
    for (unsigned int i = 0; i < dim; ++i) {
      PyReceivePointer item(PySequence_GetItem(o, i));
      ret(i) = PyFloat_AsDouble(item);
    }
    return ret;
  }
  template <class SwigData>
  static bool get_is_cpp_object(PyObject* in, SwigData st) {
    if (!in || !PySequence_Check(in)) {
      return false;
    }
    for (unsigned int i = 0; i < PySequence_Length(in); ++i) {
      PyReceivePointer o(PySequence_GetItem(in, i));
      if (!o) {
        return false;
      }
    }
    return true;
  }
  template <class SwigData>
  static PyObject* create_python_object(const M& t, SwigData st, int OWN) {
    PyReceivePointer ret(PyList_New(t.rows()));
    for (unsigned int i = 0; i < t.rows(); ++i) {
      PyReceivePointer o(PyFloat_FromDouble(t(i)));
      // this does not increment the ref count
      IMP_PYTHON_CALL(PyList_SetItem(ret, i, o.release()));
    }
    return ret.release();
  }
};

  %}

%define IMP_SWIG_EIGEN_MATRIX(Name)
%typemap(in) IMP_Eigen::Name const& {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertEigenMatrix<IMP_Eigen::Name>::get_cpp_object($input, $descriptor(IMP_Eigen::Name*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) IMP_Eigen::Name const& {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) IMP_Eigen::Name const& {
  $1= ConvertEigenMatrix<IMP_Eigen::Name>::get_is_cpp_object($input, $descriptor(IMP_Eigen::Name*));
 }
%typemap(out) IMP_Eigen::Name const& {
  $result = ConvertEigenMatrix<IMP_Eigen::Name >::create_python_object(ValueOrObject<IMP_Eigen::Name >::get($1), $descriptor(IMP_Eigen::Name*), SWIG_POINTER_OWN);
 }
%typemap(out) IMP_Eigen::Name {
  $result = ConvertEigenMatrix<IMP_Eigen::Name >::create_python_object(ValueOrObject<IMP_Eigen::Name >::get($1), $descriptor(IMP_Eigen::Name*), SWIG_POINTER_OWN);
 }
%typemap(directorout) IMP_Eigen::Name const& {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertEigenMatrix<IMP_Eigen::Name >::get_cpp_object($input, $descriptor(IMP_Eigen::Name*)));
 }
%typemap(directorin) IMP_Eigen::Name const& {
  $input = ConvertEigenMatrix<IMP_Eigen::Name >::create_python_object($1_name, $descriptor(IMP_Eigen::Name*), SWIG_POINTER_OWN);
 }
%typemap(in) IMP_Eigen::Name* {
  collections_like_##Name##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) IMP_Eigen::Name* {
  collections_like_##Name##_must_be_returned_by_value_or_const_ref;
 }
%typemap(in) IMP_Eigen::Name& {
  collections_like_##Name##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) IMP_Eigen::Name& {
  collections_like_##Name##_must_be_returned_by_value_or_const_ref;
 }
%enddef

%define IMP_SWIG_EIGEN_VECTOR(Name)
%typemap(in) IMP_Eigen::Name const& {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertEigenVector<IMP_Eigen::Name>::get_cpp_object($input, $descriptor(IMP_Eigen::Name*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) IMP_Eigen::Name const& {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) IMP_Eigen::Name const& {
  $1= ConvertEigenVector<IMP_Eigen::Name>::get_is_cpp_object($input, $descriptor(IMP_Eigen::Name*));
 }
%typemap(out) IMP_Eigen::Name const& {
  $result = ConvertEigenVector<IMP_Eigen::Name >::create_python_object(ValueOrObject<IMP_Eigen::Name >::get($1), $descriptor(IMP_Eigen::Name*), SWIG_POINTER_OWN);
 }
%typemap(out) IMP_Eigen::Name {
  $result = ConvertEigenVector<IMP_Eigen::Name >::create_python_object(ValueOrObject<IMP_Eigen::Name >::get($1), $descriptor(IMP_Eigen::Name*), SWIG_POINTER_OWN);
 }
%typemap(directorout) IMP_Eigen::Name const& {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertEigenVector<IMP_Eigen::Name >::get_cpp_object($input, $descriptor(IMP_Eigen::Name*)));
 }
%typemap(directorin) IMP_Eigen::Name const& {
  $input = ConvertEigenVector<IMP_Eigen::Name >::create_python_object($1_name, $descriptor(IMP_Eigen::Name*), SWIG_POINTER_OWN);
 }
%typemap(in) IMP_Eigen::Name* {
  collections_like_##Name##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) IMP_Eigen::Name* {
  collections_like_##Name##_must_be_returned_by_value_or_const_ref;
 }
%typemap(in) IMP_Eigen::Name& {
  collections_like_##Name##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) IMP_Eigen::Name& {
  collections_like_##Name##_must_be_returned_by_value_or_const_ref;
 }
%enddef
