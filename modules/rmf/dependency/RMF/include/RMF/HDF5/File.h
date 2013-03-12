/**
 *  \file RMF/File.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5FILE_H
#define RMF_HDF_5FILE_H

#include <RMF/config.h>
#include "Group.h"
#include "ConstFile.h"

RMF_ENABLE_WARNINGS

namespace RMF {
/** We provide a simple set of classes for accessing core HDF5 functionality from C++. This was needed since
    - The HDF5 C interface is not trivial to use peroperly, particularly in regards to data type conversions and resource management
    - It is very easy to use the C interface incorrectly without knowning it.
    - The \external{http://hdf5.org, standard HDF5 C++ interface} doesn't really simplify use of the HDF5 library and doesn't make use of the features of C++.

    The main classes provide access too:
    - HDF5 files through RMF::File and RMF::HDF5::ConstFile
    - HDF5 groups through RMF::Group and RMF::HDF5::ConstGroup
    - HDF5 data sets through RMF::DataSetD and RMF::HDF5::ConstDataSetD

    The \c Const variants are for read only files.

    They allow one to create a manipulate data sets containing floating point values, integers, strings, variable length arrays of floating point values and a few other types. Attributes on data sets of groups of those types can also be manipulated.

    The top level classes are, in turn, derived from RMF::Object, RMF::HDF5::ConstAttributes and RMF::HDF5::MutableAttributes which provide access to general HDF5 per-object and attribute functionality (all objects in HDF5 can have arbitrary small pieces of data attached to them via attributes).

    Not all of the functionality of the HDF5 \c C library is covered in C++. You can get the corresponding native handle from most objects with methods like RMF::File::get_handle() and use that for other operations if needed. Or submit a patch to the library.
*/
namespace HDF5 {

/** Store a handle to an HDF5 file. See
   \external{http://www.hdfgroup.org/HDF5/doc/UG/UG_frame08TheFile.html,
   the HDF5 manual} for more information.*/
class RMFEXPORT File: public Group {
public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  File(SharedHandle *h);
  // silliness to make RMF easier to implement
  bool get_is_writable() const {
    unsigned int intent;
    RMF_HDF5_CALL(H5Fget_intent(get_handle(), &intent));
    return intent == H5F_ACC_RDWR;
  }
#endif
  RMF_SHOWABLE(File, "File " << get_name());
  File() {
  }
  std::string get_name() const;
  void flush();
  ~File();
};

/** Create a new hdf5 file, clearing any existing file with the same
    name if needed. The file cannot already be open.

    \exception RMF::IOException couldn't create file
 */
RMFEXPORT File create_file(std::string name);

/** Open an existing hdf5 file. The file cannot already be open/.

    \exception RMF::IOException couldn't open file
 */
RMFEXPORT File open_file(std::string name);

#ifndef RMF_DOXYGEN
/** Open an existing hdf5 file read only. The file cannot already
    be open.
 */
RMFEXPORT File
open_file_read_only_returning_nonconst(std::string name);
#endif

/** */
typedef std::vector<Group> Groups;
/** */
typedef std::vector<File> Files;

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5FILE_H */
