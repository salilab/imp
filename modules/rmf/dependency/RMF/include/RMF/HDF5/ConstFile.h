/**
 *  \file RMF/ConstFile.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5CONST_FILE_H
#define RMF_HDF_5CONST_FILE_H

#include <RMF/config.h>
#include "ConstGroup.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {

/** Store a handle to non-writeable HDF5 file. See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame08TheFile.html,
   the HDF5 manual} for more information.

   \note At the moment, a given file can only be opened once within a
   a process. This can be fixed if there is demand.
 */
class RMFEXPORT ConstFile: public ConstGroup {
public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  ConstFile(SharedHandle *h);
#endif
  ConstFile(File f);
  ConstFile() {
  }
  RMF_SHOWABLE(ConstFile, "ConstFile " << get_name());
  std::string get_name() const;
  ~ConstFile();
};

/** Open an existing hdf5 file read only. The file cannot already
    be open.
 */
RMFEXPORT ConstFile open_file_read_only(std::string name);

/** */
typedef std::vector<Group> ConstGroups;
/** */
typedef std::vector<File> ConstFiles;

/** For debugging, one can get the number of open hdf5 handles for either
    one file, or the whole system.*/
RMFEXPORT int get_number_of_open_handles(ConstFile f
                                                = ConstFile());
/** For debugging you can get the names of open handles in either one file
   or the whole process.*/
RMFEXPORT Strings get_open_handle_names(ConstFile f
                                               = ConstFile());

/** Turn on and off printing of hdf5 error messages. They can help in
      diagnostics, but, for the moment, can only be output to standard
      error and so are off by default.
 */
RMFEXPORT void set_show_errors(bool tf) ;

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5CONST_FILE_H */
