/**
 *  \file RMF/HDF5ConstFile.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_HDF_5CONST_FILE_H
#define IMPLIBRMF_HDF_5CONST_FILE_H

#include "RMF_config.h"
#include "HDF5ConstGroup.h"

namespace RMF {
  /** Store a handle to non-writeable HDF5 file. See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame08TheFile.html,
  the HDF5 manual} for more information.*/
  class RMFEXPORT HDF5ConstFile: public HDF5ConstGroup {
  public:
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
    HDF5ConstFile(HDF5SharedHandle *h);
#endif
    HDF5ConstFile(HDF5File f);
    HDF5ConstFile(){}
    IMP_RMF_SHOWABLE(HDF5ConstFile, "HDF5ConstFile " << get_name());
    std::string get_name() const;
    ~HDF5ConstFile();
  };

  /** Open an existing hdf5 file read only.
  */
  RMFEXPORT HDF5ConstFile open_hdf5_file_read_only(std::string name);

  /** */
  typedef vector<HDF5Group> HDF5ConstGroups;
  /** */
  typedef vector<HDF5File> HDF5ConstFiles;

  /** */
  inline int get_number_of_open_hdf5_handles() {
    H5garbage_collect();
    return H5Fget_obj_count(H5F_OBJ_ALL, H5F_OBJ_ALL);
  }

} /* namespace RMF */

#endif /* IMPLIBRMF_HDF_5CONST_FILE_H */
