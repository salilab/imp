#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/grid_indexes.h>
#include <IMP/base/flags.h>
namespace {
template <class T>
int check(const T &t) {
  int ret = 0;
  for (unsigned int i = 0; i < t.get_dimension(); ++i) {
    assert(t[i] == i);
    ret += t[i];
  }
  return ret;
}
template <template <int D> class T, class VT>
int check_type() {
  int ret = 0;
  T<1> v1 = T<1>(VT(0));
  ret += check(v1);
  T<2> v2 = T<2>(VT(0), VT(1));
  ret += check(v2);
  T<3> v3(0, 1, 2);
  ret += check(v3);
  T<4> v4(0, 1, 2, 3);
  ret += check(v4);
  T<5> v5(0, 1, 2, 3, 4);
  ret += check(v5);
  T<6> v6(0, 1, 2, 3, 4, 5);
  ret += check(v6);
  return ret;
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Test D-dim constructors");
  int tot = 0;
  tot += check_type<IMP::algebra::VectorD, double>();
  tot += check_type<IMP::algebra::ExtendedGridIndexD, int>();
  tot += check_type<IMP::algebra::GridIndexD, int>();

  return tot == 0;  // reverse sense
}
