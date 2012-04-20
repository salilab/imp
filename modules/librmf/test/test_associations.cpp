/**
 *  \file test_associations.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <boost/shared_ptr.hpp>
#include <RMF/FileHandle.h>

int main(int, char *[]) {
  // don't have tmp file support at this point
  RMF::FileHandle fh= RMF::create_rmf_file("/tmp/assoc.rmf");
  RMF::NodeHandle c0= fh.get_root_node().add_child("c0", RMF::GEOMETRY);
  RMF::NodeHandle c1= fh.get_root_node().add_child("c1", RMF::GEOMETRY);
  int a0= 1;
  std::cout << "set int" << std::endl;
  c0.set_association(&a0);
  std::cout << "set string" << std::endl;
  std::string st0("hi");
  c1.set_association(&st0);
  std::cout << "get int" << std::endl;
  RMF::NodeHandle b0= fh.get_node_from_association(&a0);
  assert(b0==c0);
  std::cout << "get string" << std::endl;
  RMF::NodeHandle b1= fh.get_node_from_association(&st0);
  std::cout << "check equality " << b1 << " " << c1 << std::endl;
  assert(b1==c1);
  assert(c0.get_association<int*>()==&a0);


  RMF::NodeHandle c2= fh.get_root_node().add_child("c2", RMF::GEOMETRY);
  boost::shared_ptr<int> si(new int(3));
  c2.set_association(si);
  RMF::NodeHandle c2b= fh.get_node_from_association(si);
  assert(c2==c2b);
  boost::shared_ptr<int> sib= c2.get_association<boost::shared_ptr<int> >();
  assert(sib==si);
  return 0;
}
