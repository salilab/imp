
%{
#define IMP_SWIG_CPP_WARNING(string) IMP_WARN_PREPROCESS(string)
  %}

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

/* size_t is an unsigned type, but Python only has signed integer types.
   Thus a large size_t can overflow a regular Python 'int'. SWIG is
   conservative and so converts such large values to Python 'long's instead.
   This causes hashing to fail on Python < 2.5 though, since hash values must
   be ints rather than longs. But for a hash value we don't actually care
   whether it's signed or not. So we override the default here and force the
   hash value into a signed type, so it will always fit into a Python 'int'. */
%typemap(out) std::size_t __hash__ {
  $result = PyInt_FromLong(static_cast<long>($1));
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
%{
  BOOST_STATIC_ASSERT(IMP::internal::swig::Convert<IMP::Particle>::converter ==2);
  BOOST_STATIC_ASSERT(IMP::internal::swig::Convert<IMP::internal::_TrivialDecorator>::converter ==3);
  %}





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
  BOOST_STATIC_ASSERT(0&&"Collections must be passed by value or const ref " #PluralName);
 }
%typemap(out) Namespace::PluralName* {
  BOOST_STATIC_ASSERT(0&&"Collections must be returned by value or by const ret" #PluralName);
 }
%typemap(in) Namespace::PluralName& {
  BOOST_STATIC_ASSERT(0&&"Collections must be passed by value or by const ret" #PluralName);
 }
%typemap(out) Namespace::PluralName& {
  BOOST_STATIC_ASSERT(0&&"Collections must be returned by value or by const ret" #PluralName);
 }
%enddef

%define IMP_SWIG_VALUE_CHECKS(Namespace, Name)
%typemap(out) Namespace::Name& {
  BOOST_STATIC_ASSERT(0&&"Values must be returned by value or const ref" #Name);
 }
// for some reason swig generates garbage code when either of the below is defined
/*%typemap(in) Namespace::Name& {
  IMP_SWIG_CPP_WARNING("Values should be passed by value or const ref");
  try {
  IMP::internal::swig::assign($1, IMP::internal::swig::Convert<Namespace::Name >::get_cpp_object($input, $descriptor(Namespace::Name), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*)));
  } catch (const IMP::Exception &e) {
  //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
  PyErr_SetString(PyExc_TypeError, e.what());
  return NULL;
  }
  }
  %typemap(in) Namespace::Name* {
  BOOST_STATIC_ASSERT($argnum==1 && "Values must be returned by value or const ref");
  try {
  IMP::internal::swig::assign($1, IMP::internal::swig::Convert<Namespace::Name >::get_cpp_object($input, $descriptor(Namespace::Name), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*)));
  } catch (const IMP::Exception &e) {
  //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
  PyErr_SetString(PyExc_TypeError, e.what());
  return NULL;
  }
  }*/
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
IMP_SWIG_VALUE_CHECKS(Namespace, PluralName);
%pythoncode %{
  PluralName=list
  PluralName##Temp=list
%}
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%enddef



%define IMP_SWIG_BASE_OBJECT(Namespace, Name, PluralName)
IMP_SWIG_OBJECT(Namespace, Name, PluralName);
%extend Namespace::Name {
  %pythoncode %{
    def get_type_name(self):
        return self.__class__.__name__
    def do_show(self, out):
        pass
    def get_version_info(self):
        if #Namespace == "IMP":
          return VersionInfo("python", "0")
        else:
          return IMP.VersionInfo("python", "0")%}
}
IMP_SWIG_DIRECTOR(Namespace, Name);
%enddef


%define IMP_SWIG_FORWARD_0(name, ret)
ret name() {
   return self->get_particle()->name();
}
%enddef

%define IMP_SWIG_VOID_FORWARD_0(name)
void name() {
   self->get_particle()->name();
}
%enddef


%define IMP_SWIG_FORWARD_1(name, ret, type0)
ret name(type0 a0) {
   return self->get_particle()->name(a0);
}
%enddef

%define IMP_SWIG_VOID_FORWARD_1(name, type0)
void name(type0 a0) {
   self->get_particle()->name(a0);
}
%enddef

%define IMP_SWIG_FORWARD_2(name, ret, type0, type1)
ret name(type0 a0, type1 a1) {
   return self->get_particle()->name(a0, a1);
}
%enddef

%define IMP_SWIG_VOID_FORWARD_2(name, type0, type1)
void name(type0 a0, type1 a1) {
   self->get_particle()->name(a0, a1);
}
%enddef

%define IMP_SWIG_FORWARD_3(name, ret, type0, type1, type2)
ret name(type0 a0, type1 a1, type2 a2) {
   return self->get_particle()->name(a0, a1, a2);
}
%enddef

%define IMP_SWIG_VOID_FORWARD_3(name, type0, type1, type2)
void name(type0 a0, type1 a1, type2 a2) {
   self->get_particle()->name(a0, a1, a2);
}
%enddef

%define IMP_SWIG_DECORATOR_ATTRIBUTE(Type, Key)
IMP_SWIG_VOID_FORWARD_2(add_attribute, IMP::Key, IMP::Type);
IMP_SWIG_FORWARD_1(get_value, IMP::Type, IMP::Key);
IMP_SWIG_VOID_FORWARD_2(set_value, IMP::Key, IMP::Type);
IMP_SWIG_VOID_FORWARD_1(remove_attribute, IMP::Key);
IMP_SWIG_FORWARD_1(has_attribute, bool, IMP::Key);
%enddef




%define IMP_SWIG_DECORATOR_BASE(Namespace, Name, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName##Temp,);
%pythoncode %{
  def PluralName(l=[]):
    return [Name(x) for x in l]
  PluralName##Temp=PluralName
%}
%extend Namespace::Name {
void add_attribute(IMP::FloatKey k, IMP::Float v, bool opt) {
   self->get_particle()->add_attribute(k, v, opt);
}
IMP_SWIG_DECORATOR_ATTRIBUTE(Float, FloatKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(Int, IntKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(String, StringKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(Particle*, ParticleKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(Object*, ObjectKey);
IMP_SWIG_VOID_FORWARD_2(add_cache_attribute, IMP::IntKey, int);
IMP_SWIG_VOID_FORWARD_2(add_cache_attribute, IMP::ObjectKey, IMP::Object*);
IMP_SWIG_VOID_FORWARD_0(clear_caches);
IMP_SWIG_FORWARD_1(get_derivative, double, IMP::FloatKey);
IMP_SWIG_FORWARD_0(get_name, std::string);
IMP_SWIG_VOID_FORWARD_1(set_name, std::string);
IMP_SWIG_VOID_FORWARD_3(add_to_derivative, IMP::FloatKey, double, IMP::DerivativeAccumulator);
IMP_SWIG_FORWARD_0(get_float_attributes, IMP::FloatKeys);
IMP_SWIG_FORWARD_0(get_int_attributes, IMP::IntKeys);
IMP_SWIG_FORWARD_0(get_string_attributes, IMP::StringKeys);
IMP_SWIG_FORWARD_0(get_particle_attributes, IMP::ParticleKeys);
IMP_SWIG_FORWARD_0(get_object_attributes, IMP::ObjectKeys);
IMP_SWIG_VOID_FORWARD_2(set_is_optimized, IMP::FloatKey, bool);
IMP_SWIG_FORWARD_1(get_is_optimized, bool, IMP::FloatKey);
}
%pythonprepend Namespace::Name {

}
IMP_SWIG_VALUE_CHECKS(Namespace, Name);
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%enddef


%define IMP_SWIG_DECORATOR(Namespace, Name, PluralName)
IMP_SWIG_DECORATOR_BASE(Namespace, Name, PluralName);
%{
  BOOST_STATIC_ASSERT(IMP::internal::swig::Convert<Namespace::Name>::converter==3);
%}
%enddef


%define IMP_SWIG_DECORATOR_WITH_TRAITS(Namespace, Name, PluralName)
IMP_SWIG_DECORATOR_BASE(Namespace, Name, PluralName);
%{
  BOOST_STATIC_ASSERT(IMP::internal::swig::Convert<Namespace::Name>::converter==4);
%}
%enddef








%define IMP_SWIG_OBJECT_TUPLE(Namespace, Name, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, PluralName##Temp,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, Name, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Particle, Name,);
IMP_SWIG_VALUE_CHECKS(Namespace, Name);
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
%typemap(directorout) Namespace::Name const& {
  // hack to get around swig's evil value wrapper being randomly used
  IMP::internal::swig::assign($result, SWIG_NewPointerObj(new Namespace::Name(*$input), $descriptor(Namespace::Name*), SWIG_POINTER_OWN | %newpointer_flags));
 }
%typemap(directorin) Namespace::Name const& {
  $input = SWIG_NewPointerObj(new Namespace::Name($1_name), $descriptor(Namespace::Name*), SWIG_POINTER_OWN | %newpointer_flags);
 }
IMP_SWIG_VALUE_CHECKS(Namespace, Name);
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
%pythoncode %{
  def PairName(a,b):
    return (a,b)
  PluralName= list
%}
%enddef

%define IMP_SWIG_SEQUENCE_PAIR(Namespace, Name0, Name1, PairName)
%typemap(out) Namespace::PairName {
  PyObject *first=IMP::internal::swig::Convert<Namespace::PairName::first_type >::create_python_object(IMP::internal::swig::ValueOrObject<Namespace::PairName::first_type >::get($1.first), $descriptor(Name0), SWIG_POINTER_OWN);
  PyObject *second=IMP::internal::swig::Convert<Namespace::PairName::second_type >::create_python_object(IMP::internal::swig::ValueOrObject<Namespace::PairName::second_type >::get($1.second), $descriptor(Name1), SWIG_POINTER_OWN);

  $result=PyTuple_New(2);
  PyTuple_SetItem($result,0,first);
  PyTuple_SetItem($result,1,second);
 }
%pythoncode %{
  def PairName(a,b):
    return (a,b)
%}
%enddef


%define IMP_SWIG_GRAPH(Namespace, Name, Type, Label)
%inline %{
namespace IMP {
namespace internal {
template <class G, class L>
class BoostDigraph;
}
}
%}
%typemap(out) Namespace::Type {
  typedef IMP::internal::BoostDigraph<Namespace::Type, Label > GT;
  IMP_NEW(GT, ret, ($1));
  IMP::internal::ref(ret.get());
  %set_output(SWIG_NewPointerObj(%as_voidptr(ret), $descriptor(IMP::internal::BoostDigraph<Namespace::Type, Label >*), $owner | SWIG_POINTER_OWN));
 }
%typemap(out) Namespace::Type const& {
  typedef IMP::internal::BoostDigraph<Namespace::Type, Label > GT;
  IMP_NEW(GT, ret, (*$1));
  IMP::internal::ref(ret.get());
  %set_output(SWIG_NewPointerObj(%as_voidptr(ret), $descriptor(IMP::internal::BoostDigraph<Namespace::Type, Label >*), $owner | SWIG_POINTER_OWN));
 }
%typemap(in) Namespace::Type const& {
      void *vp;
      int res=SWIG_ConvertPtr($input, &vp, $descriptor(IMP::internal::BoostDigraph<Namespace::Type, Label >*), 0 );
      if (!SWIG_IsOK(res)) {
        IMP_THROW( "Wrong type.", IMP::ValueException);
      }
      if (!vp) {
        IMP_THROW( "Wrong type.", IMP::ValueException);
      }
      IMP::internal::BoostDigraph<Namespace::Type, Label >* p= reinterpret_cast< IMP::internal::BoostDigraph<Namespace::Type, Label >*>(vp);
      $1= &p->access_graph();
 }
%template(Name) ::IMP::internal::BoostDigraph< Namespace::Type, Label>;
%enddef
