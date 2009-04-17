// Make sure that we refcount any pointers to TYPE (e.g. IMP::Particle)
// returned from C++ methods.
%define IMP_REFCOUNT_RETURN(TYPE)

// If a C++ method returns a pointer to TYPE, increase its reference count
// so that the object is not deleted while Python holds a pointer to it.
// Take ownership of the pointer (SWIG_POINTER_OWN) so that SWIG calls unref
// on it when we are done.
%typemap(out) TYPE * {
   if (!($owner & SWIG_POINTER_NEW)) {
     // out typemaps are also called for constructors, which already use %ref
     // to increase the reference count. So don't do it twice.
     IMP::internal::ref($1);
   }
   %set_output(SWIG_NewPointerObj(%as_voidptr($1), $descriptor(TYPE *), $owner | SWIG_POINTER_OWN));
}

// Specialize the traits_from class to do the same thing as the out typemap.
// This class is used by swig::from(), which is primarily used by SWIG
// to return TYPE pointers from STL iterators.

// In order to get this *after* the definition of the traits_from template,
// we need to use insert(wrapper) below. But the wrapper code is extern C.
// Ideally SWIG would also us to insert code after the C++ definitions and
// before the C wrapper section.
%insert(wrapper) %{
#ifdef __cplusplus
}
#endif

namespace swig {
  template <>
  struct traits_from<TYPE *> {
    static PyObject *from(TYPE *f) {
      IMP::internal::ref(f);
      return SWIG_NewPointerObj(SWIG_as_voidptr(f), type_info<TYPE>(), SWIG_POINTER_OWN);
    }
  };
}
#ifdef __cplusplus
extern "C" {
#endif
%}
%enddef
