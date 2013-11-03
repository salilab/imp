/**
 * \file soap_hdf5.cpp
 * \brief Handling of SOAP HDF5 files.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/soap_hdf5.h>
#include <IMP/base/check_macros.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

namespace {

/* Convert the topmost HDF5 error into an IMP exception */
herr_t errwalkfunc(unsigned n, const H5E_error_t *err_desc, void *)
{
  if (n == 0) {
    std::string errmsg = (err_desc->desc && strlen(err_desc->desc) > 0) ?
                         err_desc->desc : "HDF5 error";
    IMP_THROW(errmsg, IOException);
  }
  return 0;
}

}

void handle_hdf5_error()
{
  H5Ewalk(H5E_DEFAULT, H5E_WALK_DOWNWARD, errwalkfunc, NULL);
  IMP_THROW("Generic HDF5 error", IOException);
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
