/**
 *  \file RMF/FileHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__FILE_HANDLE_H
#define RMF__FILE_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "Key.h"
#include "FileConstHandle.h"


namespace RMF {
  class HDF5Group;

  //! A handle for an RMF file
  /** Use this handle to perform operations relevant to the
      whole RMF hierarchy as well as to start traversal of the
      hierarchy.

      Make sure to check out the base class documentation
      for the non-modifying methods.

      \see create_rmf_file
      \see open_rmf_file
  */
  class RMFEXPORT FileHandle: public FileConstHandle {
    friend class NodeHandle;
    friend class internal::SharedData;
    FileHandle(internal::SharedData *shared_);
  public:
    //! Empty file handle, no open file.
    FileHandle(){}
#ifndef RMF_DOXYGEN
    FileHandle(std::string name, bool create);
#endif

    /** Frames can have associated comments which can be used to label
        particular frames of interest.*/
    void set_frame_name(std::string comment);

    /** Return the root of the hierarchy stored in the file.
     */
    NodeHandle get_root_node() const {
      return NodeHandle(0, get_shared_data());
    }

#ifndef SWIG
    /** Each node in the hierarchy can be associated with some arbitrary bit
        of external data. Nodes can be extracted using these bits of data.
    */
    template <class T>
      NodeHandle get_node_from_association(const T&d) const {
      if (! get_shared_data()->get_has_associated_node(d)) {
        return NodeHandle();
      } else {
        return NodeHandle(get_shared_data()->get_associated_node(d),
                          get_shared_data());
      }
    }
#else
    NodeHandle get_node_from_association(void*d) const ;
#endif
    /** Return a NodeHandle from a NodeID. The NodeID must refer
        to a valid NodeHandle.*/
    NodeHandle get_node_from_id(NodeID id) const;
    /** Suggest how many frames the file is likely to have. This can
        make writing more efficient as space will be preallocated.
    */
    void set_number_of_frames_hint(unsigned int i) {
      get_shared_data()->save_frames_hint(i);
    }
    /** Each RMF structure has an associated description. This should
        consist of unstructured text describing the contents of the RMF
        data. Conventionally. this description can consist of multiple
        paragraphs, each separated by a newline character and should end
        in a newline.
    */
    void set_description(std::string descr);

    /** Each RMF structure has an associated field that the code that
        produced the file can use to describe itself.
    */
    void set_producer(std::string);


    /*HDF5Group get_hdf5_group() const {
      return get_shared_data()->get_group();
      }*/
  };

  typedef vector<FileHandle> FileHandles;

  /**
   Create an RMF from a file system path.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't create file, or unsupported file format
   */
  RMFEXPORT FileHandle create_rmf_file(std::string path);

  /**
   Open an RMF from a file system path.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't create file, or unsupported file format
*/
  RMFEXPORT FileHandle open_rmf_file(std::string path);


} /* namespace RMF */

#endif /* RMF__FILE_HANDLE_H */
