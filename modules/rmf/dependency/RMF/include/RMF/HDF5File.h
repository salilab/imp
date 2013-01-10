/**
 *  \file RMF/HDF5File.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5FILE_H
#define RMF_HDF_5FILE_H

#include <RMF/config.h>
#include "HDF5Group.h"
#include "HDF5ConstFile.h"


namespace RMF {
/** Store a handle to an HDF5 file. See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame08TheFile.html,
   the HDF5 manual} for more information.*/
class RMFEXPORT HDF5File: public HDF5Group {
public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  HDF5File(HDF5SharedHandle *h);
  // silliness to make RMF easier to implement
  bool get_is_writable() const {
    unsigned int intent;
    RMF_HDF5_CALL(H5Fget_intent(get_handle(), &intent));
    return intent == H5F_ACC_RDWR;
  }
#endif
  RMF_SHOWABLE(HDF5File, "HDF5File " << get_name());
  HDF5File() {
  }
  std::string get_name() const;
  void flush();
  ~HDF5File();
};

/** Create a new hdf5 file, clearing any existing file with the same
    name if needed. The file cannot already be open.

    \exception RMF::IOException couldn't create file
 */
RMFEXPORT HDF5File create_hdf5_file(std::string name);

/** Open an existing hdf5 file. The file cannot already be open/.

    \exception RMF::IOException couldn't open file
 */
RMFEXPORT HDF5File open_hdf5_file(std::string name);

#ifndef RMF_DOXYGEN
/** Open an existing hdf5 file read only. The file cannot already
    be open.
 */
RMFEXPORT HDF5File
open_hdf5_file_read_only_returning_nonconst(std::string name);
#endif

/** */
typedef vector<HDF5Group> HDF5Groups;
/** */
typedef vector<HDF5File> HDF5Files;


} /* namespace RMF */

#endif /* RMF_HDF_5FILE_H */
