/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/HDF5Group.h>
#include <RMF/HDF5File.h>
#include <H5Fpublic.h>
#include <H5public.h>
#include <boost/scoped_array.hpp>

namespace RMF {
namespace internal {
bool show_hdf5_errors = false;
}

HDF5Object::HDF5Object(HDF5SharedHandle *h): h_(h) {
}


HDF5File HDF5Object::get_file() const {
  RMF_HDF5_NEW_HANDLE(h, H5Iget_file_id(get_handle()), &H5Fclose);
  return HDF5File(h.get());
}

HDF5Group::HDF5Group(HDF5SharedHandle *h):
  P(h) {
}

HDF5ConstGroup::HDF5ConstGroup(HDF5SharedHandle *h):
  P(h) {
}


HDF5Group::HDF5Group(HDF5Group parent, std::string name):
  P(new HDF5SharedHandle(H5Gopen2(parent.get_handle(),
                                  name.c_str(),
                                  H5P_DEFAULT), &H5Gclose,
                         name)) {
}

HDF5ConstGroup::HDF5ConstGroup(HDF5ConstGroup parent, std::string name):
  P(new HDF5SharedHandle(H5Gopen2(parent.get_handle(),
                                  name.c_str(),
                                  H5P_DEFAULT), &H5Gclose,
                         name)) {
}

HDF5Group HDF5Group::add_child_group(std::string name) {
  RMF_USAGE_CHECK(!H5Lexists(get_handle(), name.c_str(), H5P_DEFAULT),
                  internal::get_error_message("Child named ",
                                              name, " already exists"));
  RMF_HDF5_HANDLE(, H5Gcreate2(get_handle(), name.c_str(),
                               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
                  &H5Gclose);
  return HDF5Group(*this, name);
}

unsigned int HDF5ConstGroup::get_number_of_children() const {
  unsigned int n = get_number_of_links();
  // later check that they are groups
  return n;
}
std::string HDF5ConstGroup::get_child_name(unsigned int i) const {
  int sz = H5Lget_name_by_idx(get_handle(), ".",
                              H5_INDEX_NAME, H5_ITER_NATIVE, (hsize_t)i,
                              NULL, 0, H5P_DEFAULT);
  boost::scoped_array<char> buf(new char[sz + 1]);
  RMF_HDF5_CALL(H5Lget_name_by_idx(get_handle(), ".",
                                   H5_INDEX_NAME, H5_ITER_NATIVE, (hsize_t)i,
                                   buf.get(), sz + 1, H5P_DEFAULT));
  return std::string(buf.get());
}
bool HDF5ConstGroup::get_has_child(std::string name) const {
  return H5Lexists(get_handle(), name.c_str(), H5P_DEFAULT);
}
bool HDF5ConstGroup::get_child_is_group(unsigned int i) const {
  H5O_info_t info;
  RMF_HDF5_HANDLE(c, H5Oopen(get_handle(), get_child_name(i).c_str(),
                             H5P_DEFAULT), &H5Oclose);
  RMF_HDF5_CALL(H5Oget_info(c, &info));
  return info.type == H5O_TYPE_GROUP; //H5O_TYPE_DATASET
}
HDF5ConstGroup HDF5ConstGroup::get_child_group(unsigned int i) const {
  return HDF5ConstGroup(new HDF5SharedHandle(H5Gopen2(get_handle(),
                                                      get_child_name(i).c_str(),
                                                      H5P_DEFAULT),
                                             &H5Gclose,
                                             "open group"));
}
HDF5Group HDF5Group::get_child_group(unsigned int i) const {
  return HDF5Group(new HDF5SharedHandle(H5Gopen2(get_handle(),
                                                      get_child_name(i).c_str(),
                                                 H5P_DEFAULT), &H5Gclose,
                                        "open group"));
}
bool HDF5ConstGroup::get_child_is_data_set(unsigned int i) const {
  H5O_info_t info;
  RMF_HDF5_HANDLE(c, H5Oopen(get_handle(), get_child_name(i).c_str(),
                             H5P_DEFAULT), &H5Oclose);
  RMF_HDF5_CALL(H5Oget_info(c, &info));
  return info.type == H5O_TYPE_DATASET; //H5O_TYPE_DATASET
}




namespace {
hid_t get_parameters() {
  hid_t plist = H5Pcreate(H5P_FILE_ACCESS);
  RMF_HDF5_CALL(H5Pset_sieve_buf_size(plist, 1000000));
  RMF_HDF5_CALL(H5Pset_cache(plist, 0, 10000, 10000000, 0.0));
#if defined(H5_VERS_MAJOR) && H5_VERS_MAJOR >= 1 \
  && H5_VERS_MINOR >= 8 && H5_VERS_RELEASE >= 6
  RMF_HDF5_CALL(H5Pset_libver_bounds(plist, H5F_LIBVER_18, H5F_LIBVER_LATEST));
#endif
  return plist;
}
herr_t error_function(hid_t, void *) {
  if (internal::show_hdf5_errors) {
    H5Eprint2(H5E_DEFAULT, stderr);
  }
  // eat hdf5 error as I check the error code explicitly
  return 0;
}
}

// throws RMF::IOException on error
HDF5File create_hdf5_file(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fcreate(name.c_str(),
                                   H5F_ACC_TRUNC, H5P_DEFAULT,
                                   plist), &H5Fclose);
  return HDF5File(h.get());
}

HDF5File open_hdf5_file(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fopen(name.c_str(),
                                 H5F_ACC_RDWR, plist),
                      &H5Fclose);
  return HDF5File(h.get());
}

HDF5ConstFile open_hdf5_file_read_only(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fopen(name.c_str(),
                                 H5F_ACC_RDONLY, plist),
                      &H5Fclose);
  return HDF5ConstFile(h.get());
}

HDF5File open_hdf5_file_read_only_returning_nonconst(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fopen(name.c_str(),
                                 H5F_ACC_RDONLY, plist),
                      &H5Fclose);
  return HDF5File(h.get());
}


HDF5File::HDF5File(HDF5SharedHandle *h): HDF5Group(h) {
}
HDF5ConstFile::HDF5ConstFile(HDF5SharedHandle *h): HDF5ConstGroup(h) {
}
HDF5ConstFile::HDF5ConstFile(HDF5File h):
  HDF5ConstGroup(h.get_shared_handle()) {
}

void HDF5File::flush() {
  RMF_HDF5_CALL(H5Fflush(get_handle(), H5F_SCOPE_LOCAL));
}

/*bool HDF5File::get_is_writable() const {
   unsigned int intent;
   RMF_HDF5_CALL(H5Fget_intent(get_handle(), &intent));
   return intent==H5F_ACC_RDWR;
   }*/

std::string HDF5ConstFile::get_name() const {
  int sz = H5Fget_name(get_handle(), NULL, 0);
  boost::scoped_array<char> buf(new char[sz + 1]);
  RMF_HDF5_CALL(H5Fget_name(get_handle(), buf.get(), sz + 1));
  return std::string(buf.get());
}
std::string HDF5File::get_name() const {
  int sz = H5Fget_name(get_handle(), NULL, 0);
  boost::scoped_array<char> buf(new char[sz + 1]);
  RMF_HDF5_CALL(H5Fget_name(get_handle(), buf.get(), sz + 1));
  return std::string(buf.get());
}

HDF5File::~HDF5File() {
}
HDF5ConstFile::~HDF5ConstFile() {
}



int get_number_of_open_hdf5_handles(HDF5ConstFile f) {
  H5garbage_collect();
  if (f == HDF5ConstFile()) {
    return H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
  } else {
    return H5Fget_obj_count(f.get_handle(), H5F_OBJ_ALL);
  }
}
Strings get_open_hdf5_handle_names(HDF5ConstFile f) {
  Strings ret;
  int n = get_number_of_open_hdf5_handles(f);
  hid_t ref;
  if (f == HDF5ConstFile()) {
    ref = H5F_OBJ_ALL;
  } else {
    ref = f.get_handle();
  }
  boost::scoped_array<hid_t> arr(new hid_t[n]);
  int num = H5Fget_obj_ids(ref, H5F_OBJ_ALL, n, arr.get());
  boost::scoped_array<char> buf(new char[10000]);
  for (int i = 0; i < num; ++i) {
    int len = H5Iget_name(arr[i], buf.get(), 10000);
    if (len > 0) {
      std::ostringstream oss;
      std::string name(buf.get());
      int flen = H5Fget_name(arr[i], buf.get(), 10000);
      if (flen > 0) {
        oss << buf.get() << name;
      }
      ret.push_back(oss.str());
    }
  }
  return ret;
}
} /* namespace RMF */
