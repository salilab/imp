/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "RMF/BufferConstHandle.h"
#include "RMF/BufferHandle.h"
#include "RMF/config.h"
#include "backend/IOFactory.h"
#include "traits.h"
#include "factory.h"
#include "io.h"

namespace RMF {
namespace backends {
struct IO;
}  // namespace backends
}  // namespace RMF

RMF_ENABLE_WARNINGS
namespace RMF {

namespace avro2 {

template <bool GZIP, bool OLD>
class Avro2IOFileFactory : public backends::IOFactory {
 public:
  virtual std::string get_file_extension() const RMF_OVERRIDE {
    if (OLD) return ".rmf3";
    if (GZIP)
      return ".rmfz";
    else
      return ".rmf";
  }
  virtual boost::shared_ptr<backends::IO> read_file(const std::string &name)
      const RMF_OVERRIDE {
    return boost::make_shared<Avro2IO<ReaderTraits<FileReaderBase> > >(name);
  }
  virtual boost::shared_ptr<backends::IO> create_file(const std::string &name)
      const RMF_OVERRIDE {
    return boost::make_shared<Avro2IO<FileWriterTraits<GZIP> > >(name);
  }
};

class Avro2IOBufferFactory : public backends::IOFactory {
 public:
  virtual std::string get_file_extension() const RMF_OVERRIDE {
    return ".none";
  }
  virtual boost::shared_ptr<backends::IO> read_buffer(BufferConstHandle buffer)
      const RMF_OVERRIDE {
    try {
      return boost::make_shared<Avro2IO<ReaderTraits<BufferReaderBase> > >(
          buffer);
    }
    catch (const std::exception &e) {
      RMF_INFO("Avro2 reader can't read buffer: " << e.what());
      return boost::shared_ptr<backends::IO>();
    }
  }
  virtual boost::shared_ptr<backends::IO> create_buffer(BufferHandle buffer)
      const RMF_OVERRIDE {
    return boost::make_shared<Avro2IO<BufferWriterTraits> >(buffer);
  }
};

std::vector<boost::shared_ptr<backends::IOFactory> > get_factories() {
  std::vector<boost::shared_ptr<backends::IOFactory> > ret;
  ret.push_back(boost::make_shared<Avro2IOFileFactory<false, false> >());
  ret.push_back(boost::make_shared<Avro2IOFileFactory<true, false> >());
  ret.push_back(boost::make_shared<Avro2IOFileFactory<false, true> >());
  ret.push_back(boost::make_shared<Avro2IOBufferFactory>());
  return ret;
}

}  // namespace
} /* namespace RMF */

RMF_DISABLE_WARNINGS
