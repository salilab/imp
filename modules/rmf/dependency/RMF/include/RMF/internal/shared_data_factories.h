/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_FACTORIES_H
#define RMF_INTERNAL_SHARED_DATA_FACTORIES_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "RMF/BufferConstHandle.h"
#include "RMF/BufferHandle.h"
#include "RMF/config.h"
#include "RMF/internal/SharedData.h"
#include "SharedData.h"

namespace RMF {
class BufferConstHandle;
class BufferHandle;
namespace internal {
class SharedData;
}  // namespace internal
}  // namespace RMF

RMF_ENABLE_WARNINGS

namespace RMF {

namespace internal {

RMFEXPORT boost::shared_ptr<SharedData> create_file(const std::string& name);
RMFEXPORT boost::shared_ptr<SharedData> create_buffer(BufferHandle buffer);
RMFEXPORT boost::shared_ptr<SharedData> read_file(const std::string& name);
RMFEXPORT boost::shared_ptr<SharedData> read_buffer(BufferConstHandle buffer);

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_FACTORIES_H */
