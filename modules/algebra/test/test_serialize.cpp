#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <IMP/algebra/VectorD.h>
#include <IMP/flags.h>

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test vector serialization");

  std::ostringstream ofs;
  IMP::algebra::Vector3D v(1,2,3);
  boost::archive::text_oarchive oa(ofs);
  oa << v << v << v << v << v << v;

  std::ostringstream bofs;
  IMP::algebra::Vector3D v2(4,5,6);
  boost::archive::binary_oarchive boa(bofs);
  boa << v2 << v2 << v2 << v2 << v2 << v2;

  std::istringstream ifs(ofs.str());
  boost::archive::text_iarchive ia(ifs);
  IMP::algebra::Vector3D newv;
  ia >> newv;

  assert(int(newv[0]) == 1);
  assert(int(newv[1]) == 2);
  assert(int(newv[2]) == 3);

  return 0;
}
