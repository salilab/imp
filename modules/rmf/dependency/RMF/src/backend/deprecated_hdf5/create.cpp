/**
 *  \file RMF/paths.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include "factory.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "../BackwardsIO.h"
#include "../IOFactory.h"
#include "HDF5SharedData.h"
#include "RMF/compiler_macros.h"

namespace RMF {
namespace backends {
struct IO;
}  // namespace backends
}  // namespace RMF

RMF_ENABLE_WARNINGS

namespace RMF {
namespace hdf5_backend {
namespace {

typedef backends::BackwardsIO<HDF5SharedData> MIO;

struct HDF5Factory : public RMF::backends::IOFactory {
  virtual std::string get_file_extension() const RMF_OVERRIDE {
    return ".rmf-hdf5";
  }
  virtual boost::shared_ptr<RMF::backends::IO> read_file(
      const std::string& name) const RMF_OVERRIDE {
    return boost::make_shared<MIO>(name, false, true);
  }
  virtual boost::shared_ptr<RMF::backends::IO> create_file(
      const std::string& name) const RMF_OVERRIDE {
    return boost::make_shared<MIO>(name, true, false);
  }
  virtual ~HDF5Factory() {}
};

}  // namespace
std::vector<boost::shared_ptr<backends::IOFactory> > get_factories() {
  return std::vector<boost::shared_ptr<backends::IOFactory> >(
      1, boost::make_shared<HDF5Factory>());
}
}  // namespace avro_backend
}  // namespace RMF

RMF_DISABLE_WARNINGS
