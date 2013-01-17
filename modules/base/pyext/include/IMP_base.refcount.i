// IMP methods that return pointers to IMP objects (e.g. IMP::Particle)
// always return 'weak' pointers (known to Python as "borrowed references").
// It is the responsibility of the caller to handle the reference count.
// Thus, when we create a SWIG Python proxy of any such object, we should
// make sure that we increase the reference count so that C++ does not
// free the object while we have a Python reference to it.

// There are two functions defined here which should be used for any IMP
// refcounted type that is returned from IMP methods:

// IMP_REFCOUNT_RETURN(TYPE) should be used for types that can be returned
//     either by regular methods or by iterators. Typically this is any type
//     that is used in an IMP_LIST or a std::vector (e.g. Particles,
//     Restraints, ScoreStates). If in doubt, use this function.

// IMP_REFCOUNT_RETURN_SINGLE(TYPE) should be used for types that are never used
//     in an iterator. Use this if you tried IMP_REFCOUNT_RETURN(TYPE) and it
//     failed to compile with an error like
//     'type_name' is not a member of 'swig::traits<TYPE>'


%define IMP_REFCOUNT_RETURN_SINGLE(TYPE)
// If a C++ method returns a pointer to TYPE, increase its reference count
// so that the object is not deleted while Python holds a pointer to it.
// Take ownership of the pointer (SWIG_POINTER_OWN) so that SWIG calls unref
// on it when we are done.
%typemap(out) TYPE * {
   if (!($owner & SWIG_POINTER_NEW)) {
     // out typemaps are also called for constructors, which already use %ref
     // to increase the reference count. So don't do it twice.
     IMP::base::internal::ref($1);
   }
   %set_output(SWIG_NewPointerObj(%as_voidptr($1), $descriptor(TYPE *), $owner | SWIG_POINTER_OWN));
}
%enddef


%define IMP_REFCOUNT_RETURN(TYPE)
IMP_REFCOUNT_RETURN_SINGLE(TYPE)

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
      IMP::base::internal::ref(f);
      return SWIG_NewPointerObj(SWIG_as_voidptr(f), type_info<TYPE>(), SWIG_POINTER_OWN);
    }
  };
}
#ifdef __cplusplus
extern "C" {
#endif
%}
%enddef
