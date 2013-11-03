/**
 * \file soap_hdf5.h
 * \brief Handling of SOAP HDF5 files.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SOAP_HDF_5_H
#define IMPSCORE_FUNCTOR_SOAP_HDF_5_H

#include <IMP/score_functor/score_functor_config.h>
#include <boost/noncopyable.hpp>
#include <hdf5.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

// Convert the HDF5 error into an IMP exception
IMPSCOREFUNCTOREXPORT
void handle_hdf5_error();

#define IMP_HDF5_OBJECT(Name, Close) \
    ~Name() { \
      if (id_ >= 0 && Close(id_) < 0) { \
        handle_hdf5_error(); \
      } \
    } \
    hid_t get() { return id_; }

#define IMP_HDF5_ERRCHECK(block) \
    if ((block) < 0) { handle_hdf5_error(); }

class Hdf5File : public boost::noncopyable {
  hid_t id_;
public:
  Hdf5File(std::string filename) : id_(-1) {
    IMP_HDF5_ERRCHECK(id_ = H5Fopen(filename.c_str(), H5F_ACC_RDONLY,
                                    H5P_DEFAULT));
  }
  IMP_HDF5_OBJECT(Hdf5File, H5Fclose)
};

class Hdf5Dataset;
class Hdf5Attribute;

class Hdf5Dataspace : public boost::noncopyable {
  hid_t id_;
public:
  // Get the dataspace for a given dataset
  Hdf5Dataspace(Hdf5Dataset &dataset);

  // Get the dataspace for a given attribute
  Hdf5Dataspace(Hdf5Attribute &attribute);

  Ints get_simple_extent_dims() {
    int ndims;
    IMP_HDF5_ERRCHECK(ndims = H5Sget_simple_extent_ndims(id_));
    std::vector<hsize_t> ret(ndims);
    IMP_HDF5_ERRCHECK(H5Sget_simple_extent_dims(id_, &ret[0], NULL));
    return Ints(ret);
  }

  // Get the total number of points in the dataspace
  int get_simple_extent_npoints() {
    int npoints;
    IMP_HDF5_ERRCHECK(npoints = H5Sget_simple_extent_npoints(id_));
    return npoints;
  }

  IMP_HDF5_OBJECT(Hdf5Dataspace, H5Sclose)
};

// Variable-length (NULL-terminated, C-style) string datatype
class Hdf5StringDatatype : public boost::noncopyable {
  hid_t id_;
public:
  Hdf5StringDatatype() : id_(-1) {
    IMP_HDF5_ERRCHECK(id_ = H5Tcopy(H5T_C_S1));
    IMP_HDF5_ERRCHECK(H5Tset_size(id_, H5T_VARIABLE));
  }

  IMP_HDF5_OBJECT(Hdf5StringDatatype, H5Tclose)
};

class Hdf5Attribute : public boost::noncopyable {
  hid_t id_;
public:
  Hdf5Attribute(hid_t loc_id, std::string name) : id_(-1) {
    IMP_HDF5_ERRCHECK(id_ = H5Aopen(loc_id, name.c_str(), H5P_DEFAULT));
  }

  void read_float(float *data) {
    IMP_HDF5_ERRCHECK(H5Aread(id_, H5T_NATIVE_FLOAT, data));
  }

  IMP_HDF5_OBJECT(Hdf5Attribute, H5Aclose)
};

class Hdf5Dataset : public boost::noncopyable {
  hid_t id_;
  // Make sure all dimensions match expected values
  void check_dims(Ints dims) {
    Hdf5Dataspace space(*this);
    Ints actual_dims = space.get_simple_extent_dims();
    check_rank(dims.size(), actual_dims.size());
    check_each_dim(dims, actual_dims);
  }
  void check_rank(unsigned rank, unsigned actual_rank) {
    if (actual_rank != rank) {
      IMP_THROW("Dataset should be " << rank
                << "-dimensional, but is " << actual_rank,
                ValueException);
    }
  }
  void check_each_dim(Ints dims, Ints actual_dims) {
    for (unsigned i = 0; i < dims.size(); ++i) {
      if (dims[i] != actual_dims[i]) {
        IMP_THROW("Dataset dimension " << i << " is incorrect; "
                  << "it should be " << dims[i]
                  << ", but is " << actual_dims[i],
                  ValueException);
      }
    }
  }
  // Read into a buffer, after ensuring that the size of the dataset
  // is as expected
  void read_dims(void *data, hid_t type_id, Ints dims) {
    check_dims(dims);
    IMP_HDF5_ERRCHECK(H5Dread(id_, type_id, H5S_ALL, H5S_ALL,
                              H5P_DEFAULT, data));
  }
  // Read into a buffer (size is not checked)
  void read(void *data, hid_t type_id) {
    IMP_HDF5_ERRCHECK(H5Dread(id_, type_id, H5S_ALL, H5S_ALL,
                              H5P_DEFAULT, data));
  }
public:
  Hdf5Dataset(hid_t loc_id, std::string name) : id_(-1) {
    IMP_HDF5_ERRCHECK(id_ = H5Dopen(loc_id, name.c_str(), H5P_DEFAULT));
  }

  void read_float(float *data) {
    read(data, H5T_NATIVE_FLOAT);
  }

  void read_int(int *data) {
    read(data, H5T_NATIVE_INT);
  }

  // Read into an int buffer, after ensuring the size is as expected
  void read_int_dims(int *data, Ints dims) {
    read_dims(data, H5T_NATIVE_INT, dims);
  }

  // Read a 1-dimensional dataset into a std::vector<int> and return it.
  // If size is specified, the dataset must contain that many elements.
  std::vector<int> read_int_vector(int size=0) {
    Hdf5Dataspace space(*this);
    Ints actual_dims = space.get_simple_extent_dims();
    check_rank(1, actual_dims.size());
    if (size != 0) {
      Ints dims(1, size);
      check_each_dim(dims, actual_dims);
    }
    std::vector<int> ret(actual_dims[0]);
    read_int(&ret[0]);
    return ret;
  }

  // Read a dataset into a std::vector<std::string>. The dataset can be
  // any size or shape.
  std::vector<std::string> read_string_vector() {
    Hdf5Dataspace space(*this);
    int nstr = space.get_simple_extent_npoints();
    Hdf5StringDatatype str_type;
    std::vector<char *> strpointers(nstr);
    read(&strpointers[0], str_type.get());
    // Copy data into our own structure, so we can use HDF5's reclaim
    // function on the original
    std::vector<std::string> ret;
    for (int i = 0; i < nstr; ++i) {
      ret.push_back(strpointers[i]);
    }
    IMP_HDF5_ERRCHECK(H5Dvlen_reclaim(str_type.get(), space.get(),
                                      H5P_DEFAULT, &strpointers[0]));
    return ret;
  }

  IMP_HDF5_OBJECT(Hdf5Dataset, H5Dclose)
};

class Hdf5Group : public boost::noncopyable {
  hid_t id_;
public:
  Hdf5Group(hid_t loc_id, std::string name) : id_(-1) {
    IMP_HDF5_ERRCHECK(id_ = H5Gopen(loc_id, name.c_str(), H5P_DEFAULT));
  }

  // Read and return a single float attribute attached to this group
  float read_float_attribute(std::string name) {
    Hdf5Attribute attr(id_, name);
    Hdf5Dataspace space(attr);
    int npoints = space.get_simple_extent_npoints();
    if (npoints != 1) {
      IMP_THROW("Attribute " << name << " should be of size 1 (it is "
                << npoints << ")", ValueException);
    }
    float val;
    attr.read_float(&val);
    return val;
  }

  IMP_HDF5_OBJECT(Hdf5Group, H5Gclose)
};

Hdf5Dataspace::Hdf5Dataspace(Hdf5Dataset &dataset) : id_(-1) {
  IMP_HDF5_ERRCHECK(id_ = H5Dget_space(dataset.get()));
}

Hdf5Dataspace::Hdf5Dataspace(Hdf5Attribute &attribute) : id_(-1) {
  IMP_HDF5_ERRCHECK(id_ = H5Aget_space(attribute.get()));
}

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SOAP_HDF_5_H */
