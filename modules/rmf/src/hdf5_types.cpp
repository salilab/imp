/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/hdf5_types.h>
#include <boost/scoped_array.hpp>
#include <IMP/base_types.h>

IMPRMF_BEGIN_NAMESPACE

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
                        H5T_NATIVE_DOUBLE,
                        is, sp, H5P_DEFAULT, &ret));
  return ret;
}
void FloatTraits::write_values_dataset(hid_t d, hid_t is,
                                       hid_t s,
                                       const std::vector<double>& v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         H5T_NATIVE_DOUBLE, is, s,
                         H5P_DEFAULT, &v[0]));
}
std::vector<double> FloatTraits::read_values_dataset(hid_t d, hid_t is,
                                                    hid_t sp,
                                                    unsigned int sz) {
  std::vector<double> ret(sz);
  IMP_HDF5_CALL(H5Dread(d,
                        H5T_NATIVE_DOUBLE,
                        is, sp, H5P_DEFAULT, &ret[0]));
  return ret;
}

std::vector<double>
FloatTraits::read_values_attribute(hid_t a, unsigned int size) {
  std::vector<double> v(size);
  IMP_HDF5_CALL(H5Aread(a, H5T_NATIVE_DOUBLE, &v[0]));
  return v;
}
void FloatTraits::write_values_attribute(hid_t a,
                                         const std::vector<double> &v) {
  IMP_HDF5_CALL(H5Awrite(a, H5T_NATIVE_DOUBLE, &v[0]));
}
std::string FloatTraits::get_name() {
  return std::string("float");
}


void IntTraits::write_value_dataset(hid_t d, hid_t is,
                                      hid_t s,
                                      int v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         H5T_NATIVE_INT, is, s,
                         H5P_DEFAULT, &v));
}
int IntTraits::read_value_dataset(hid_t d, hid_t is,
                                    hid_t sp) {
  int ret;
  IMP_HDF5_CALL(H5Dread(d,
                        H5T_NATIVE_INT, is, sp, H5P_DEFAULT, &ret));
  return ret;
}
void IntTraits::write_values_dataset(hid_t d, hid_t is,
                                       hid_t s,
                                       const std::vector<int>& v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         H5T_NATIVE_INT, is, s,
                         H5P_DEFAULT, &v[0]));
}
std::vector<int> IntTraits::read_values_dataset(hid_t d, hid_t is,
                                                hid_t sp,
                                                unsigned int sz) {
  std::vector<int> ret(sz);
  IMP_HDF5_CALL(H5Dread(d,
                        H5T_NATIVE_INT,
                        is, sp, H5P_DEFAULT, &ret[0]));
  return ret;
}
std::vector<int> IntTraits::read_values_attribute(hid_t a,
                                                  unsigned int size) {
  std::vector<int> v(size);
  IMP_HDF5_CALL(H5Aread(a, H5T_NATIVE_INT, &v[0]));
  return v;
}
void IntTraits::write_values_attribute(hid_t a, const std::vector<int> &v){
  IMP_HDF5_CALL(H5Awrite(a, H5T_NATIVE_INT, &v[0]));
}
std::string IntTraits::get_name() {
  return std::string("int");
}


/*
void CharTraits::write_value_dataset(hid_t d, hid_t is,
                                      hid_t s,
                                      int v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         get_hdf5_type(), is, s,
                         H5P_DEFAULT, &v));
}
int CharTraits::read_value_dataset(hid_t d, hid_t is,
                                    hid_t sp) {
  int ret;
  IMP_HDF5_CALL(H5Dread(d,
                        get_hdf5_type(), is, sp, H5P_DEFAULT, &ret));
  return ret;
}
void CharTraits::write_values_dataset(hid_t d, hid_t is,
                                       hid_t s,
                                       const std::vector<int>& v) {
  IMP_HDF5_CALL(H5Dwrite(d,
                         get_hdf5_type(), is, s,
                         H5P_DEFAULT, &v[0]));
}
std::vector<int> CharTraits::read_values_dataset(hid_t d, hid_t is,
                                                hid_t sp,
                                                unsigned int sz) {
  std::vector<int> ret(sz);
  IMP_HDF5_CALL(H5Dread(d,
                        get_hdf5_type(),
                        is, sp, H5P_DEFAULT, &ret[0]));
  return ret;
  }*/
std::string CharTraits::read_values_attribute(hid_t a,
                                              unsigned int size) {
  std::vector<char> v(size);
  IMP_HDF5_CALL(H5Aread(a, H5T_NATIVE_CHAR, &v[0]));
  return std::string(v.begin(), v.end());
}
void CharTraits::write_values_attribute(hid_t a, std::string v){
  IMP_HDF5_CALL(H5Awrite(a, H5T_NATIVE_CHAR, v.c_str()));
}
std::string CharTraits::get_name() {
  return std::string("char");
}





std::string IndexTraits::get_name() {
  return std::string("index");
}



void StringTraits::write_value_dataset(hid_t d, hid_t is,
                                         hid_t s, std::string v) {
  /*hvl_t data;
    data.len=v.size();
    data.p= const_cast<char*>(&v[0]);
    IMP_HDF5_CALL(H5Dwrite(d,
                           get_hdf5_type(), is, s,
                           H5P_DEFAULT, &data));*/
  char *c;
  if (!v.empty()) {
    c= const_cast<char*>(v.c_str());
  } else {
    static char empty='\0';
    c=&empty;
  }
  IMP_HDF5_CALL(H5Dwrite(d, get_hdf5_memory_type(), is, s,
                         H5P_DEFAULT, &c));
 }
std::string StringTraits::read_value_dataset(hid_t d,
                                               hid_t is,
                                               hid_t sp) {
  /*hvl_t data;
    H5Dread (d, get_hdf5_type(), is, sp, H5P_DEFAULT, &data);
    Type ret(static_cast<char*>(data.p), data.len);
    H5Dvlen_reclaim(get_hdf5_type(), is, H5P_DEFAULT, &data);
    return ret;*/
  char *c=NULL;
  HDF5Handle mt(create_string_type(),
                H5Tclose);
  IMP_HDF5_CALL(H5Dread(d,  mt, is, sp, H5P_DEFAULT, &c));
  std::string ret;
  if (c) {
    ret=std::string(c);
  }
  //std::cout << "read " << ret << std::endl;
  //HDF5Handle space(H5Dget_space (d), H5Sclose);
  //IMP_HDF5_CALL(H5Dvlen_reclaim (get_hdf5_type(), space, H5P_DEFAULT, &c));
  /*IMP_INTERNAL_CHECK(c==NULL,
    "Non-check was not stomped on " << c);*/
  free(c);
  return ret;
}
void StringTraits::write_values_dataset(hid_t d, hid_t is,
                                       hid_t s,
                                        const std::vector<std::string>& ) {
  IMP_UNUSED(d);
  IMP_UNUSED(is);
  IMP_UNUSED(s);
  //IMP_UNUSED(v);
  IMP_NOT_IMPLEMENTED;
}
std::vector<std::string> StringTraits::read_values_dataset(hid_t d, hid_t is,
                                                           hid_t sp,
                                                           unsigned int sz) {
  IMP_UNUSED(d);
  IMP_UNUSED(is);
  IMP_UNUSED(sp);
  IMP_UNUSED(sz);
  IMP_NOT_IMPLEMENTED;
}

void StringTraits::write_values_attribute(hid_t d,
                                const std::vector<std::string> &) {
  IMP_UNUSED(d);
  IMP_NOT_IMPLEMENTED;
}
std::vector<std::string> StringTraits::read_values_attribute(hid_t d,
                                                             unsigned int num) {
  IMP_UNUSED(d);
  IMP_UNUSED(num);
  IMP_NOT_IMPLEMENTED;
}

std::string StringTraits::get_name() {
  return std::string("string");
}


std::string DataSetTraits::get_name() {
  return std::string("dataset");
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
void NodeIDTraits::write_values_dataset(hid_t d, hid_t is,
                                       hid_t s,
                                       const std::vector<NodeID>& v) {
  std::vector<int> vi(v.size());
  for (unsigned int i=0; i< v.size(); ++i) {
    vi[i]= v[i].get_index();
  }
  IntTraits::write_values_dataset(d, is, s, vi);
}
std::vector<NodeID> NodeIDTraits::read_values_dataset(hid_t d, hid_t is,
                                                hid_t sp,
                                                unsigned int sz) {
  std::vector<int> reti= IntTraits::read_values_dataset(d, is, sp, sz);
  std::vector<NodeID> ret(reti.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= NodeID(reti[i]);
  }
  return ret;
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
std::string NodeIDTraits::get_name() {
  return std::string("node id");
}



IMPRMF_END_NAMESPACE
