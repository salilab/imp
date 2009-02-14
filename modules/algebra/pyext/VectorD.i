
/* Provide our own implementations for some operators */
%ignore IMP::algebra::VectorD::operator[];
%ignore IMP::algebra::VectorD::operator+=;
%ignore IMP::algebra::VectorD::operator*=;
%ignore IMP::algebra::VectorD::operator/=;
%ignore IMP::algebra::VectorD::operator-=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") VectorD::__iadd__(const VectorD &) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") VectorD::__imul__(Float) %{
    def __imul__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") VectorD::__idiv__(Float) %{
    def __idiv__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") VectorD::__isub__(const VectorD &) %{
    def __isub__(self, *args):
        $action(self, *args)
        return self
  %}
 }
}

%extend IMP::algebra::VectorD {
  Float __getitem__(unsigned int index) const {
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, Float val) {
    self->operator[](index) = val;
  }
  /* Ignore C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const VectorD &o) { self->operator+=(o); }
  void __imul__(Float f) { self->operator*=(f); }
  void __idiv__(Float f) { self->operator/=(f); }
  void __isub__(const VectorD &o) { self->operator-=(o); }
};

%include "IMP/algebra/VectorD.h"
%include "IMP/algebra/Vector3D.h"

namespace IMP {
 namespace algebra {
   %template(Vector3D) VectorD<3>;
   %template(Vector4D) VectorD<4>;
   %template(distance) distance<3>;
   %template(squared_distance) squared_distance<3>;
   %template(Vector3Ds) ::std::vector<VectorD<3> >;
 }
}
