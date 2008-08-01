/* Provide our own implementations for operator[] */
%ignore IMP::Vector3D::operator[];

%extend IMP::Vector3D {
  Float __getitem__(unsigned int index) const {
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, Float val) {
    self->operator[](index) = val;
  }
};


/* C++ inplace operators return nothing, but Python expects them to
   return the original Python object */
namespace IMP {
  %feature("shadow") Vector3D::operator+= %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Vector3D::operator*= %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Vector3D::operator/= %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
}

%include "IMP/Vector3D.h"
