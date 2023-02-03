/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <memory>
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
  virtual std::string get_file_extension() const override {
    if (OLD) return ".rmf3";
    if (GZIP)
      return ".rmfz";
    else
      return ".rmf";
  }
  virtual std::shared_ptr<backends::IO> read_file(const std::string &name)
      const override {
    return std::make_shared<Avro2IO<ReaderTraits<FileReaderBase> > >(name);
  }
  virtual std::shared_ptr<backends::IO> create_file(const std::string &name)
      const override {
    return std::make_shared<Avro2IO<FileWriterTraits<GZIP> > >(name);
  }
};

class Avro2IOBufferFactory : public backends::IOFactory {
 public:
  virtual std::string get_file_extension() const override {
    return ".none";
  }
  virtual std::shared_ptr<backends::IO> read_buffer(BufferConstHandle buffer)
      const override {
    try {
      return std::make_shared<Avro2IO<ReaderTraits<BufferReaderBase> > >(
          buffer);
    }
    catch (const std::exception &e) {
      RMF_INFO("Avro2 reader can't read buffer: " << e.what());
      return std::shared_ptr<backends::IO>();
    }
  }
  virtual std::shared_ptr<backends::IO> create_buffer(BufferHandle buffer)
      const override {
    return std::make_shared<Avro2IO<BufferWriterTraits> >(buffer);
  }
};

std::vector<std::shared_ptr<backends::IOFactory> > get_factories() {
  std::vector<std::shared_ptr<backends::IOFactory> > ret;
  ret.push_back(std::make_shared<Avro2IOFileFactory<false, false> >());
  ret.push_back(std::make_shared<Avro2IOFileFactory<true, false> >());
  ret.push_back(std::make_shared<Avro2IOFileFactory<false, true> >());
  ret.push_back(std::make_shared<Avro2IOBufferFactory>());
  return ret;
}

}  // namespace
} /* namespace RMF */

RMF_DISABLE_WARNINGS
