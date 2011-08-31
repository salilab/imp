/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/HDF5Group.h>

namespace rmf {

HDF5Group::HDF5Group(HDF5SharedHandle *h): h_(h) {
}

// create from possibly group
HDF5Group::HDF5Group(HDF5Group parent, std::string name) {
  h_= new HDF5SharedHandle(H5Gopen(parent.h_->get_hid(), name.c_str(),
                                   H5P_DEFAULT), &H5Gclose);
}
HDF5Group HDF5Group::add_child(std::string name) {
  IMP_RMF_USAGE_CHECK(!H5Lexists(h_->get_hid(), name.c_str(), H5P_DEFAULT),
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



namespace {
hid_t get_parameters() {
  hid_t plist= H5Pcreate(H5P_FILE_ACCESS);
  IMP_HDF5_CALL(H5Pset_sieve_buf_size(plist, 1000000));
  IMP_HDF5_CALL(H5Pset_cache(plist, 0, 1000, 1000000, 0.0));
  return plist;
}
}

HDF5File create_hdf5_file(std::string name) {
  HDF5Handle plist(get_parameters(), H5Pclose);
  return HDF5File(new HDF5SharedHandle(H5Fcreate(name.c_str(),
                                                 H5F_ACC_TRUNC, H5P_DEFAULT,
                                                 plist), &H5Fclose));
}

HDF5File open_hdf5_file(std::string name) {
  HDF5Handle plist(get_parameters(), H5Pclose);
  return HDF5File(new HDF5SharedHandle(H5Fopen(name.c_str(),
                                               H5F_ACC_RDWR, plist),
                                       &H5Fclose));
}

HDF5File open_hdf5_file_read_only(std::string name) {
  HDF5Handle plist(get_parameters(), H5Pclose);
  return HDF5File(new HDF5SharedHandle(H5Fopen(name.c_str(),
                                               H5F_ACC_RDONLY, plist),
                                       &H5Fclose));
}

HDF5File::HDF5File(HDF5SharedHandle *h): HDF5Group(h){}

void HDF5File::flush() {
  IMP_HDF5_CALL(H5Fflush(get_handle(), H5F_SCOPE_LOCAL));
}

HDF5File::~HDF5File() {
}


} /* namespace rmf */
