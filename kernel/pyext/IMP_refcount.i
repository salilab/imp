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

%enddef
