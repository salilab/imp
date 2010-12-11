%ignore IMP::algebra::Point::operator[];
%extend IMP::algebra::Point {
  double __getitem__(unsigned int index) const {
    if ( index >= D ) throw IMP::IndexException("");
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, double val) {
    self->operator[](index) = val;
  }
  unsigned int __len__() {return D;}
};

%include "std_vector.i"
namespace std
{
  %template() vector<double>;
  %template() vector<int>;
}

%include "IMP/algebra/GeometricHash.h"

namespace IMP {
  namespace algebra {
    %template() Point<double, 2>;
    %template() Point<double, 3>;
    %template() Point<double, 4>;
    %template() Point<double, 5>;
    %template() Point<int, 2>;
    %template() Point<int, 3>;
    %template() Point<int, 4>;
    %template() Point<int, 5>;
    %template(GeometricHash2) GeometricHash<int, double, 2>;
    %template(GeometricHash3) GeometricHash<int, double, 3>;
    %template(GeometricHash4) GeometricHash<int, double, 4>;
    %template(GeometricHash5) GeometricHash<int, double, 5>;
  }
}
