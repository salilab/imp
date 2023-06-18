/**
 *  \file has_cereal_raw_pointer.cpp
 *  \brief Check if cereal uses raw pointers for registerSharedPointer()
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#include <cereal/archives/binary.hpp>
#include <sstream>

int main()
{
  void *void_ptr = nullptr;
  std::stringstream ss;
  cereal::BinaryOutputArchive oarchive(ss);
  // Old versions of cereal use a raw pointer here; newer versions want
  // a shared_ptr instead
  oarchive.registerSharedPointer(void_ptr);
  return 0;
}
