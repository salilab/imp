/**
 * \file soap_helpers.cpp
 * \brief Helper functions for SOAP.
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/soap_helpers.h>

#ifdef IMP_SCORE_FUNCTOR_USE_HDF5

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

void SoapPotential::read(Hdf5File &file_id, const SoapDoublets &doublets) {
  read_feature_info(file_id);

  Hdf5Dataset dset(file_id.get(), "/mdt");
  Hdf5Dataspace space(dset);
  dims_ = space.get_simple_extent_dims();
  if (dims_.size() != 6) {
    IMP_THROW("SOAP potential should be 6-dimensional (it is "
                  << dims_.size() << "-dimensional)",
              ValueException);
  }

  int n_classes = doublets.get_number_of_classes();
  if (dims_[CLASS1] != n_classes || dims_[CLASS2] != n_classes) {
    IMP_THROW("SOAP potential dimensions ("
                  << dims_[CLASS1] << ", " << dims_[CLASS2]
                  << ") do not match the number of tuple classes (" << n_classes
                  << ")",
              ValueException);
  }

  stride_.resize(dims_.size());
  stride_[dims_.size() - 1] = 1;
  for (unsigned i = dims_.size() - 1; i > 0; --i) {
    stride_[i - 1] = stride_[i] * dims_[i];
  }
  int ndata = 1;
  for (unsigned i = 0; i < dims_.size(); ++i) {
    ndata *= dims_[i];
  }

  data_.reset(new float[ndata]);
  dset.read_float(data_.get());
}

void SoapPotential::read_feature_info(Hdf5File &file_id) {
  bin_min_.resize(4);
  bin_width_.resize(4);
  inv_bin_width_.resize(4);
  std::vector<int> ifeat = get_feature_types(file_id, 6);
  for (unsigned i = 0; i < 4; ++i) {
    get_feature_info(file_id, ifeat[i], bin_min_[i], bin_width_[i]);
    inv_bin_width_[i] = 1. / bin_width_[i];
  }
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif  // IMP_SCORE_FUNCTOR_USE_HDF5
