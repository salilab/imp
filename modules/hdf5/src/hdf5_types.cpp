/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/hdf5_types.h>
#include <boost/scoped_array.hpp>
#include <IMP/base_types.h>

IMPHDF5_BEGIN_NAMESPACE

hid_t FloatTraits::get_hdf5_type() {
  return H5T_NATIVE_DOUBLE;
}
void FloatTraits::write_value_dataset(hid_t d, hid_t is,
                                        hid_t s,
                                        double v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         H5T_NATIVE_DOUBLE, is, s,
                         H5P_DEFAULT, &v));
}
double FloatTraits::read_value_dataset(hid_t d, hid_t is,
                                        hid_t sp) {
  double ret;
  IMP_HDF5_CALL(H5Dread(d,
                        get_hdf5_type(),
                        is, sp, H5P_DEFAULT, &ret));
  return ret;
}
std::vector<double>
FloatTraits::read_values_attribute(hid_t a, unsigned int size) {
  std::vector<double> v(size);
  IMP_HDF5_CALL(H5Aread(a, get_hdf5_type(), &v[0]));
  return v;
}
void FloatTraits::write_values_attribute(hid_t a,
                                         const std::vector<double> &v) {
  IMP_HDF5_CALL(H5Awrite(a, get_hdf5_type(), &v[0]));
}
const double& FloatTraits::get_null_value() {
  static const double ret= std::numeric_limits<double>::max();
  return ret;
}
const double& FloatTraits::get_fill_value() {
  return get_null_value();
}
std::string FloatTraits::get_name() {
  return std::string("float");
}



hid_t IntTraits::get_hdf5_type() {
  return H5T_NATIVE_INT;
}
void IntTraits::write_value_dataset(hid_t d, hid_t is,
                                      hid_t s,
                                      int v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         get_hdf5_type(), is, s,
                         H5P_DEFAULT, &v));
}
int IntTraits::read_value_dataset(hid_t d, hid_t is,
                                    hid_t sp) {
  int ret;
  IMP_HDF5_CALL(H5Dread(d,
                        get_hdf5_type(), is, sp, H5P_DEFAULT, &ret));
  return ret;
}
std::vector<int> IntTraits::read_values_attribute(hid_t a,
                                                  unsigned int size) {
  std::vector<int> v(size);
  IMP_HDF5_CALL(H5Aread(a, get_hdf5_type(), &v[0]));
  return v;
}
void IntTraits::write_values_attribute(hid_t a, const std::vector<int> &v){
  IMP_HDF5_CALL(H5Awrite(a, get_hdf5_type(), &v[0]));
}
const int& IntTraits::get_null_value() {
  static const int ret= std::numeric_limits<int>::max();
  return ret;
}
const int& IntTraits::get_fill_value() {
  return get_null_value();
}
std::string IntTraits::get_name() {
  return std::string("int");
}


const int& IndexTraits::get_null_value() {
  static const int ret=-1;
  return ret;
}
const int& IndexTraits::get_fill_value() {
  return get_null_value();
}
std::string IndexTraits::get_name() {
  return std::string("index");
}



namespace {
  const size_t max_length=40;
}
hid_t StringTraits::get_hdf5_type() {
  hid_t r= H5Tcopy(H5T_C_S1);
  IMP_HDF5_CALL(H5Tset_size(r, max_length));
  IMP_HDF5_CALL(H5Tset_strpad(r, H5T_STR_NULLTERM));
  return r;
}
void StringTraits::write_value_dataset(hid_t d, hid_t is,
                                         hid_t s, std::string str) {
  char buf[max_length]={'\0'};
  std::copy(str.begin(), str.begin()+std::min(max_length-1, str.size()),
            buf);
  IMP_HDF5_CALL(H5Dwrite(d,
                         get_hdf5_type(), is, s,
                         H5P_DEFAULT, &buf));
}
std::string StringTraits::read_value_dataset(hid_t d,
                                               hid_t is,
                                               hid_t sp) {
  char buf[max_length]={'\0'};
  IMP_HDF5_CALL(H5Dread(d,
                        get_hdf5_type(), is, sp, H5P_DEFAULT, &buf));
  return std::string(buf);
}

void StringTraits::write_values_attribute(hid_t d,
                                const std::vector<std::string> &values) {
  boost::scoped_array<char> buf(new char[values.size()*max_length]);
  std::fill(buf.get(), buf.get()+values.size()*max_length, '\0');
  for (unsigned int i=0; i< values.size(); ++i) {
    std::copy(values[i].begin(), values[i].begin()+std::min(max_length-1,
                                                            values[i].size()),
              &buf[i*max_length]);
  }
  char *ptr= buf.get();
  IMP_HDF5_CALL(H5Awrite(d,
                         get_hdf5_type(), ptr));
}
std::vector<std::string> StringTraits::read_values_attribute(hid_t d,
                                                             unsigned int num) {
  // memory leak...
  boost::scoped_array<char> buf(new char[num*max_length]);
  std::fill(buf.get(), buf.get()+num*max_length, '\0');
  char *ptr= buf.get();
  IMP_HDF5_CALL(H5Aread(d,
                        get_hdf5_type(), ptr));
  std::vector<std::string> ret(num);
  for (unsigned int i=0; i< num; ++i) {
    ret[i]= std::string(&buf[i*max_length]);
  }
  return ret;
}
const char*& StringTraits::get_null_value() {
  static const char buf[max_length]={'\0'};
  static const char *ptr=buf;
  return ptr;
}
const char& StringTraits::get_fill_value() {
  return *get_null_value();
}

std::string StringTraits::get_name() {
  return std::string("string");
}





hid_t NodeIDTraits::get_hdf5_type() {
  return IndexTraits::get_hdf5_type();
}
void NodeIDTraits::write_value_dataset(hid_t d, hid_t is,
                                         hid_t s, NodeID str) {
  return IndexTraits::write_value_dataset(d, is, s, str.get_index());
}
NodeID NodeIDTraits::read_value_dataset(hid_t d,
                                        hid_t is,
                                        hid_t sp) {
  int i= IndexTraits::read_value_dataset(d, is, sp);
  if (i>=0) return NodeID(i);
  else return NodeID();
}

void NodeIDTraits::write_values_attribute(hid_t d,
                                const std::vector<NodeID> &values) {
  Ints is(values.size());
  for (unsigned int i=0; i< values.size(); ++i) {
    is[i]=values[i].get_index();
  }
  IndexTraits::write_values_attribute(d, is);
}
std::vector<NodeID> NodeIDTraits::read_values_attribute(hid_t d,
                                                             unsigned int num) {
  Ints is= IndexTraits::read_values_attribute(d, num);
  std::vector<NodeID> ret(is.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]=NodeID(is[i]);
  }
  return ret;
}
const NodeID& NodeIDTraits::get_null_value() {
  static NodeID n;
  return n;
}
const NodeID& NodeIDTraits::get_fill_value() {
  return get_null_value();
}

std::string NodeIDTraits::get_name() {
  return std::string("node id");
}



IMPHDF5_END_NAMESPACE
