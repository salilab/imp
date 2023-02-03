
#ifndef SRC_BACKEND_HDF5_FACTORY__H_
#define SRC_BACKEND_HDF5_FACTORY__H_

#include "RMF/config.h"
#include "backend/IOFactory.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace hdf5_backend {
RMFEXPORT std::vector<std::shared_ptr<backends::IOFactory> > get_factories();
}
}

RMF_DISABLE_WARNINGS
#endif
