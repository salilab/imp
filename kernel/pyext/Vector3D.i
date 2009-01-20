
/* Provide our own implementations for some operators */
%ignore IMP::VectorD::operator[];
%ignore IMP::VectorD::operator+=;
%ignore IMP::VectorD::operator*=;
%ignore IMP::VectorD::operator/=;
%ignore IMP::VectorD::operator-=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
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

%extend IMP::VectorD {
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

%include "IMP/VectorD.h"

namespace IMP {
   %template(Vector3D) VectorD<3>;
   %template(Vector4D) VectorD<4>;
   %template(random_vector_on_sphere) random_vector_on_sphere<3>;
   %template(random_vector_in_sphere) random_vector_in_sphere<3>;
   %template(random_vector_in_box) random_vector_in_box<3>;
   %template(random_vector_on_unit_sphere) random_vector_on_unit_sphere<3>;
   %template(random_vector_in_unit_sphere) random_vector_in_unit_sphere<3>;
   %template(random_vector_in_unit_box) random_vector_in_unit_box<3>;
   %template(distance) distance<3>;
   %template(squared_distance) squared_distance<3>;
   %template(Vector3Ds) ::std::vector<VectorD<3> >;
}
