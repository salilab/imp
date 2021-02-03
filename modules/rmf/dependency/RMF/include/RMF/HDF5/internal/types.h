/**
 *  \file RMF/types.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_TYPES_H
#define RMF_INTERNAL_TYPES_H

#include "RMF/config.h"
#include "RMF/infrastructure_macros.h"
#include <hdf5.h>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {
namespace internal {
inline hid_t create_string_type() {
  hid_t tid1 = H5Tcopy(H5T_C_S1);
  RMF_HDF5_CALL(H5Tset_size(tid1, H5T_VARIABLE));
  return tid1;
}
inline hid_t get_string_type() {
  static hid_t ret = create_string_type();
  return ret;
}

}  // namespace internal
} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_TYPES_H */
