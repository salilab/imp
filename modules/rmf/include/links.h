/**
 *  \file IMP/rmf/links.h
 *  \brief Manage links between \imp objects and a part of the RMF file.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_LINKS_H
#define IMPRMF_LINKS_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/object_macros.h>
#include <IMP/log_macros.h>

IMPRMF_BEGIN_NAMESPACE

class SaveLink;
class LoadLink;
IMP_OBJECTS(SaveLink, SaveLinks);

IMP_OBJECTS(LoadLink, LoadLinks);

/** Manage a link between an \imp object and a part
    of the RMF file. This allows conformations to be
    loaded flexibly.

    LoadLinks must not save any handles to RMF objects.

    \unstable{LoadLink}
*/
class IMPRMFEXPORT LoadLink : public Object {
  bool frame_loaded_;

 protected:
  virtual void do_load(RMF::FileConstHandle fh) = 0;
  LoadLink(std::string name);

 public:
  void load(RMF::FileConstHandle fh) {
    IMP_OBJECT_LOG;
    set_was_used(true);
    do_load(fh);
    frame_loaded_ = true;
  }
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(LoadLink);
};
/** Manage a link between an \imp object and a part
    of the RMF file. This allows conformations to be
    saved flexibly.

    SaveLinks must not save any handles to RMF objects.

    \unstable{SaveLink}
*/
class IMPRMFEXPORT SaveLink : public Object {
  bool frame_saved_;

 protected:
  virtual void do_save(RMF::FileHandle hf) = 0;
  SaveLink(std::string name);

 public:
  void save(RMF::FileHandle fh) {
    IMP_OBJECT_LOG;
    set_was_used(true);
    do_save(fh);
    frame_saved_ = true;
  }
  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(SaveLink);
};

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_LINKS_H */
