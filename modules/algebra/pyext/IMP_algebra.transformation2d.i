
/* Provide our own implementations for some operators */
%ignore IMP::algebra::Transformation2D::operator*=;
%ignore IMP::algebra::Transformation2D::operator/=;
%ignore IMP::algebra::Rotation2D::operator*=;
%ignore IMP::algebra::Rotation2D::operator/=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") Transformation2D::__imul__(double) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Transformation2D::__itruediv__(double) %{
    def __itruediv__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Rotation2D::__imul__(double) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Rotation2D::__itruediv__(double) %{
    def __itruediv__(self, *args):
        $action(self, *args)
        return self
  %}

 }
}
