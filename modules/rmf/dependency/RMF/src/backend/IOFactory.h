/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_IO_FACTORY_H
#define RMF_INTERNAL_IO_FACTORY_H

#include "RMF/config.h"
#include "RMF/internal/SharedData.h"
#include "RMF/BufferHandle.h"
#include "RMF/BufferConstHandle.h"
#include "RMF/infrastructure_macros.h"
#include <memory>

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backends {

class IOFactory {
 public:
  virtual std::string get_file_extension() const = 0;
  virtual std::shared_ptr<IO> read_buffer(BufferConstHandle) const {
    return std::shared_ptr<IO>();
  }
  virtual std::shared_ptr<IO> read_file(const std::string &) const {
    return std::shared_ptr<IO>();
  }
  virtual std::shared_ptr<IO> create_buffer(BufferHandle) const {
    return std::shared_ptr<IO>();
  }
  virtual std::shared_ptr<IO> create_file(const std::string &) const {
    return std::shared_ptr<IO>();
  }
  virtual ~IOFactory() {}
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_IO_FACTORY_H */
