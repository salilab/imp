
/* Provide our own implementations for some operators */
%ignore IMP::algebra::Transformation3D::operator*=;
%ignore IMP::algebra::Transformation3D::operator/=;
%ignore IMP::algebra::Rotation3D::operator*=;
%ignore IMP::algebra::Rotation3D::operator/=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") Transformation3D::__imul__(double) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Transformation3D::__idiv__(double) %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Rotation3D::__imul__(double) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Rotation3D::__idiv__(double) %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}

 }
}

%extend IMP::algebra::Rotation3D {
  /* Support new-style "true" division */
  %pythoncode %{
  __truediv__ = __div__
  %}
}

%extend IMP::algebra::Transformation3D {
  /* Support new-style "true" division */
  %pythoncode %{
  __truediv__ = __div__
  %}
}
