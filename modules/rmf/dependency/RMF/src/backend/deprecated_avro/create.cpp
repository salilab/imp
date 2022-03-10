/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "factory.h"

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
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

typedef backends::BackwardsIO<avro_backend::AvroSharedData<
    avro_backend::SingleAvroFile> > SingleAvroShareData;
typedef backends::BackwardsIO<avro_backend::AvroSharedData<
    avro_backend::MultipleAvroFileWriter> > AvroWriterShareData;
typedef backends::BackwardsIO<avro_backend::AvroSharedData<
    avro_backend::MultipleAvroFileReader> > AvroReaderShareData;

struct SingleTextAvroFactory : public RMF::backends::IOFactory {
  virtual std::string get_file_extension() const override {
    return ".rmf-text";
  }
  virtual boost::shared_ptr<RMF::backends::IO> read_file(
      const std::string& name) const override {
    return boost::make_shared<SingleAvroShareData>(name, false, true);
  }
  virtual boost::shared_ptr<RMF::backends::IO> create_file(
      const std::string& name) const override {
    return boost::make_shared<SingleAvroShareData>(name, true, false);
  }
  virtual ~SingleTextAvroFactory() {}
};

struct SingleAvroFactory : public SingleTextAvroFactory {
  virtual std::string get_file_extension() const override {
    return ".rmfa";
  }
  /*virtual boost::shared_ptr<RMF::backends::IO> create_buffer(
      BufferHandle buffer) const override {
    return boost::make_shared<SingleAvroShareData>(buffer);
    }*/
  virtual boost::shared_ptr<RMF::backends::IO> read_buffer(
      BufferConstHandle buffer) const override {
    try {
      return boost::make_shared<SingleAvroShareData>(buffer);
    }
    catch (const std::exception &e) {
      RMF_INFO("Can't read buffer with old reader: " << e.what());
      return boost::shared_ptr<RMF::backends::IO>();
    }
  }
  virtual ~SingleAvroFactory() {}
};

struct MultipleAvroFactory : public RMF::backends::IOFactory {
  virtual std::string get_file_extension() const override {
    return ".rmf-avro";
  }
  virtual boost::shared_ptr<RMF::backends::IO> read_file(
      const std::string& name) const override {
    return boost::make_shared<AvroReaderShareData>(name, false, true);
  }
  virtual boost::shared_ptr<RMF::backends::IO> create_file(
      const std::string& name) const override {
    return boost::make_shared<AvroWriterShareData>(name, true, false);
  }
  virtual ~MultipleAvroFactory() {}
};
}  // namespace
std::vector<boost::shared_ptr<backends::IOFactory> > get_factories() {
  std::vector<boost::shared_ptr<backends::IOFactory> > ret;
  ret.push_back(boost::make_shared<MultipleAvroFactory>());
  ret.push_back(boost::make_shared<SingleAvroFactory>());
  ret.push_back(boost::make_shared<SingleTextAvroFactory>());
  return ret;
}
}  // namespace avro_backend
}  // namespace RMF

RMF_DISABLE_WARNINGS
