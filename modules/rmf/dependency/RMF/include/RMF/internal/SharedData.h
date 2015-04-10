/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_H
#define RMF_INTERNAL_SHARED_DATA_H

#include <boost/shared_ptr.hpp>
#include <string>

#include "RMF/ID.h"
#include "RMF/config.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "SharedDataCategory.h"
#include "SharedDataData.h"
#include "SharedDataFile.h"
#include "SharedDataHierarchy.h"
#include "SharedDataKeys.h"
#include "SharedDataFrames.h"
#include "SharedDataUserData.h"
#include "RMF/names.h"
#include "RMF/traits.h"
#include "RMF/types.h"
#include "paths.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace backends {
struct IO;
}

#define RMF_HOIST(Traits, UCName)                \
  using SharedDataKeys<Traits>::get_key;         \
  using SharedDataKeys<Traits>::get_name;        \
  using SharedDataKeys<Traits>::get_keys;        \
  using SharedDataKeys<Traits>::get_category;    \
  using SharedDataKeys<Traits>::ensure_key;      \
  using SharedDataKeys<Traits>::access_key_data; \
  using SharedDataKeys<Traits>::get_key_data;

#define RMF_SHARED_DATA_PARENT(Traits, UCName) \
 public                                        \
  SharedDataKeys<Traits>,

namespace internal {

class RMFEXPORT SharedData
    : public SharedDataUserData,
      public SharedDataFile,
      public SharedDataHierarchy,
      public SharedDataCategory,
      public SharedDataData,
      RMF_FOREACH_TYPE(RMF_SHARED_DATA_PARENT) public SharedDataFrames {
  std::string path_;
  bool write_;
  boost::shared_ptr<backends::IO> io_;
  FrameID loaded_frame_;

 public:
  using SharedDataHierarchy::get_name;
  using SharedDataCategory::get_name;
  using SharedDataCategory::get_category;
  using SharedDataHierarchy::set_name;
  using SharedDataHierarchy::set_type;

  RMF_FOREACH_TYPE(RMF_HOIST);

  SharedData(boost::shared_ptr<backends::IO> io, std::string name, bool write,
             bool created);
  void set_loaded_frame(FrameID frame);
  FrameID add_frame(std::string name, FrameType type);
  FrameID add_frame(std::string name, FrameID parent, FrameType type);
  void flush();
  void reload();
  FrameID get_loaded_frame() const { return loaded_frame_; }
  std::string get_file_path() const { return path_; }
  std::string get_file_name() const { return internal::get_file_name(path_); }
  ~SharedData();
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_H */
