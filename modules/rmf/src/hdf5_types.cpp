/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/hdf5_types.h>
#include <boost/scoped_array.hpp>
#include <IMP/rmf/HDF5DataSetD.h>

namespace rmf {

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
                                         const Floats& v) {
    IMP_HDF5_CALL(H5Dwrite(d,
                           H5T_NATIVE_DOUBLE, is, s,
                           H5P_DEFAULT, &v[0]));
  }
  Floats FloatTraits::read_values_dataset(hid_t d, hid_t is,
                                          hid_t sp,
                                          unsigned int sz) {
    Floats ret(sz);
    IMP_HDF5_CALL(H5Dread(d,
                          H5T_NATIVE_DOUBLE,
                          is, sp, H5P_DEFAULT, &ret[0]));
    return ret;
  }

  Floats
  FloatTraits::read_values_attribute(hid_t a, unsigned int size) {
    Floats v(size);
    IMP_HDF5_CALL(H5Aread(a, H5T_NATIVE_DOUBLE, &v[0]));
    return v;
  }
  void FloatTraits::write_values_attribute(hid_t a,
                                           const Floats &v) {
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
                                       const Ints& v) {
    IMP_HDF5_CALL(H5Dwrite(d,
                           H5T_NATIVE_INT, is, s,
                           H5P_DEFAULT, &v[0]));
  }
  Ints IntTraits::read_values_dataset(hid_t d, hid_t is,
                                      hid_t sp,
                                      unsigned int sz) {
    Ints ret(sz);
    IMP_HDF5_CALL(H5Dread(d,
                          H5T_NATIVE_INT,
                          is, sp, H5P_DEFAULT, &ret[0]));
    return ret;
  }
  Ints IntTraits::read_values_attribute(hid_t a,
                                        unsigned int size) {
    Ints v(size);
    IMP_HDF5_CALL(H5Aread(a, H5T_NATIVE_INT, &v[0]));
    return v;
  }
  void IntTraits::write_values_attribute(hid_t a, const Ints &v){
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
    const Ints& v) {
    IMP_HDF5_CALL(H5Dwrite(d,
    get_hdf5_type(), is, s,
    H5P_DEFAULT, &v[0]));
    }
    Ints CharTraits::read_values_dataset(hid_t d, hid_t is,
    hid_t sp,
    unsigned int sz) {
    Ints ret(sz);
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
                                          const Strings & ) {
    IMP_RMF_UNUSED(d);
    IMP_RMF_UNUSED(is);
    IMP_RMF_UNUSED(s);
    //IMP_UNUSED(v);
    IMP_RMF_NOT_IMPLEMENTED;
  }
  Strings StringTraits::read_values_dataset(hid_t d, hid_t is,
                                            hid_t sp,
                                            unsigned int sz) {
    IMP_RMF_UNUSED(d);
    IMP_RMF_UNUSED(is);
    IMP_RMF_UNUSED(sp);
    IMP_RMF_UNUSED(sz);
    IMP_RMF_NOT_IMPLEMENTED;
    return Strings();
  }

  void StringTraits::write_values_attribute(hid_t d,
                                            const Strings &) {
    IMP_RMF_UNUSED(d);
    IMP_RMF_NOT_IMPLEMENTED;
  }
  Strings StringTraits::read_values_attribute(hid_t d,
                                              unsigned int num) {
    IMP_RMF_UNUSED(d);
    IMP_RMF_UNUSED(num);
    IMP_RMF_NOT_IMPLEMENTED;
    return Strings();
  }

  std::string StringTraits::get_name() {
    return std::string("string");
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
                                          const NodeIDs& v) {
    Ints vi(v.size());
    for (unsigned int i=0; i< v.size(); ++i) {
      vi[i]= v[i].get_index();
    }
    IntTraits::write_values_dataset(d, is, s, vi);
  }
  NodeIDs NodeIDTraits::read_values_dataset(hid_t d, hid_t is,
                                            hid_t sp,
                                            unsigned int sz) {
    Ints reti= IntTraits::read_values_dataset(d, is, sp, sz);
    NodeIDs ret(reti.size());
    for (unsigned int i=0; i< ret.size(); ++i) {
      ret[i]= NodeID(reti[i]);
    }
    return ret;
  }
  void NodeIDTraits::write_values_attribute(hid_t d,
                                            const NodeIDs &values) {
    Ints is(values.size());
    for (unsigned int i=0; i< values.size(); ++i) {
      is[i]=values[i].get_index();
    }
    IndexTraits::write_values_attribute(d, is);
  }
  NodeIDs NodeIDTraits::read_values_attribute(hid_t d,
                                              unsigned int num) {
    Ints is= IndexTraits::read_values_attribute(d, num);
    NodeIDs ret(is.size());
    for (unsigned int i=0; i< ret.size(); ++i) {
      ret[i]=NodeID(is[i]);
    }
    return ret;
  }
  std::string NodeIDTraits::get_name() {
    return std::string("node id");
  }

} /* namespace rmf */
