/**
 *  \file test_associations.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <boost/shared_ptr.hpp>
#include <RMF/FileHandle.h>

namespace {
struct MyInt {
  int i;
};
int get_uint(MyInt i) {
  return i.i;
}

void test(const char *fname)
{
  RMF::FileHandle fh = RMF::create_rmf_file(fname);
  RMF::NodeHandle c0 = fh.get_root_node().add_child("c0", RMF::GEOMETRY);
  RMF::NodeHandle c1 = fh.get_root_node().add_child("c1", RMF::GEOMETRY);
  int a0 = 1;
  std::cout << "set int" << std::endl;
  c0.set_association(&a0);
  std::cout << "set string" << std::endl;
  std::string st0("hi");
  c1.set_association(&st0);
  std::cout << "get int" << std::endl;
  RMF::NodeHandle b0 = fh.get_node_from_association(&a0);
  assert(b0 == c0);
  std::cout << "get string" << std::endl;
  RMF::NodeHandle b1 = fh.get_node_from_association(&st0);
  std::cout << "check equality " << b1 << " " << c1 << std::endl;
  assert(b1 == c1);
  assert(c0.get_association<int*>() == &a0);


  RMF::NodeHandle c2 = fh.get_root_node().add_child("c2", RMF::GEOMETRY);
  boost::shared_ptr<int> si(new int(3));
  c2.set_association(si);
  RMF::NodeHandle c2b = fh.get_node_from_association(si);
  assert(c2 == c2b);
  boost::shared_ptr<int> sib = c2.get_association<boost::shared_ptr<int> >();
  assert(sib == si);


  RMF::NodeHandle c3 = fh.get_root_node().add_child("c3", RMF::GEOMETRY);
  MyInt sint = {6};
  c3.set_association(sint);
  RMF::NodeHandle c3b = fh.get_node_from_association(sint);
  assert(c3 == c3b);
  MyInt sintb = c3.get_association<MyInt >();
  assert(sintb.i == sint.i);
}
}

int main(int, char *[]) {
  try {
    // don't have tmp file support at this point
    const char fname[] = "/tmp/assoc.rmf";
    test(fname);
    remove(fname);
  } catch (const std::exception &e) {
    std::cerr << "Terminated with error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
