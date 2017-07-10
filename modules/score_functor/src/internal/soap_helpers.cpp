/**
 * \file soap_helpers.cpp
 * \brief Helper functions for SOAP.
 *
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
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

void SoapDoublets::read(Hdf5File &file_id) {
  Hdf5Group group(file_id.get(), "/library/tuples");
  Hdf5Dataset ntypes_ds(group.get(), "ntypes");
  std::vector<int> ntypes = ntypes_ds.read_int_vector();
  unsigned total_types = std::accumulate(ntypes.begin(), ntypes.end(), 0) * 3;

  Hdf5Dataset type_names_ds(group.get(), "type_names");
  std::vector<std::string> type_names = type_names_ds.read_string_vector();

  if (type_names.size() != total_types) {
    IMP_THROW("Number of atom types (" << type_names.size()
                                       << ") does not match sum of ntypes ("
                                       << total_types << ")",
              ValueException);
  }

  int class_id = 0;
  for (unsigned i = 0; i < type_names.size(); i += 3) {
    atom::ResidueType rt(type_names[i]);
    atom::AtomType at1(type_names[i + 1]);
    atom::AtomType at2(type_names[i + 2]);
    doublets_[std::make_pair(rt, at1)][at2] = class_id;
    if (--ntypes[class_id] == 0) {
      class_id++;
    }
  }
  n_classes_ = class_id;
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
