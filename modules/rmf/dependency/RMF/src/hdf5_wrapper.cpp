/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <H5Dpublic.h>
#include <H5Epublic.h>
#include <H5Fpublic.h>
#include <H5Gpublic.h>
#include <H5Ipublic.h>
#include <H5Lpublic.h>
#include <H5Opublic.h>
#include <H5Ppublic.h>
#include <H5Tpublic.h>
#include <H5public.h>
#include <boost/make_shared.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <ostream>
#include <string>

#include "RMF/HDF5/ConstFile.h"
#include "RMF/HDF5/ConstGroup.h"
#include "RMF/HDF5/File.h"
#include "RMF/HDF5/Group.h"
#include "RMF/HDF5/Object.h"
#include "RMF/HDF5/handle.h"
#include "RMF/HDF5/infrastructure_macros.h"
#include "RMF/HDF5/types.h"
#include "RMF/compiler_macros.h"
#include "RMF/exceptions.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {
namespace {
bool show_errors = false;
}

void set_show_errors(bool tf) { show_errors = tf; }

Object::Object(boost::shared_ptr<SharedHandle> h) : h_(h) {}

File Object::get_file() const {
  RMF_HDF5_NEW_HANDLE(h, H5Iget_file_id(get_handle()), &H5Fclose);
  return File(h);
}

Group::Group(boost::shared_ptr<SharedHandle> h) : P(h) {}

ConstGroup::ConstGroup(boost::shared_ptr<SharedHandle> h) : P(h) {}

Group::Group(Group parent, std::string name)
    : P(boost::make_shared<SharedHandle>(H5Gopen2(parent.get_handle(),
                                                  name.c_str(), H5P_DEFAULT),
                                         &H5Gclose, name)) {}

ConstGroup::ConstGroup(ConstGroup parent, std::string name)
    : P(boost::make_shared<SharedHandle>(H5Gopen2(parent.get_handle(),
                                                  name.c_str(), H5P_DEFAULT),
                                         &H5Gclose, name)) {}

Group Group::add_child_group(std::string name) {
  RMF_USAGE_CHECK(!H5Lexists(get_handle(), name.c_str(), H5P_DEFAULT),
                  RMF::internal::get_error_message("Child named ", name,
                                                   " already exists"));
  RMF_HDF5_HANDLE(, H5Gcreate2(get_handle(), name.c_str(), H5P_DEFAULT,
                               H5P_DEFAULT, H5P_DEFAULT),
                  &H5Gclose);
  return Group(*this, name);
}

unsigned int ConstGroup::get_number_of_children() const {
  unsigned int n = get_number_of_links();
  // later check that they are groups
  return n;
}
std::string ConstGroup::get_child_name(unsigned int i) const {
  int sz = H5Lget_name_by_idx(get_handle(), ".", H5_INDEX_NAME, H5_ITER_NATIVE,
                              (hsize_t)i, NULL, 0, H5P_DEFAULT);
  boost::scoped_array<char> buf(new char[sz + 1]);
  RMF_HDF5_CALL(H5Lget_name_by_idx(get_handle(), ".", H5_INDEX_NAME,
                                   H5_ITER_NATIVE, (hsize_t)i, buf.get(),
                                   sz + 1, H5P_DEFAULT));
  return std::string(buf.get());
}
bool ConstGroup::get_has_child(std::string name) const {
  return H5Lexists(get_handle(), name.c_str(), H5P_DEFAULT);
}
bool ConstGroup::get_child_is_group(unsigned int i) const {
  H5O_info_t info;
  RMF_HDF5_HANDLE(c,
                  H5Oopen(get_handle(), get_child_name(i).c_str(), H5P_DEFAULT),
                  &H5Oclose);
  RMF_HDF5_CALL(H5Oget_info(c, &info));
  return info.type == H5O_TYPE_GROUP;  // H5O_TYPE_DATASET
}
ConstGroup ConstGroup::get_child_group(unsigned int i) const {
  return ConstGroup(boost::make_shared<SharedHandle>(
      H5Gopen2(get_handle(), get_child_name(i).c_str(), H5P_DEFAULT), &H5Gclose,
      "open group"));
}
Group Group::get_child_group(unsigned int i) const {
  return Group(boost::make_shared<SharedHandle>(
      H5Gopen2(get_handle(), get_child_name(i).c_str(), H5P_DEFAULT), &H5Gclose,
      "open group"));
}
bool ConstGroup::get_child_is_data_set(unsigned int i) const {
  H5O_info_t info;
  RMF_HDF5_HANDLE(c,
                  H5Oopen(get_handle(), get_child_name(i).c_str(), H5P_DEFAULT),
                  &H5Oclose);
  RMF_HDF5_CALL(H5Oget_info(c, &info));
  return info.type == H5O_TYPE_DATASET;  // H5O_TYPE_DATASET
}

namespace {
hid_t get_parameters() {
  hid_t plist = H5Pcreate(H5P_FILE_ACCESS);
  RMF_HDF5_CALL(H5Pset_sieve_buf_size(plist, 1000000));
  RMF_HDF5_CALL(H5Pset_cache(plist, 0, 10000, 10000000, 0.0));
#if defined(H5_VERS_MAJOR) && H5_VERS_MAJOR >= 1 && H5_VERS_MINOR >= 8 && \
    H5_VERS_RELEASE >= 6
  RMF_HDF5_CALL(H5Pset_libver_bounds(plist, H5F_LIBVER_18, H5F_LIBVER_LATEST));
#endif
  return plist;
}
herr_t error_function(hid_t, void*) {
  if (show_errors) {
    H5Eprint2(H5E_DEFAULT, stderr);
  }
  // eat hdf5 error as I check the error code explicitly
  return 0;
}
}

// throws RMF::IOException on error
File create_file(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(
      h, H5Fcreate(name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, plist), &H5Fclose);
  return File(h);
}

File open_file(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fopen(name.c_str(), H5F_ACC_RDWR, plist), &H5Fclose);
  return File(h);
}

ConstFile open_file_read_only(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fopen(name.c_str(), H5F_ACC_RDONLY, plist),
                      &H5Fclose);
  return ConstFile(h);
}

File open_file_read_only_returning_nonconst(std::string name) {
  RMF_HDF5_CALL(H5open());
  RMF_HDF5_CALL(H5Eset_auto2(H5E_DEFAULT, &error_function, NULL));
  RMF_HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  RMF_HDF5_NEW_HANDLE(h, H5Fopen(name.c_str(), H5F_ACC_RDONLY, plist),
                      &H5Fclose);
  return File(h);
}

File::File(boost::shared_ptr<SharedHandle> h) : Group(h) {}
ConstFile::ConstFile(boost::shared_ptr<SharedHandle> h) : ConstGroup(h) {}
ConstFile::ConstFile(File h) : ConstGroup(h.get_shared_handle()) {}

void File::flush() { RMF_HDF5_CALL(H5Fflush(get_handle(), H5F_SCOPE_LOCAL)); }

/*bool File::get_is_writable() const {
   unsigned int intent;
   RMF_HDF5_CALL(H5Fget_intent(get_handle(), &intent));
   return intent==H5F_ACC_RDWR;
   }*/

std::string ConstFile::get_name() const {
  int sz = H5Fget_name(get_handle(), NULL, 0);
  boost::scoped_array<char> buf(new char[sz + 1]);
  RMF_HDF5_CALL(H5Fget_name(get_handle(), buf.get(), sz + 1));
  return std::string(buf.get());
}
std::string File::get_name() const {
  int sz = H5Fget_name(get_handle(), NULL, 0);
  boost::scoped_array<char> buf(new char[sz + 1]);
  RMF_HDF5_CALL(H5Fget_name(get_handle(), buf.get(), sz + 1));
  return std::string(buf.get());
}

File::~File() {}
ConstFile::~ConstFile() {}

int get_number_of_open_handles(ConstFile f) {
  H5garbage_collect();
  if (f == ConstFile()) {
    return H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
  } else {
    return H5Fget_obj_count(f.get_handle(), H5F_OBJ_ALL);
  }
}
Strings get_open_handle_names(ConstFile f) {
  Strings ret;
  int n = get_number_of_open_handles(f);
  hid_t ref;
  if (f == ConstFile()) {
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

void StringsTraits::write_value_dataset(hid_t d, hid_t iss, hid_t s,
                                        const StringsTraits::Type& v) {
  boost::scoped_array<char*> buf;

  hvl_t data;
  data.len = v.size();
  if (data.len > 0) {
    buf.reset(new char* [v.size()]);
    for (unsigned int i = 0; i < v.size(); ++i) {
      buf[i] = new char[v[i].size() + 1];
      std::copy(v[i].begin(), v[i].end(), buf[i]);
      buf[i][v[i].size()] = '\0';
    }
    data.p = buf.get();
  } else {
    data.p = NULL;
  }
  RMF_HDF5_CALL(
      H5Dwrite(d, get_hdf5_memory_type(), iss, s, H5P_DEFAULT, &data));
  for (unsigned int i = 0; i < v.size(); ++i) {
    delete[] buf[i];
  }
}

StringsTraits::Type StringsTraits::read_value_dataset(hid_t d, hid_t iss,
                                                      hid_t sp) {
  hvl_t data;
  H5Dread(d, get_hdf5_memory_type(), iss, sp, H5P_DEFAULT, &data);
  Type ret(data.len);
  char** cp = static_cast<char**>(data.p);
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = cp[i];
    free(cp[i]);
  }

  free(data.p);
  return ret;
}

const hvl_t& StringsTraits::get_fill_value() {
  static hvl_t ret = {0, NULL};
  return ret;
}

hid_t StringsTraits::get_hdf5_fill_type() {
  static RMF_HDF5_HANDLE(
      ints_type, H5Tvlen_create(StringTraits::get_hdf5_disk_type()), H5Tclose);
  return ints_type;
}
hid_t StringsTraits::get_hdf5_disk_type() {
  static RMF_HDF5_HANDLE(
      ints_type, H5Tvlen_create(StringTraits::get_hdf5_disk_type()), H5Tclose);
  return ints_type;
}
hid_t StringsTraits::get_hdf5_memory_type() {
  static RMF_HDF5_HANDLE(ints_type,
                         H5Tvlen_create(StringTraits::get_hdf5_memory_type()),
                         H5Tclose);
  return ints_type;
}

void StringTraits::write_value_dataset(hid_t d, hid_t iss, hid_t s,
                                       const StringTraits::Type& v) {
  static char empty = '\0';
  char* c;
  if (!v.empty()) {
    c = new char[v.size() + 1];
    std::copy(v.begin(), v.end(), c);
    c[v.size()] = '\0';
  } else {
    c = &empty;
  }
  RMF_HDF5_CALL(H5Dwrite(d, get_hdf5_memory_type(), iss, s, H5P_DEFAULT, &c));
  if (!v.empty()) delete[] c;
}

StringTraits::Type StringTraits::read_value_dataset(hid_t d, hid_t iss,
                                                    hid_t sp) {
  char* c = NULL;
  RMF_HDF5_HANDLE(mt, internal::create_string_type(), H5Tclose);
  RMF_HDF5_CALL(H5Dread(d, mt, iss, sp, H5P_DEFAULT, &c));
  std::string ret;
  if (c) {
    ret = std::string(c);
  }
  free(c);
  return ret;
}

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS
