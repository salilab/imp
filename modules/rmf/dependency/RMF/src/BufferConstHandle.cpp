/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/BufferConstHandle.h"
#include "RMF/log.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <fstream>

RMF_ENABLE_WARNINGS

namespace RMF {

BufferConstHandle read_buffer(std::string file_name) {
  std::ifstream szstr(file_name.c_str(),
                      std::ios::in | std::ios::binary | std::ios::ate);
  unsigned int size = szstr.tellg();
  std::ifstream in(file_name.c_str(), std::ios::in | std::ios::binary);
  boost::shared_ptr<std::vector<char> > data =
      boost::make_shared<std::vector<char> >(size);
  RMF_TRACE("Found buffer of size " << data->size());
  in.read(&(*data)[0], data->size());
  return BufferConstHandle(data);
}

void write_buffer(BufferConstHandle buffer, std::string file_name) {
  std::ofstream out(file_name.c_str(), std::ios::out | std::ios::binary);
  RMF_TRACE("Writing buffer of size " << buffer.get_buffer().size());
  const std::vector<char> &buf = buffer.get_buffer();
  out.write(&buf[0], buffer.get_buffer().size());
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
