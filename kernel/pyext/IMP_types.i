
/*%pythonprepend Namespace::Name::~Name %{
        print "bye"
%}*/

/* SWIG's default behavior for functions that return a reference to an object
   is to create a Python object that points to that object. This doesn't work
   well if the IMP object is not refcounted and the C++ object is deleted before
   it is used in Python (for example, algebra::Transformation3D::get_rotation()
   returns a const reference to its internal algebra::Rotation3D object; if the
   Transformation3D is destroyed before the Rotation3D is used, an invalid
   memory access will ensue).

   This typemap modifies the default behavior to instead copy the returned
   object (as if it were returned by value). It should *probably* only be used
   for simple types that do not use large amounts of memory, and only for
   returns of const references. (In other cases, it may make sense to make the
   type refcounted, or not to expose it to Python.)

   Example usage:

   %apply REFCOPY & { const Rotation3D & };
*/
%typemap(out, noblock=1) REFCOPY & {
  %set_output(SWIG_NewPointerObj(%new_copy(*$1, $*ltype), $descriptor, SWIG_POINTER_OWN | %newpointer_flags));
 }


/* Add additional IMP_CONTAINER methods for scripting languages */
%define IMP_SWIG_CONTAINER(Namespace, ContainedNamespace, type, Ucname, lcname)
     %extend Namespace::type {
  ContainedNamespace::Ucname##s get_##lcname##s() const {
    ContainedNamespace::Ucname##s ret(self->lcname##s_begin(), self->lcname##s_end());
    return ret;
  }
 }
%enddef


%typemap(in) IMP::Particle* {
  try {
  $1 = IMP::internal::swig::Convert<IMP::Particle >::get_cpp_object($input, $descriptor(IMP::Particle*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*));
  } catch (const IMP::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typecheck(SWIG_TYPECHECK_POINTER) IMP::Particle* {
                                   try {
  IMP::internal::swig::Convert<IMP::Particle >::get_cpp_object($input, $descriptor(IMP::Particle*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*));
  $1=1;
} catch (...) {
$1=0;
}
 }



%define IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, CONSTREF)
%typemap(in) Namespace::PluralName CONSTREF {
  try {
    // hack to get around swig's value wrapper being randomly used
    IMP::internal::swig::assign($1, IMP::internal::swig::Convert<Namespace::PluralName >::get_cpp_object($input, $descriptor(Namespace::Name*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*)));
  } catch (const IMP::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) Namespace::PluralName CONSTREF {
  IMP::internal::swig::delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) Namespace::PluralName CONSTREF {
  $1= IMP::internal::swig::Convert<Namespace::PluralName >::get_is_cpp_object($input, $descriptor(Namespace::Name*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*));
 }
%typemap(out) Namespace::PluralName CONSTREF {
  $result = IMP::internal::swig::Convert<Namespace::PluralName >::create_python_object(IMP::internal::swig::ValueOrObject<Namespace::PluralName >::get($1), $descriptor(Namespace::Name*), SWIG_POINTER_OWN);
 }

%typemap(directorout) Namespace::PluralName CONSTREF {
    // hack to get around swig's evil value wrapper being randomly used
    IMP::internal::swig::assign($result, IMP::internal::swig::Convert<Namespace::PluralName >::get_cpp_object($input, $descriptor(Namespace::Name*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*)));
 }

%typemap(directorin) Namespace::PluralName CONSTREF {
  $input = IMP::internal::swig::Convert<Namespace::PluralName >::create_python_object($1_name, $descriptor(Namespace::Name*), SWIG_POINTER_OWN);
 }
%typemap(in) Namespace::PluralName* {
   BOOST_STATIC_ASSERT(0&&"Collections must be passed by value or const ref");
}
%typemap(out) Namespace::PluralName* {
   BOOST_STATIC_ASSERT(0&&"Collections must be returned by value or by const ret");
}
%enddef


%define IMP_SWIG_OBJECT(Namespace,Name, PluralName)
%typemap(out) Namespace::Name* {
   if (!($owner & SWIG_POINTER_NEW)) {
     // out typemaps are also called for constructors, which already use %ref
     // to increase the reference count. So don't do it twice.
     IMP::internal::ref($1);
   }
   %set_output(SWIG_NewPointerObj(%as_voidptr($1), $descriptor(Namespace::Name *), $owner | SWIG_POINTER_OWN));
}
/*%typemap(in) const Namespace::Name& {
   IMP_COMPILE_WARNING("Objects should be passed by pointer.");
}*/
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp,);
%pythoncode %{
  PluralName=list
  PluralName##Temp=list
%}
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%enddef




%define IMP_SWIG_BASE_OBJECT(Namespace, Name, PluralName)
IMP_SWIG_OBJECT(Namespace, Name, PluralName);
IMP_SWIG_DIRECTOR(Namespace, Name);
%enddef








%define IMP_SWIG_DECORATOR(Namespace, Name, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp,);
%pythoncode %{
  def PluralName(l=[]):
      return [Name(x) for x in l]
  PluralName##Temp=PluralName
%}
/*%typemap(in) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Decorators must be passed by value (or const ref, but there is no reason to do this)");
}
%typemap(out) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Decorators must be returned by value (or by const ref, but there is no reason to do this)");
}*/
%{
  BOOST_STATIC_ASSERT(IMP::internal::swig::Convert<Namespace::Name>::converter==3);
%}
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%enddef


%define IMP_SWIG_DECORATOR_WITH_TRAITS(Namespace, Name, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp,);
%pythoncode %{
  PluralName=list
  PluralName##Temp=list
%}
/*%typemap(in) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Decorators must be passed by value (or const ref, but there is no reason to do this)");
}
%typemap(out) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Decorators must be returned by value (or by const ref, but there is no reason to do this)");
}*/
%{
  BOOST_STATIC_ASSERT(IMP::internal::swig::Convert<Namespace::Name>::converter==4);
%}
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%enddef








%define IMP_SWIG_OBJECT_TUPLE(Namespace, Name, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName##Temp,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, Name, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, Name,);
/*%typemap(in) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Values must be passed by value or const ref");
}
%typemap(out) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Values must be returned by value or const ref");
}*/
%pythoncode %{
  PluralName=list
  PluralName##Temp=list
%}
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%enddef
 //SWIGTYPE_p_##SwigNamespace##Name











%define IMP_SWIG_VALUE(Namespace, Name, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName,);
%typemap(out) Namespace::Name const& {
  $result=SWIG_NewPointerObj(new Namespace::Name(*$1), $descriptor(Namespace::Name*), SWIG_POINTER_OWN | %newpointer_flags);
}
/*%typemap(in) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Values must be passed by value or const ref");
}
%typemap(out) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Values must be returned by value or const ref");
}*/
%pythoncode %{
  PluralName=list
%}
%feature("valuewrapper") PluralName;
%{
void test_##PluralName() {
   Namespace::PluralName nm;
}
%}
%enddef


%define IMP_SWIG_PAIR(Namespace, Name, PairName, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PairName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PairName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName,);
%feature("valuewrapper") PluralName;
/*%typemap(in) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&& "Values must be passed by value or const ref");
}
%typemap(out) Namespace::Name* {
   BOOST_STATIC_ASSERT(0&&"Values must be returned by value or const ref");
}*/
%pythoncode %{
  def PairName(a,b):
    return (a,b)
  PluralName= list
%}
%enddef
