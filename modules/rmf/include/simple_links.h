/**
 *  \file IMP/rmf/simple_links.h
 *  \brief Manage links between IMP objects and RMF nodes.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_SIMPLE_LINKS_H
#define IMPRMF_SIMPLE_LINKS_H

#include <IMP/rmf/rmf_config.h>
#include "links.h"
#include "associations.h"
#include <IMP/Object.h>
#include <IMP/Pointer.h>
#include <IMP/object_macros.h>
#include <IMP/log_macros.h>
#include <IMP/Model.h>
#include <RMF/RestoreCurrentFrame.h>
#include <RMF/SetCurrentFrame.h>
#include <RMF/names.h>
#include <RMF/decorators.h>

IMPRMF_BEGIN_NAMESPACE

/** Manage a link between an IMP object and an RMF node.

    \unstable{SimpleLoadLink}
*/
template <class O>
class SimpleLoadLink : public LoadLink {
  Vector<Pointer<O> > os_;
  RMF::NodeIDs nhs_;

 protected:
  virtual void do_load_one(RMF::NodeConstHandle nh, O *o) = 0;
  void do_load(RMF::FileConstHandle fh) IMP_OVERRIDE {
    IMP_OBJECT_LOG;
    for (unsigned int i = 0; i < os_.size(); ++i) {
      IMP_LOG_VERBOSE("Loading " << fh.get_node(nhs_[i]) << std::endl);
      do_load_one(fh.get_node(nhs_[i]), os_[i]);
    }
  }
  virtual void do_add_link(O *, RMF::NodeConstHandle) {};
  void add_link(O *o, RMF::NodeConstHandle nh) {
    os_.push_back(o);
    nhs_.push_back(nh.get_id());
    set_association(nh, o, true);
  }
  virtual bool get_is(RMF::NodeConstHandle nh) const = 0;
  virtual O *do_create(RMF::NodeConstHandle) { IMP_FAILURE("Wrong create"); }
  virtual O *do_create(RMF::NodeConstHandle, Model *) {
    IMP_FAILURE("Wrong create");
  }
  SimpleLoadLink(std::string name) : LoadLink(name) {}

 public:
  /** Create all the entities under the passed root.*/
  Vector<Pointer<O> > create(RMF::NodeConstHandle rt) {
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE("Creating IMP objects from " << rt << std::endl);
    RMF::SetCurrentFrame sf(rt.get_file(), RMF::FrameID(0));
    RMF::NodeConstHandles ch = rt.get_children();
    Vector<Pointer<O> > ret;
    for (unsigned int i = 0; i < ch.size(); ++i) {
      IMP_LOG_VERBOSE("Checking " << ch[i] << std::endl);
      if (get_is(ch[i])) {
        IMP_LOG_VERBOSE("Adding " << ch[i] << std::endl);
        Pointer<O> o = do_create(ch[i]);
        add_link(o, ch[i]);
        ret.push_back(o);
        o->set_was_used(true);
      }
    }
    return ret;
  }

  /** Create all the entities under the passed root.*/
  Vector<Pointer<O> > create(RMF::NodeConstHandle rt,
                                         Model *m) {
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE("Creating Model objects from " << rt << std::endl);
    RMF::SetCurrentFrame sf(rt.get_file(), RMF::FrameID(0));
    RMF::NodeConstHandles ch = rt.get_children();
    Vector<Pointer<O> > ret;
    for (unsigned int i = 0; i < ch.size(); ++i) {
      IMP_LOG_VERBOSE("Checking " << ch[i] << std::endl);
      if (get_is(ch[i])) {
        IMP_LOG_VERBOSE("Adding " << ch[i] << std::endl);
        Pointer<O> o = do_create(ch[i], m);
        add_link(o, ch[i]);
        ret.push_back(o);
        o->set_was_used(true);
      }
    }
    return ret;
  }

  void link(RMF::NodeConstHandle rt,
            const Vector<Pointer<O> > &ps) {
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE("Linking " << rt << " to " << ps << std::endl);

    RMF::RestoreCurrentFrame sf(rt.get_file());
    set_was_used(true);
    RMF::NodeConstHandles chs = rt.get_children();
    RMF::NodeConstHandles matching_chs;
    IMP_FOREACH(RMF::NodeConstHandle ch, rt.get_children()) {
      IMP_LOG_VERBOSE("Checking " << ch << std::endl);
      if (get_is(ch)) matching_chs.push_back(ch);
    }
    if (matching_chs.size() != ps.size()) {
      IMP_THROW("Founding " << matching_chs.size() << " matching nodes "
                            << "but passed " << ps.size() << " to match with.",
                ValueException);
    }
    for (unsigned int i = 0; i < matching_chs.size(); ++i) {
      IMP_LOG_VERBOSE("Linking " << matching_chs[i] << std::endl);
      add_link(ps[i], matching_chs[i]);
      ps[i]->set_was_used(true);
      do_add_link(ps[i], matching_chs[i]);
    }
  }
};

/** Manage a link between an IMP object and an RMF node.

    \unstable{SimpleSaveLink}
*/
template <class O>
class SimpleSaveLink : public SaveLink {
  Vector<Pointer<O> > os_;
  RMF::NodeIDs nhs_;

 protected:
  virtual void do_save_one(O *o, RMF::NodeHandle nh) = 0;
  void do_save(RMF::FileHandle fh) IMP_OVERRIDE {
    for (unsigned int i = 0; i < os_.size(); ++i) {
      IMP_LOG_VERBOSE("Saving to " << fh.get_node(nhs_[i]) << std::endl);

      os_[i]->set_was_used(true);
      IMP_LOG_VERBOSE("Saving " << Showable(os_[i]) << std::endl);
      do_save_one(os_[i], fh.get_node(nhs_[i]));
    }
  }
  virtual void do_add(O *o, RMF::NodeHandle c) { add_link(o, c); }
  virtual RMF::NodeType get_type(O *o) const = 0;
  void add_link(O *o, RMF::NodeConstHandle nh) {
    os_.push_back(o);
    nhs_.push_back(nh.get_id());
    set_association(nh, o, true);
  }
  SimpleSaveLink(std::string name) : SaveLink(name) {}

 public:
  void add(RMF::NodeHandle parent, const Vector<Pointer<O> > &os) {
    IMP_OBJECT_LOG;
    IMP_LOG_TERSE("Adding " << os << " to rmf" << std::endl);
    RMF::FileHandle file = parent.get_file();
    RMF::decorator::AliasFactory af(file);
    for (unsigned int i = 0; i < os.size(); ++i) {
      std::string nicename = RMF::get_as_node_name(os[i]->get_name());
      if (get_has_associated_node(file, os[i])) {
        RMF::NodeHandle c = parent.add_child(nicename, RMF::ALIAS);
        af.get(c).set_aliased(get_node_from_association(file, os[i]));
      } else {
        RMF::NodeHandle c = parent.add_child(nicename, get_type(os[i]));
        do_add(os[i], c);
        os[i]->set_was_used(true);
      }
    }
  }
};

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_SIMPLE_LINKS_H */
