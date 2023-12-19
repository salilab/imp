/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "factory.h"

#include <boost/lexical_cast.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../BackwardsIO.h"
#include "AvroSharedData.h"
#include "MultipleAvroFileReader.h"
#include "MultipleAvroFileWriter.h"
#include "RMF/BufferHandle.h"
#include "RMF/compiler_macros.h"
#include "SingleAvroFile.h"
#include "backend/IOFactory.h"
#include "AvroSharedData.impl.h"

namespace RMF {
namespace backends {
struct IO;
}  // namespace backends
}  // namespace RMF

RMF_ENABLE_WARNINGS

namespace RMF {
namespace avro_backend {
namespace {

using SingleAvroShareData = backends::BackwardsIO<avro_backend::AvroSharedData<
    avro_backend::SingleAvroFile> >;
using AvroWriterShareData = backends::BackwardsIO<avro_backend::AvroSharedData<
    avro_backend::MultipleAvroFileWriter> >;
using AvroReaderShareData = backends::BackwardsIO<avro_backend::AvroSharedData<
    avro_backend::MultipleAvroFileReader> >;

struct SingleTextAvroFactory : public RMF::backends::IOFactory {
  virtual std::string get_file_extension() const override {
    return ".rmf-text";
  }
  virtual std::shared_ptr<RMF::backends::IO> read_file(
      const std::string& name) const override {
    return std::make_shared<SingleAvroShareData>(name, false, true);
  }
  virtual std::shared_ptr<RMF::backends::IO> create_file(
      const std::string& name) const override {
    return std::make_shared<SingleAvroShareData>(name, true, false);
  }
  virtual ~SingleTextAvroFactory() = default;
};

struct SingleAvroFactory : public SingleTextAvroFactory {
  virtual std::string get_file_extension() const override {
    return ".rmfa";
  }
  /*virtual std::shared_ptr<RMF::backends::IO> create_buffer(
      BufferHandle buffer) const override {
    return std::make_shared<SingleAvroShareData>(buffer);
    }*/
  virtual std::shared_ptr<RMF::backends::IO> read_buffer(
      BufferConstHandle buffer) const override {
    try {
      return std::make_shared<SingleAvroShareData>(buffer);
    }
    catch (const std::exception &e) {
      RMF_INFO("Can't read buffer with old reader: " << e.what());
      return std::shared_ptr<RMF::backends::IO>();
    }
  }
  virtual ~SingleAvroFactory() = default;
};

struct MultipleAvroFactory : public RMF::backends::IOFactory {
  virtual std::string get_file_extension() const override {
    return ".rmf-avro";
  }
  virtual std::shared_ptr<RMF::backends::IO> read_file(
      const std::string& name) const override {
    return std::make_shared<AvroReaderShareData>(name, false, true);
  }
  virtual std::shared_ptr<RMF::backends::IO> create_file(
      const std::string& name) const override {
    return std::make_shared<AvroWriterShareData>(name, true, false);
  }
  virtual ~MultipleAvroFactory() = default;
};
}  // namespace
std::vector<std::shared_ptr<backends::IOFactory> > get_factories() {
  std::vector<std::shared_ptr<backends::IOFactory> > ret;
  ret.push_back(std::make_shared<MultipleAvroFactory>());
  ret.push_back(std::make_shared<SingleAvroFactory>());
  ret.push_back(std::make_shared<SingleTextAvroFactory>());
  return ret;
}
}  // namespace avro_backend
}  // namespace RMF

RMF_DISABLE_WARNINGS
