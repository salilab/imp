%ignore IMP::multifit::GeometricHash::begin();
%ignore IMP::multifit::GeometricHash::end();
%include "std_vector.i"
namespace std
{
  %template(DoubleVector) vector<double>;
  %template(IntVector) vector<int>;
}

%include "IMP/multifit/GeometricHash.h"

namespace IMP {
  namespace multifit {
    %template(GeometricHash2) GeometricHash<int, 2>;
    %template(GeometricHash3) GeometricHash<int, 3>;
    %template(GeometricHash4) GeometricHash<int, 4>;
    %template(GeometricHash5) GeometricHash<int, 5>;
  }
}
