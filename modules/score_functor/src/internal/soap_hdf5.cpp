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
static herr_t errwalkfunc(unsigned n, const H5E_error_t *err_desc, void *)
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

Hdf5Dataspace::Hdf5Dataspace(Hdf5Dataset &dataset) : id_(-1) {
  IMP_HDF5_ERRCHECK(id_ = H5Dget_space(dataset.get()));
}

Hdf5Dataspace::Hdf5Dataspace(Hdf5Attribute &attribute) : id_(-1) {
  IMP_HDF5_ERRCHECK(id_ = H5Aget_space(attribute.get()));
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
