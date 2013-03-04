/**
 *  \file test_fill.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <hdf5.h>
#include <iostream>

//#include <RMF/hdf5_handle.h>

#define HDF5_CALL(x)                                             \
  {                                                              \
    herr_t ret = x;                                              \
    if (ret < 0) {                                               \
      std::cerr << "Error " << ret << " on " << #x << std::endl; \
    }                                                            \
  }

#define HDF5_HANDLE(name, cmd, cleanup) hid_t name = cmd
//#define HDF5_HANDLE(name, cmd, cleanup) RMF_HDF5_HANDLE(name, cmd, cleanup)

namespace {
const int D = 2;
void set_size(hid_t ds,
              int i, int j) {
  hsize_t nd[D];
  nd[0] = i;
  nd[1] = j;
  HDF5_CALL(H5Dset_extent(ds, &nd[0]));
}
hid_t get_parameters() {
  hid_t plist = H5Pcreate(H5P_FILE_ACCESS);
  HDF5_CALL(H5Pset_sieve_buf_size(plist, 1000000));
  HDF5_CALL(H5Pset_cache(plist, 0, 1000, 1000000, 0.0));
  return plist;
}


void set_value(hid_t ds,
               int i, int j,
               double v) {
  hsize_t ij[2]; ij[0] = i; ij[1] = j;
  hsize_t one = 1;
  HDF5_HANDLE(ids,   H5Screate_simple(1, &one, NULL), &H5Sclose);
  hsize_t ones[2]; ones[0] = 1; ones[1] = 1;
  HDF5_HANDLE(space, H5Dget_space(ds),                &H5Sclose);
  HDF5_CALL(H5Sselect_hyperslab(space,
                                H5S_SELECT_SET, ij,
                                ones, ones,
                                NULL));
  H5Dwrite(ds,
           H5T_NATIVE_DOUBLE, ids, space,
           H5P_DEFAULT, &v);
}
}

int main(int argc, char *argv[]) {
  HDF5_CALL(H5open());
  HDF5_HANDLE(plist, get_parameters(), H5Pclose);
  HDF5_HANDLE(file,  H5Fcreate("test.hdf5",
                               H5F_ACC_TRUNC, H5P_DEFAULT,
                               plist), &H5Fclose);
  hsize_t dims[D] = {0};
  hsize_t cdims[D] = {64};
  cdims[D - 1] = 1;
  hsize_t maxs[D];
  maxs[0] = H5S_UNLIMITED;
  maxs[1] = H5S_UNLIMITED;
  double fill = -1;
  HDF5_HANDLE(space,  H5Screate_simple(D, dims, maxs), &H5Sclose);
  HDF5_HANDLE(dplist, H5Pcreate(H5P_DATASET_CREATE),   &H5Pclose);
    HDF5_CALL(H5Pset_chunk(dplist, D, cdims));
    HDF5_CALL(H5Pset_fill_value(dplist, H5T_NATIVE_DOUBLE,
                              &fill));
  if (argc > 1 && argv[1][0] == '+') {
    std::cout << "old" << std::endl;
    HDF5_CALL(H5Pset_fill_time(dplist, H5D_FILL_TIME_IFSET));
    HDF5_CALL(H5Pset_alloc_time(dplist, H5D_ALLOC_TIME_LATE));
  } else if (argc > 1 && argv[1][0] == '-') {
    std::cout << "new" << std::endl;
    HDF5_CALL(H5Pset_fill_time(dplist, H5D_FILL_TIME_ALLOC));
    HDF5_CALL(H5Pset_alloc_time(dplist, H5D_ALLOC_TIME_INCR));
  } else {
    std::cout << "default" << std::endl;
    HDF5_CALL(H5Pset_fill_time(dplist, H5D_FILL_TIME_IFSET));
    HDF5_CALL(H5Pset_alloc_time(dplist, H5D_ALLOC_TIME_EARLY));
  }
  HDF5_HANDLE(ds, H5Dcreate2(file,
                             "dataset",
                             H5T_IEEE_F64LE,
                             space, H5P_DEFAULT, dplist, H5P_DEFAULT),
              &H5Dclose);
  set_size(ds, 1, 1);
  set_value(ds, 0, 0, .5);
  set_size(ds, 1, 2);
  set_value(ds, 0, 1, 1);
  set_size(ds, 1, 3);
  set_value(ds, 0, 2, 0);
  set_size(ds, 3, 4);
  return 0;
}
