/* Provide our own implementations for some operators */
%ignore IMP::Vector3D::operator[];
%ignore IMP::Vector3D::operator+=;
%ignore IMP::Vector3D::operator*=;
%ignore IMP::Vector3D::operator/=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
  %feature("shadow") Vector3D::__iadd__(const Vector3D &) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Vector3D::__imul__(Float) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") Vector3D::__idiv__(Float) %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
}

%extend IMP::Vector3D {
  Float __getitem__(unsigned int index) const {
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, Float val) {
    self->operator[](index) = val;
  }
  /* Ignore C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const Vector3D &o) { self->operator+=(o); }
  void __imul__(Float f) { self->operator*=(f); }
  void __idiv__(Float f) { self->operator/=(f); }
};

%include "IMP/Vector3D.h"
