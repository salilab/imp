/**
 *  \file RMF/HDF5/ConstFile.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF5_CONST_FILE_H
#define RMF_HDF5_CONST_FILE_H

#include "RMF/config.h"
#include "ConstGroup.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

/** Store a handle to a non-writable HDF5 file. See
   \external{https://support.hdfgroup.org/HDF5/doc/RM/RM_H5F.html,
   the HDF5 manual} for more information.

   \note At the moment, a given file can only be opened once within a
   a process. This can be fixed if there is demand.
 */
class RMFEXPORT ConstFile : public ConstGroup {
 public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  ConstFile(boost::shared_ptr<SharedHandle> h);
#endif
  ConstFile(File f);
  ConstFile() {}
  RMF_SHOWABLE(ConstFile, "ConstFile " << get_name());
  std::string get_name() const;
  ~ConstFile();
};

/** Open an existing HDF5 file read only. The file cannot already
    be open.
 */
RMFEXPORT ConstFile open_file_read_only(std::string name);

/** */
typedef std::vector<Group> ConstGroups;
/** */
typedef std::vector<File> ConstFiles;

/** For debugging, one can get the number of open HDF5 handles for either
    one file, or the whole system.*/
RMFEXPORT int get_number_of_open_handles(ConstFile f = ConstFile());
/** For debugging you can get the names of open handles in either one file
   or the whole process.*/
RMFEXPORT Strings get_open_handle_names(ConstFile f = ConstFile());

/** Turn on and off printing of HDF5 error messages. They can help in
      diagnostics, but, for the moment, can only be output to standard
      error and so are off by default.
 */
RMFEXPORT void set_show_errors(bool tf);

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF5_CONST_FILE_H */
