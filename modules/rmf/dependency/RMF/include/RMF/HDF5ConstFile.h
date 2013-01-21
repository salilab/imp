/**
 *  \file RMF/HDF5ConstFile.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5CONST_FILE_H
#define RMF_HDF_5CONST_FILE_H

#include <RMF/config.h>
#include "HDF5ConstGroup.h"

namespace RMF {
/** Store a handle to non-writeable HDF5 file. See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame08TheFile.html,
   the HDF5 manual} for more information.

   \note At the moment, a given file can only be opened once within a
   a process. This can be fixed if there is demand.
 */
class RMFEXPORT HDF5ConstFile: public HDF5ConstGroup {
public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  HDF5ConstFile(HDF5SharedHandle *h);
#endif
  HDF5ConstFile(HDF5File f);
  HDF5ConstFile() {
  }
  RMF_SHOWABLE(HDF5ConstFile, "HDF5ConstFile " << get_name());
  std::string get_name() const;
  ~HDF5ConstFile();
};

/** Open an existing hdf5 file read only. The file cannot already
    be open.
 */
RMFEXPORT HDF5ConstFile open_hdf5_file_read_only(std::string name);

/** */
typedef vector<HDF5Group> HDF5ConstGroups;
/** */
typedef vector<HDF5File> HDF5ConstFiles;

/** For debugging, one can get the number of open hdf5 handles for either
    one file, or the whole system.*/
RMFEXPORT int get_number_of_open_hdf5_handles(HDF5ConstFile f
                                                = HDF5ConstFile());
/** For debugging you can get the names of open handles in either one file
   or the whole process.*/
RMFEXPORT Strings get_open_hdf5_handle_names(HDF5ConstFile f
                                               = HDF5ConstFile());

} /* namespace RMF */

#endif /* RMF_HDF_5CONST_FILE_H */
