/**
 *  \file RMF/HDF5File.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_HDF_5FILE_H
#define IMPLIBRMF_HDF_5FILE_H

#include "RMF_config.h"
#include "HDF5Group.h"


namespace RMF {
  /** Store a handle to an HDF5 file. See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame08TheFile.html,
  the HDF5 manual} for more information.*/
  class RMFEXPORT HDF5File: public HDF5Group {
  public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    HDF5File(HDF5SharedHandle *h);
#endif
    bool get_is_writable() const;
    std::string get_name() const;
    void flush();
    ~HDF5File();
  };

  /** Create a new hdf5 file, clearing any existing file with the same
      name if needed.
  */
  RMFEXPORT HDF5File create_hdf5_file(std::string name);

  /** Open an existing hdf5 file.
  */
  RMFEXPORT HDF5File open_hdf5_file(std::string name);

  /** Open an existing hdf5 file read only.
  */
  RMFEXPORT HDF5File open_hdf5_file_read_only(std::string name);

  /** */
  typedef vector<HDF5Group> HDF5Groups;
  /** */
  typedef vector<HDF5File> HDF5Files;

  /** */
  inline int get_number_of_open_hdf5_handles() {
    H5garbage_collect();
    return H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
  }

} /* namespace RMF */

#endif /* IMPLIBRMF_HDF_5FILE_H */
