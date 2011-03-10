/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/hdf5_wrapper.h>

IMPHDF5_BEGIN_NAMESPACE


HDF5Group::HDF5Group(std::string name, bool clear) {
  bool exists= (!clear && boost::filesystem::exists(name));
  HDF5Handle plist(H5Pcreate(H5P_FILE_ACCESS), &H5Pclose);
  IMP_HDF5_CALL(H5Pset_sieve_buf_size(plist, 1000000));
  IMP_HDF5_CALL(H5Pset_cache(plist, 0, 100000, 100000000, 0.0));
  if (exists) {
    h_= new HDF5SharedHandle(H5Fopen(name.c_str(), H5F_ACC_RDWR, plist),
                             &H5Fclose);
  } else {
    h_= new HDF5SharedHandle(H5Fcreate(name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
                                       plist), &H5Fclose);
  }
}
// create from possibly group
HDF5Group::HDF5Group(HDF5Group parent, std::string name) {
  h_= new HDF5SharedHandle(H5Gopen(parent.h_->get_hid(), name.c_str(),
                                   H5P_DEFAULT), &H5Gclose);
}
HDF5Group HDF5Group::add_child(std::string name) {
  IMP_USAGE_CHECK(!H5Lexists(h_->get_hid(), name.c_str(), H5P_DEFAULT),
                  "Child named " << name << " already exists");
  HDF5Handle(H5Gcreate2(h_->get_hid(), name.c_str(),
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
             &H5Gclose);
  return HDF5Group(*this, name);
}

unsigned int HDF5Group::get_number_of_children() const {
  unsigned int n= get_number_of_links();
  // later check that they are groups
  return n;
}
std::string HDF5Group::get_child_name(unsigned int i) const {
  static const int max_len=1000;
  char buf[max_len];
  IMP_HDF5_CALL(H5Lget_name_by_idx(h_->get_hid(), ".",
                                   H5_INDEX_NAME, H5_ITER_NATIVE, (hsize_t)i,
                                   buf, max_len, H5P_DEFAULT));
  return std::string(buf);
}
bool HDF5Group::get_has_child(std::string name) const {
  return H5Lexists(h_->get_hid(), name.c_str(), H5P_DEFAULT);
}
hid_t HDF5Group::get_handle() const {
  return h_->get_hid();
}
bool HDF5Group::get_child_is_group(unsigned int i) const {
  H5O_info_t info;
  HDF5Handle c(H5Oopen(h_->get_hid(), get_child_name(i).c_str(),
                       H5P_DEFAULT), &H5Oclose);
  IMP_HDF5_CALL(H5Oget_info(c, &info));
  return info.type== H5O_TYPE_GROUP; //H5O_TYPE_DATASET
}
bool HDF5Group::get_child_is_data_set(unsigned int i) const {
  H5O_info_t info;
  HDF5Handle c(H5Oopen(h_->get_hid(), get_child_name(i).c_str(),
                       H5P_DEFAULT), &H5Oclose);
  IMP_HDF5_CALL(H5Oget_info(c, &info));
  return info.type== H5O_TYPE_DATASET; //H5O_TYPE_DATASET
}

bool HDF5Group::get_has_attribute(std::string nm) const {
  return H5Aexists_by_name(h_->get_hid(), ".", nm.c_str(), H5P_DEFAULT) > 0;
}
IMPHDF5_END_NAMESPACE
