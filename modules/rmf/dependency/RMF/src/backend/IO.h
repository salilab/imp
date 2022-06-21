/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_IO_H
#define RMF_INTERNAL_IO_H

#include <boost/shared_ptr.hpp>
#include <string>

#include "RMF/BufferConstHandle.h"
#include "RMF/BufferHandle.h"
#include "RMF/config.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"

namespace RMF {
class BufferConstHandle;
class BufferHandle;

namespace internal {
class SharedData;
}  // namespace internal
}  // namespace RMF

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backends {

struct IO {
  virtual void save_loaded_frame(internal::SharedData *shared_data) = 0;
  virtual void load_loaded_frame(internal::SharedData *shared_data) = 0;

  virtual void save_static_frame(internal::SharedData *shared_data) = 0;
  virtual void load_static_frame(internal::SharedData *shared_data) = 0;

  /** @} */
  /** \name File
      - description
      - producer
      - format
      - version
   @{ */
  virtual void load_file(internal::SharedData *shared_data) = 0;
  virtual void save_file(const internal::SharedData *shared_data) = 0;
  /** @} */
  /** \name Node Hierarchy
      @{ */
  virtual void load_hierarchy(internal::SharedData *shared_data) = 0;
  virtual void save_hierarchy(const internal::SharedData *shared_data) = 0;
  /** @} */
  virtual void flush() = 0;
  virtual ~IO() {}
};

RMFEXPORT boost::shared_ptr<IO> create_file(const std::string &name);
RMFEXPORT boost::shared_ptr<IO> create_buffer(BufferHandle buffer);
RMFEXPORT boost::shared_ptr<IO> read_file(const std::string &name);
RMFEXPORT boost::shared_ptr<IO> read_buffer(BufferConstHandle buffer);

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_IO_H */
