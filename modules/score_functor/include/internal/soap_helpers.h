/**
 * \file soap_helpers.h
 * \brief Helper functions for SOAP.
 *
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SOAP_HELPERS_H
#define IMPSCORE_FUNCTOR_SOAP_HELPERS_H

#include <IMP/score_functor/score_functor_config.h>

#ifdef IMP_SCORE_FUNCTOR_USE_HDF5

#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include "soap_hdf5.h"
#include <boost/shared_array.hpp>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

// A single doublet in the Model.
// Only the 'other atom' in the doublet is stored here; the principal atom
// is that passed to the OrientedSoap score.
struct SoapModelDoublet {
  atom::Atom atom;
  int doublet_class;

  SoapModelDoublet(atom::Atom atom_in, int doublet_class_in)
      : atom(atom_in), doublet_class(doublet_class_in) {}
};

// Storage of all doublet library information
class SoapDoublets {
  // Mapping from atom type to SOAP type, for each possible 'other atom'
  // in an interaction
  typedef std::map<atom::AtomType, int> OtherAtoms;
  typedef std::pair<atom::ResidueType, atom::AtomType> Key;
  typedef std::map<Key, OtherAtoms> DoubletMap;

  DoubletMap doublets_;

  int n_classes_;

 public:
  SoapDoublets() : n_classes_(0) {}

  void read(Hdf5File &file_id);

  int get_number_of_classes() const { return n_classes_; }

  // Get a list of all doublets that the given atom is involved in
  std::vector<SoapModelDoublet> get_for_atom(atom::Atom a1) const {
    std::vector<SoapModelDoublet> ret;
    atom::Residue r = atom::get_residue(a1);
    atom::AtomType a1t = a1.get_atom_type();
    atom::ResidueType rt = r.get_residue_type();
    DoubletMap::const_iterator others = doublets_.find(std::make_pair(rt, a1t));
    if (others != doublets_.end()) {
      atom::Hierarchy mhd(r.get_particle());
      // Iterate through all atoms in the residue, and return only those
      // that are found in the doublet map
      for (unsigned int i = 0; i < mhd.get_number_of_children(); ++i) {
        atom::Atom a2(mhd.get_child(i));
        atom::AtomType a2t = a2.get_atom_type();
        OtherAtoms::const_iterator it = others->second.find(a2t);
        if (it != others->second.end()) {
          ret.push_back(SoapModelDoublet(a2, it->second));
        }
      }
    }
    return ret;
  }
};

// Storage of the SOAP statistical potential
class SoapPotential {
  std::vector<double> bin_min_, bin_width_, inv_bin_width_;
  boost::shared_array<float> data_;
  Ints dims_, stride_;

  void read_feature_info(Hdf5File &file_id);

  std::vector<int> get_feature_types(Hdf5File &file_id, int size) {
    Hdf5Dataset dset(file_id.get(), "/features");
    return dset.read_int_vector(size);
  }

  void get_feature_info(Hdf5File &file_id, int ifeat, double &bin_min,
                        double &bin_width) {
    std::string group_name =
        boost::str(boost::format("/library/feature%d") % ifeat);
    Hdf5Group group(file_id.get(), group_name);
    bin_min = group.read_float_attribute("first_bin");
    bin_width = group.read_float_attribute("bin_width");
  }

 public:
  static const int DISTANCE = 0, ANGLE1 = 1, ANGLE2 = 2, DIHEDRAL = 3,
                   CLASS1 = 4, CLASS2 = 5;
  SoapPotential() {}

  void read(Hdf5File &file_id, const SoapDoublets &doublets);

  // Get the maximum distance between any two principal atoms
  double get_max_range() const {
    return bin_min_[DISTANCE] + bin_width_[DISTANCE] * dims_[DISTANCE];
  }

  // Convert a raw value into the corresponding bin index
  int get_index(int i, double value) const {
    int ret = (int)((value - bin_min_[i]) * inv_bin_width_[i]);
    return (ret >= dims_[i]) ? -1 : ret;
  }

  // Look up an index in the potential
  float get_value(Ints index) const {
    int bin_index = 0;
    for (int i = 0; i < 6; ++i) {
      bin_index += index[i] * stride_[i];
    }
    return data_[bin_index];
  }

  float get_value(int dist_bin, double angle1, double angle2, double dihedral,
                  int class1_bin, int class2_bin) const {
    Ints index(6, 0);
    index[ANGLE1] = get_index(ANGLE1, angle1);
    index[ANGLE2] = get_index(ANGLE2, angle2);
    index[DIHEDRAL] = get_index(DIHEDRAL, dihedral);
    if (index[ANGLE1] < 0 || index[ANGLE2] < 0 || index[DIHEDRAL] < 0) {
      return 0.;
    } else {
      index[DISTANCE] = dist_bin;
      index[CLASS1] = class1_bin;
      index[CLASS2] = class2_bin;
      return get_value(index);
    }
  }
};

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

#endif /* IMPSCORE_FUNCTOR_SOAP_HELPERS_H */
