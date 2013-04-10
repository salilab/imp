
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
%enddef






%typemap(in) IMP::kernel::Particle* {
  try {
    $1 = Convert<IMP::kernel::Particle >::get_cpp_object($input, $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typecheck(SWIG_TYPECHECK_POINTER) IMP::kernel::Particle* {
  try {
    Convert<IMP::kernel::Particle >::get_cpp_object($input, $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
    $1=1;
  } catch (...) {
    $1=0;
  }
 }
/*%{
  BOOST_STATIC_ASSERT(Convert<IMP::Particle>::converter ==2);
  BOOST_STATIC_ASSERT(Convert<IMP::internal::_TrivialDecorator>::converter ==3);
  %}*/


%define IMP_SWIG_SHOW_VALUE(Name)
  std::string __str__() const {
    std::ostringstream out;
    self->show(out);
    return out.str();
  }
  std::string __repr__() const {
    std::ostringstream out;
    self->show(out);
    return out.str();
  }
%enddef

%define IMP_SWIG_SHOW_OBJECT(Name)
  std::string __str__() const {
    std::ostringstream oss;
    oss << '"' << self->get_name() << '"';
    return oss.str();
  }
  std::string __repr__() const {
    std::ostringstream oss;
    oss << '"' << self->get_name() << '"';
    return oss.str();
  }
%enddef

%define IMP_SWIG_SHOWABLE_VALUE(Namespace, Name)
     %extend Namespace::Name {
  IMP_SWIG_SHOW_VALUE(Name);
 }
%enddef

%define IMP_SWIG_SHOWABLE_OBJECT(Namespace, Name)
     %extend Namespace::Name {
  IMP_SWIG_SHOW_OBJECT(Name);
 }
%enddef


%define IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(NamespaceName, NamespacePluralName, CONSTREF)
  %typemap(in) NamespacePluralName CONSTREF {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertSequence<NamespacePluralName, Convert< NamespaceName > >::get_cpp_object($input, $descriptor(NamespaceName*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) NamespacePluralName CONSTREF {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) NamespacePluralName CONSTREF {
  $1= ConvertSequence<NamespacePluralName, Convert< NamespaceName > >::get_is_cpp_object($input, $descriptor(NamespaceName*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
 }
%typemap(out) NamespacePluralName CONSTREF {
  $result = ConvertSequence<NamespacePluralName, Convert< NamespaceName > >::create_python_object(ValueOrObject<NamespacePluralName >::get($1), $descriptor(NamespaceName*), SWIG_POINTER_OWN);
 }
%typemap(directorout) NamespacePluralName CONSTREF {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertSequence<NamespacePluralName, Convert< NamespaceName > >::get_cpp_object($input, $descriptor(NamespaceName*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
 }
%typemap(directorin) NamespacePluralName CONSTREF {
  $input = ConvertSequence<NamespacePluralName, Convert< NamespaceName > >::create_python_object($1_name, $descriptor(NamespaceName*), SWIG_POINTER_OWN);
 }
%typemap(in) NamespacePluralName* {
  collections_like_##PluralName##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) NamespacePluralName* {
  collections_like_##PluralName##_must_be_returned_by_value_or_const_ref;
 }
%typemap(in) NamespacePluralName& {
  collections_like_##PluralName##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) NamespacePluralName& {
  collections_like_##PluralName##_must_be_returned_by_value_or_const_ref;
 }
%enddef





%define IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PluralName, CONSTREF)
IMP_SWIG_SEQUENCE_TYPEMAP_IMPL(Name, Namespace::PluralName, CONSTREF)
%enddef


%define IMP_SWIG_DOUBLY_NESTED_SEQUENCE_TYPEMAP(Name, FirstList, SecondList, ThirdList, CONSTREF)
  %typemap(in) ThirdList CONSTREF {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertSequence<ThirdList, ConvertSequence< SecondList, ConvertSequence<FirstList, Convert< Name > > > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) ThirdList CONSTREF {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) ThirdList CONSTREF {
  $1= ConvertSequence<ThirdList, ConvertSequence< SecondList, ConvertSequence< FirstList, Convert< Name > > > >::get_is_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
 }
%typemap(out) ThirdList CONSTREF {
  $result = ConvertSequence<ThirdList, ConvertSequence<SecondList, ConvertSequence< FirstList, Convert< Name > > > >::create_python_object(ValueOrObject<ThirdList >::get($1), $descriptor(Name*), SWIG_POINTER_OWN);
 }
%typemap(directorout) ThirdList CONSTREF {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertSequence<ThirdList, ConvertSequence< SecondList, ConvertSequence< FirstList, Convert< Name > > > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
 }
%typemap(directorin) ThirdList CONSTREF {
  $input = ConvertSequence<ThirdList, ConvertSequence< SecondList, ConvertSequence<FirstList, Convert< Name > > > >::create_python_object($1_name, $descriptor(Name*), SWIG_POINTER_OWN);
 }
%typemap(in) ThirdList* {
  collections_like_##ThirdList##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) ThirdList* {
  collections_like_##ThirdList##_must_be_returned_by_value_or_const_ref;
 }
%typemap(in) ThirdList& {
  collections_like_##ThirdList##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) ThirdList& {
  collections_like_##ThirdList##_must_be_returned_by_value_or_const_ref;
 }
%enddef




%define IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Name, IntermediateName, PluralName, CONSTREF)
  %typemap(in) PluralName CONSTREF {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertSequence<PluralName, ConvertSequence< IntermediateName, Convert< Name > > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) PluralName CONSTREF {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) PluralName CONSTREF {
  $1= ConvertSequence<PluralName, ConvertSequence< IntermediateName, Convert< Name > > >::get_is_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
 }
%typemap(out) PluralName CONSTREF {
  $result = ConvertSequence<PluralName, ConvertSequence< IntermediateName, Convert< Name > > >::create_python_object(ValueOrObject<PluralName >::get($1), $descriptor(Name*), SWIG_POINTER_OWN);
 }
%typemap(directorout) PluralName CONSTREF {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertSequence<PluralName, ConvertSequence< IntermediateName, Convert< Name > > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
 }
%typemap(directorin) PluralName CONSTREF {
  $input = ConvertSequence<PluralName, ConvertSequence< IntermediateName, Convert< Name > > >::create_python_object($1_name, $descriptor(Name*), SWIG_POINTER_OWN);
 }
%typemap(in) PluralName* {
  collections_like_##PluralName##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) PluralName* {
  collections_like_##PluralName##_must_be_returned_by_value_or_const_ref;
 }
%typemap(in) PluralName& {
  collections_like_##PluralName##_must_be_passed_by_value_or_const_ref;
 }
%typemap(out) PluralName& {
  collections_like_##PluralName##_must_be_returned_by_value_or_const_ref;
 }
%enddef







%define IMP_SWIG_OBJECT_CHECKS(Namespace, Name)
%typemap(out) Namespace::Name& {
  objects_like_##Name##_must_be_returned_by_pointer;
 }
%typemap(in) Namespace::Name& {
  objects_like_##Name##_must_be_passed_by_pointer;
  }
%typemap(out) Namespace::Name const& {
  objects_like_##Name##_must_be_returned_by_pointer;
 }
%typemap(in) Namespace::Name const& {
  objects_like_##Name##_must_be_passed_by_pointer;
  }
%typemap(in) IMP::base::Pointer<Namespace::Name> {
  objects_like_##Name##_must_be_passed_by_raw_pointer;
}
%typemap(out) IMP::base::Pointer<Namespace::Name> {
  objects_like_##Name##_must_be_passed_by_raw_pointer;
}
%pythoncode %{
_object_types.append(#Name)
%}
%enddef






%define IMP_SWIG_VALUE_CHECKS(Namespace, Name, Type)
%typemap(in) Namespace::Name const& = Type const &;
%typecheck(SWIG_TYPECHECK_POINTER) Namespace::Name const& = Type const &;
%typemap(in) Namespace::Name & {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
}

%typemap(in) Namespace::Name *self = Type *;
%typecheck(SWIG_TYPECHECK_POINTER) Namespace::Name *self = Type *;
%typemap(in) Namespace::Name * {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
}
%typemap(directorin) Namespace::Name * {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
  }



//%typemap(out) Namespace::Name const& = SWIGTYPE const &;
%typemap(out) Namespace::Name & {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_non_const_ref;
}
// we apparently need this both here and below for different swig versions
%typemap(out) Namespace::Name *Namespace::Name = Type *;
%typemap(out) Namespace::Name *self = Type *;
%typemap(out) Namespace::Name * {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_pointer;
}
%typemap(out) Namespace::Name *Namespace::Name::Name = Type *;

%typemap(directorout) Namespace::Name & {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_non_const_ref;
}
%typemap(directorout) Namespace::Name * {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_pointer;
}
%enddef











%define IMP_SWIG_OBJECT_INSTANCE(Namespace,Name, Nicename, PluralName)
IMP_SWIG_VALUE_CHECKS(Namespace, PluralName, SWIGTYPE);
%typemap(out) Namespace::Name* {
  if (!($owner & SWIG_POINTER_NEW)) {
    // out typemaps are also called for constructors, which already use %ref
    // to increase the reference count. So don't do it twice.
    IMP::base::internal::ref($1);
  }
  %set_output(SWIG_NewPointerObj(%as_voidptr($1), $descriptor(Namespace::Name *), $owner | SWIG_POINTER_OWN));
 }
/*%typemap(in) const Namespace::Name& {
  IMP_COMPILE_WARNING("Objects should be passed by pointer.");
  }*/
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName##Temp, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName##Temp,);
IMP_SWIG_OBJECT_CHECKS(Namespace, Name);
%feature("valuewrapper") PluralName;
%feature("valuewrapper") PluralName##Temp;
%template(_object_cast_to_##Name) IMP::base::object_cast<Namespace::Name>;
%enddef

%define IMP_SWIG_OBJECT(Namespace,Name, PluralName)
IMP_SWIG_OBJECT_INSTANCE(Namespace, Name, Name, PluralName);
IMP_SWIG_SHOWABLE_OBJECT(Namespace, Name);
%extend Namespace::Name {
  %pythoncode %{
    def get_from(o):
       return _object_cast_to_##Name(o)
    get_from = staticmethod(get_from)
  %}
 }
%enddef

%define IMP_SWIG_OBJECT_TEMPLATE(Namespace, Name)
IMP_SWIG_SHOWABLE_OBJECT(Namespace, Name);
%extend Namespace::Name {
  %pythoncode %{
    def get_from(o):
       return _object_cast_to_##Name(o)
    get_from = staticmethod(get_from)
  %}
 }
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
          return IMP.VersionInfo("python", "0")
    def get_from(o):
       return _object_cast_to_##Name(o)
    get_from = staticmethod(get_from)
  %}
}
IMP_SWIG_DIRECTOR(Namespace, Name);
%{
namespace {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
  void test_##PluralName##s() {
    Namespace::PluralName nm;
  }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}
%}
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
IMP_SWIG_VOID_FORWARD_2(add_attribute, IMP::kernel::Key, IMP::Type);
IMP_SWIG_FORWARD_1(get_value, IMP::Type, IMP::kernel::Key);
IMP_SWIG_VOID_FORWARD_2(set_value, IMP::kernel::Key, IMP::Type);
IMP_SWIG_VOID_FORWARD_1(remove_attribute, IMP::kernel::Key);
IMP_SWIG_FORWARD_1(has_attribute, bool, IMP::kernel::Key);
%enddef








%define IMP_SWIG_DECORATOR_BASE(Namespace, Name, PluralName)
IMP_SWIG_VALUE_CHECKS(Namespace, Name, SWIGTYPE);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName,);
%pythoncode %{
def PluralName(l=[]):
    return [Name(x) for x in l]
_plural_types.append(#PluralName)
%}
%extend Namespace::Name {
void add_attribute(IMP::kernel::FloatKey k, IMP::Float v, bool opt) {
   self->get_particle()->add_attribute(k, v, opt);
}
IMP_SWIG_DECORATOR_ATTRIBUTE(Float, FloatKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(Int, IntKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(String, StringKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(Particle*, ParticleIndexKey);
IMP_SWIG_DECORATOR_ATTRIBUTE(Object*, ObjectKey);
IMP_SWIG_FORWARD_1(get_derivative, double, IMP::kernel::FloatKey);
IMP_SWIG_FORWARD_0(get_name, std::string);
IMP_SWIG_FORWARD_0(clear_caches, void);
IMP_SWIG_VOID_FORWARD_1(set_name, std::string);
IMP_SWIG_VOID_FORWARD_1(set_check_level, IMP::base::CheckLevel);
IMP_SWIG_VOID_FORWARD_3(add_to_derivative, IMP::kernel::FloatKey, double, IMP::kernel::DerivativeAccumulator);
IMP_SWIG_VOID_FORWARD_2(set_is_optimized, IMP::kernel::FloatKey, bool);
IMP_SWIG_FORWARD_1(get_is_optimized, bool, IMP::kernel::FloatKey);
IMP_SWIG_FORWARD_0(get_check_level, IMP::base::CheckLevel);
}
%pythonprepend Namespace::Name {
}
%extend Namespace::Name {
  bool __eq__(Name o) const {
     return *self ==o;
  }
  bool __ne__(Name o) const {
     return *self !=o;
  }
  bool __le__(Name o) const {
     return *self <= o;
  }
  bool __lt__(Name o) const {
     return *self < o;
  }
  bool __ge__(Name o) const {
     return *self >= o;
  }
  bool __gt__(Name o) const {
     return *self > o;
  }
  bool __eq__(Particle *d) const {
    return self->get_particle() == d;
  }
  bool __ne__(Particle *d) const {
     return self->get_particle() != d;
  }
  bool __le__(Particle *d) const {
     return self->get_particle() <= d;
  }
  bool __lt__(Particle *d) const {
     return self->get_particle() < d;
  }
  bool __ge__(Particle *d) const {
     return self->get_particle() >= d;
  }
  bool __gt__(Particle *d) const {
     return self->get_particle() > d;
  }
}
%feature("valuewrapper") PluralName;
IMP_SWIG_SHOWABLE_VALUE(Namespace, Name);
%enddef





%define IMP_SWIG_DECORATOR(Namespace, Name, PluralName)
IMP_SWIG_DECORATOR_BASE(Namespace, Name, PluralName);
%{
  BOOST_STATIC_ASSERT(Convert< Namespace::Name >::converter==3);
%}
%pythoncode %{
_value_types.append(#Name)
%}
%enddef






%define IMP_SWIG_DECORATOR_WITH_TRAITS(Namespace, Name, PluralName)
IMP_SWIG_DECORATOR_BASE(Namespace, Name, PluralName);
%{
  BOOST_STATIC_ASSERT(Convert< Namespace::Name >::converter==4);
%}
%pythoncode %{
_value_types.append(#Name)
%}
%enddef








%define IMP_SWIG_ARRAY(Namespace, Name, PluralName, Member)
IMP_SWIG_VALUE_CHECKS(Namespace, Name, SWIGTYPE);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Member, Namespace::Name, Namespace::PluralName, const&);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Member, Namespace::Name, Namespace::PluralName,);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Member, Name, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Member, Name,);
%pythoncode %{
PluralName=list
_plural_types.append(#PluralName)
_value_types.append(#Name)
%}
%feature("valuewrapper") PluralName;
%enddef
 //SWIGTYPE_p_##SwigNamespace##Name





%define IMP_SWIG_VALUE_IMPL(Namespace, Name, TemplateName, UniqueName, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PluralName,);

%typemap(out) Namespace::Name const& {
  $result=SWIG_NewPointerObj(new Namespace::Name(*$1), $descriptor(Namespace::Name*), SWIG_POINTER_OWN | %newpointer_flags);
 }
%typemap(directorout) Namespace::Name const& {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, SWIG_NewPointerObj(new Namespace::Name(*$input), $descriptor(Namespace::Name*), SWIG_POINTER_OWN | %newpointer_flags));
 }
%typemap(directorin) Namespace::Name const& {
  $input = SWIG_NewPointerObj(new Namespace::Name($1_name), $descriptor(Namespace::Name*), SWIG_POINTER_OWN | %newpointer_flags);
 }
%pythoncode %{
PluralName=list
_plural_types.append(#PluralName)
_value_types.append(#Name)
%}
%feature("valuewrapper") PluralName;
%{
  namespace {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
  void test_##UniqueName() {
    Namespace::PluralName nm;
    //using namespace Namespace;
    //using namespace std;
    //float and all are not by reference
    //std::ostream& (*ptr)(std::ostream &, const Namespace::Name &)=operator<<;
  }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
  }
%}
%enddef


%define IMP_SWIG_VALUE_INSTANCE(Namespace, Name, TemplateName, PluralName)
IMP_SWIG_VALUE_CHECKS(Namespace, Name, SWIGTYPE);
IMP_SWIG_VALUE_IMPL(Namespace, Name, TemplateName, PluralName, PluralName);
%enddef


%define IMP_SWIG_VALUE_BUILTIN(Namespace, Name, PluralName, Type)
%typemap(in) Namespace::Name const& = Type const &;
%typecheck(SWIG_TYPECHECK_POINTER) Namespace::Name const& = Type const &;
%typemap(in) Namespace::Name & {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
}
%typemap(in) Namespace::Name * {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
}
%typemap(directorin) Namespace::Name * {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
  }
//%typemap(out) Namespace::Name const& = SWIGTYPE const &;
%typemap(out) Namespace::Name & {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_non_const_ref;
}

%typemap(out) Namespace::Name * {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_pointer;
}
%typemap(directorout) Namespace::Name & {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_non_const_ref;
}
%typemap(directorout) Namespace::Name * {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_pointer;
}
IMP_SWIG_VALUE_IMPL(Namespace, Name, TemplateName, PluralName, PluralName);
%enddef

%define IMP_SWIG_VALUE(Namespace, Name, PluralName)
IMP_SWIG_VALUE_INSTANCE(Namespace, Name, Name, PluralName)
IMP_SWIG_SHOWABLE_VALUE(Namespace, Name);
%enddef

 // a value that has implicit constructors
%define IMP_SWIG_VALUE_IMPLICIT(Namespace, Name, PluralName)
IMP_SWIG_VALUE_IMPL(Namespace, Name, Name, PluralName, PluralName);
IMP_SWIG_SHOWABLE_VALUE(Namespace, Name);
%enddef

%define IMP_SWIG_VALUE_TEMPLATE(Namespace, Name)
IMP_SWIG_SHOWABLE_VALUE(Namespace, Name);
%enddef


%define IMP_SWIG_GENERIC_OBJECT_TEMPLATE(Namespace, Name, lcname, argument)
%template(Name) Namespace::Generic##Name<argument>;
%template(create_##lcname) Namespace::create_##lcname<argument>;
%enddef



%define IMP_SWIG_NATIVE_VALUE(Name)

%typemap(in) Name & {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
}
%typemap(in) Name * {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
}
%typemap(directorin) Name * {
  values_like_##Name##_must_be_passed_by_value_or_const_ref_not_non_const_ref;
  }
//%typemap(out) Namespace::Name const& = SWIGTYPE const &;
/*%typemap(out) Name & {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_non_const_ref;
  }*/
%typemap(out) Name * {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_pointer;
}
%typemap(directorout) Name & {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_non_const_ref;
}
%typemap(directorout) Name * {
  values_like_##Name##_must_be_returned_by_value_or_const_ref_not_pointer;
}
%typemap(in) IMP::base::Vector< Name > const& {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) IMP::base::Vector< Name > const& {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) IMP::base::Vector< Name > const& {
  $1= ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::get_is_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
 }
%typemap(out) IMP::base::Vector< Name > const& {
  $result = ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::create_python_object(ValueOrObject<IMP::base::Vector< Name > >::get($1), $descriptor(Name*), SWIG_POINTER_OWN);
 }
%typemap(directorout) IMP::base::Vector< Name > const& {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
 }
%typemap(directorin) IMP::base::Vector< Name > const& {
  $input = ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::create_python_object($1_name, $descriptor(Name*), SWIG_POINTER_OWN);
 }

%typemap(in) IMP::base::Vector< Name > {
  try {
    // hack to get around swig's value wrapper being randomly used
    assign($1, ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
  } catch (const IMP::base::Exception &e) {
    //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
    PyErr_SetString(PyExc_TypeError, e.what());
    return NULL;
  }
 }
%typemap(freearg) IMP::base::Vector< Name > {
  delete_if_pointer($1);
 }
%typecheck(SWIG_TYPECHECK_POINTER) IMP::base::Vector< Name > {
  $1= ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::get_is_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*));
 }
%typemap(out) IMP::base::Vector< Name > {
  $result = ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::create_python_object(ValueOrObject<IMP::base::Vector< Name > >::get($1), $descriptor(Name*), SWIG_POINTER_OWN);
 }
%typemap(directorout) IMP::base::Vector< Name > {
  // hack to get around swig's evil value wrapper being randomly used
  assign($result, ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::get_cpp_object($input, $descriptor(Name*), $descriptor(IMP::kernel::Particle*), $descriptor(IMP::kernel::Decorator*)));
 }
%typemap(directorin) IMP::base::Vector< Name > {
  $input = ConvertSequence<IMP::base::Vector< Name >, Convert< Name > >::create_python_object($1_name, $descriptor(Name*), SWIG_POINTER_OWN);
 }

%enddef





%define IMP_SWIG_RAII_INSTANCE(Namespace, Name, NiceName)
  %typemap(in) Namespace::Name* {
  BOOST_STATIC_ASSERT($argnum==1); // RAII object Namespace::Name cannot be passed as an argument
try {
  $1=ConvertRAII<Namespace::Name >::get_cpp_object($input, $descriptor(Namespace::Name*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*));
} catch (const IMP::base::Exception &e) {
  //PyErr_SetString(PyExc_ValueError,"Wrong type in sequence");
  PyErr_SetString(PyExc_TypeError, e.what());
  return NULL;
  }
   }
%typemap(in) Namespace::Name {
 }
%typecheck(SWIG_TYPECHECK_POINTER) Namespace::Name * {
  $1= ConvertRAII<Namespace::Name >::get_is_cpp_object($input, $descriptor(Namespace::Name*), $descriptor(IMP::Particle*), $descriptor(IMP::Decorator*));
 }
%typemap(out) Namespace::Name {
 }
%pythoncode %{
  _raii_types.append(#Name)
%}
%enddef


%define IMP_SWIG_RAII(Namespace, Name)
IMP_SWIG_RAII_INSTANCE(Namespace, Name, Name)
IMP_SWIG_SHOWABLE_VALUE(Namespace, Name);
%enddef

%define IMP_SWIG_RAII_TEMPLATE(Namespace, Name)
IMP_SWIG_SHOWABLE_VALUE(Namespace, Name);
%enddef



%define IMP_SWIG_PAIR(Namespace, Name, PairName, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PairName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Namespace::Name, PairName,);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Namespace::Name, Namespace::PairName, Namespace::PluralName, const&);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Namespace::Name, Namespace::PairName, Namespace::PluralName,);
%feature("valuewrapper") PluralName;
%enddef

%define IMP_SWIG_NATIVE_PAIR(Namespace, Name, PairName, PluralName)
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PairName, const&);
IMP_SWIG_SEQUENCE_TYPEMAP(Namespace, Name, PairName,);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Name, Namespace::PairName, Namespace::PluralName, const&);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Name, Namespace::PairName, Namespace::PluralName,);
%feature("valuewrapper") PluralName;
%enddef

%define IMP_SWIG_NATIVE_VALUES_LIST(Namespace, Name, PluralName, PluralListName)
IMP_SWIG_VALUE_CHECKS(Namespace, PluralListName, SWIGTYPE);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Name, Namespace::PluralName, Namespace::PluralListName, const&);
IMP_SWIG_NESTED_SEQUENCE_TYPEMAP(Name, Namespace::PluralName, Namespace::PluralListName,);
IMP_SWIG_DOUBLY_NESTED_SEQUENCE_TYPEMAP(Name, Namespace::PluralName, Namespace::PluralListName, Namespace::PluralListName##s, const&);
IMP_SWIG_DOUBLY_NESTED_SEQUENCE_TYPEMAP(Name, Namespace::PluralName, Namespace::PluralListName, Namespace::PluralListName##s,);
%enddef

%define IMP_SWIG_SEQUENCE_PAIR(Namespace, Name0, Name1, PairName)
%typemap(out) Namespace::PairName {
  PyObject *first=ConvertSequence<Namespace::PairName::first_type, Convert< Namespace::PairName::first_type::value_type> >::create_python_object(ValueOrObject<Namespace::PairName::first_type >::get($1.first), $descriptor(Name0), SWIG_POINTER_OWN);
  PyObject *second=ConvertSequence<Namespace::PairName::second_type, Convert< Namespace::PairName::second_type::value_type> >::create_python_object(ValueOrObject<Namespace::PairName::second_type >::get($1.second), $descriptor(Name1), SWIG_POINTER_OWN);

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
  namespace base {
namespace internal {
template <class G, class L>
class BoostDigraph;
}
  }
}
%}
%typemap(out) Namespace::Type {
  typedef IMP::base::internal::BoostDigraph<Namespace::Type, Label > GT;
  IMP_NEW(GT, ret, ($1));
  IMP::base::internal::ref(ret.get());
  %set_output(SWIG_NewPointerObj(%as_voidptr(ret), $descriptor(IMP::base::internal::BoostDigraph<Namespace::Type, Label >*), $owner | SWIG_POINTER_OWN));
 }
%typemap(out) Namespace::Type const& {
  typedef IMP::base::internal::BoostDigraph<Namespace::Type, Label > GT;
  IMP_NEW(GT, ret, (*$1));
  IMP::base::internal::ref(ret.get());
  %set_output(SWIG_NewPointerObj(%as_voidptr(ret), $descriptor(IMP::base::internal::BoostDigraph<Namespace::Type, Label >*), $owner | SWIG_POINTER_OWN));
 }
%typecheck(SWIG_TYPECHECK_POINTER) Namespace::Type const& {
  void *vp;
  $1=SWIG_IsOK(SWIG_ConvertPtr($input, &vp, $descriptor(IMP::base::internal::BoostDigraph<Namespace::Type, Label >*), 0 ));
 }
%typemap(in) Namespace::Type const& {
      void *vp;
      int res=SWIG_ConvertPtr($input, &vp, $descriptor(IMP::base::internal::BoostDigraph<Namespace::Type, Label >*), 0 );
      if (!SWIG_IsOK(res)) {
        IMP_THROW( "Wrong type.", IMP::base::ValueException);
      }
      if (!vp) {
        IMP_THROW( "Wrong type.", IMP::base::ValueException);
      }
      IMP::base::internal::BoostDigraph<Namespace::Type, Label >* p= reinterpret_cast< IMP::base::internal::BoostDigraph<Namespace::Type, Label >*>(vp);
      $1= &p->access_graph();
 }
%template(Name) ::IMP::base::internal::BoostDigraph< Namespace::Type, Label>;
%pythoncode %{
_value_types.append(#Name)
%}
%rename(_##Type##VertexIndex) Type##VertexIndex;
%enddef
