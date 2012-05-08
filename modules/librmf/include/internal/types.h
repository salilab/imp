/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_TYPES_H
#define IMPLIBRMF_INTERNAL_TYPES_H

#include "../RMF_config.h"
#include <hdf5.h>


namespace RMF {
  namespace internal {
    inline hid_t create_string_type() {
      hid_t tid1 = H5Tcopy (H5T_C_S1);
      IMP_HDF5_CALL(H5Tset_size (tid1,H5T_VARIABLE));
      return tid1;
    }
    inline hid_t get_string_type() {
      static hid_t ret= create_string_type();
      return ret;
    }

    template <class T, class Ts, int Index>
  struct BaseTraits {
    typedef T Type;
    typedef Ts Types;
    static int get_index() {
      return Index;
    }
    static const T& get_fill_value() {
      return get_null_value();
    }
    static bool get_is_null_value(const T& f) {
      return f== get_null_value();
    }
    static const T& get_null_value() {
      static const T ret;
      return ret;
    }
  };

  } // namespace internal
} /* namespace RMF */


#endif /* IMPLIBRMF_INTERNAL_TYPES_H */
