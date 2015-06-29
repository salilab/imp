%define RMF_SHADOW_NULLABLE(Class, function)
%feature("shadow") Class::function const %{
    def function(self, *args):
        return _handle_nullable( $action(self, *args))
%}
%enddef


%define RMF_SWIG_VECTOR(NAMESPACE, TYPE)
 %typemap(out) NAMESPACE::TYPE & front {
        $result = SWIG_NewPointerObj(%new_copy(*$1, $*ltype),
        $descriptor(NAMESPACE::TYPE &),
        SWIG_POINTER_OWN | %newpointer_flags);
  }
%typemap(out) NAMESPACE::TYPE & back {
        $result = SWIG_NewPointerObj(%new_copy(*$1, $*ltype),
        $descriptor(NAMESPACE::TYPE &),
        SWIG_POINTER_OWN | %newpointer_flags);
        }
%typemap(out) NAMESPACE::TYPE & __getitem__ {
        $result = SWIG_NewPointerObj(%new_copy(*$1, $*ltype),
        $descriptor(NAMESPACE::TYPE &),
        SWIG_POINTER_OWN | %newpointer_flags);
        }
%template(TYPE##s) std::vector<NAMESPACE::TYPE>;
%enddef
