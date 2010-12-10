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
  %template(DoubleVector) vector<double>;
  %template(IntVector) vector<int>;
}

%include "IMP/algebra/GeometricHash.h"

namespace IMP {
  namespace algebra {
    %template(FPoint2) Point<double, 2>;
    %template(FPoint3) Point<double, 3>;
    %template(FPoint4) Point<double, 4>;
    %template(FPoint5) Point<double, 5>;
    %template(IPoint2) Point<int, 2>;
    %template(IPoint3) Point<int, 3>;
    %template(IPoint4) Point<int, 4>;
    %template(IPoint5) Point<int, 5>;
    %template(GeometricHash2) GeometricHash<int, double, 2>;
    %template(GeometricHash3) GeometricHash<int, double, 3>;
    %template(GeometricHash4) GeometricHash<int, double, 4>;
    %template(GeometricHash5) GeometricHash<int, double, 5>;
  }
}
