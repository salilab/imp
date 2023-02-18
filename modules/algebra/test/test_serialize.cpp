#include <fstream>
#include <IMP/algebra/VectorD.h>
#include <IMP/flags.h>
#include <cereal/archives/binary.hpp>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test vector serialization");

  std::ostringstream ofs;
  IMP::algebra::Vector3D v(1,2,3);
  cereal::BinaryOutputArchive oa(ofs);
  oa(v, v, v, v, v, v);

  std::istringstream ifs(ofs.str());
  cereal::BinaryInputArchive ia(ifs);
  IMP::algebra::Vector3D newv;
  ia(newv);

  assert(int(newv[0]) == 1);
  assert(int(newv[1]) == 2);
  assert(int(newv[2]) == 3);

  return 0;
}
