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

void test()
{
  std::string buffer;
  {
    RMF::FileHandle fh = RMF::create_rmf_buffer(buffer);
    RMF::NodeHandle c0 = fh.get_root_node().add_child("c0", RMF::GEOMETRY);
    RMF::NodeHandle c1 = fh.get_root_node().add_child("c1", RMF::GEOMETRY);
  }
  {
    RMF::FileConstHandle fh = RMF::open_rmf_buffer_read_only(buffer);
    RMF::NodeConstHandle rt = fh.get_root_node();
    RMF::NodeConstHandles ch = rt.get_children();
    assert(ch.size() == 2);
  }
}
}

int main(int, char *[]) {
  test();
  return 0;
}
