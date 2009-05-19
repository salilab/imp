
/* Provide our own implementations for some operators */
//%ignore IMP::algebra::BoundingBoxD::operator[];
%ignore IMP::algebra::BoundingBoxD::operator+=;

/* Make sure that we return the original Python object from C++ inplace
   operators (not a new Python proxy around the same C++ object) */
namespace IMP {
 namespace algebra {
  %feature("shadow") BoundingBoxD::__iadd__(const BoundingBoxD &) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
  %feature("shadow") BoundingBoxD::__iadd__(double) %{
    def __iadd__(self, *args):
        $action(self, *args)
        return self
  %}
 }
}

%extend IMP::algebra::BoundingBoxD {
  VectorD<D> __getitem__(unsigned int index) const {
    if (index >= 2) throw IMP::IndexException("");
    return self->get_corner(index);
  }
  /*void __setitem__(unsigned int index, double val) {
    self->operator[](index) = val;
  }*/
  /* Ignore C++ return value from inplace operators, so that SWIG does not
     generate a new SWIG wrapper for the return value (see above). */
  void __iadd__(const BoundingBoxD &o) { self->operator+=(o); }
  void __iadd__(double o) { self->operator+=(o); }
    unsigned int __len__() {return 2;}
};

%include "IMP/algebra/BoundingBoxD.h"

namespace IMP {
 namespace algebra {
   %template(BoundingBox3D) BoundingBoxD<3>;
   %template(BoundingBox3Ds) ::std::vector<BoundingBoxD<3> >;
 }
}
