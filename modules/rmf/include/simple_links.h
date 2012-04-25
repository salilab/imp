/**
 *  \file IMP/rmf/simple_links.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_SIMPLE_LINKS_H
#define IMPRMF_SIMPLE_LINKS_H

#include "rmf_config.h"
#include "links.h"
#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/object_macros.h>
#include <IMP/base/log_macros.h>
#include <RMF/names.h>

IMPRMF_BEGIN_NAMESPACE


typedef base::Object* AssociationType;
class SaveLink;
typedef base::Pointer<SaveLink> SaveLinkAssociationType;
class LoadLink;
typedef base::Pointer<LoadLink> LoadLinkAssociationType;

inline
void set_association(RMF::NodeConstHandle nh,
                     base::Object *o, bool overwrite=false) {
  nh.set_association(AssociationType(o), overwrite);
}

template <class T>
inline T* get_association(RMF::NodeConstHandle nh) {
  AssociationType o
    = nh.get_association<AssociationType >();
  base::Object *op= o;
  return dynamic_cast<T*>(op);
}


inline RMF::NodeConstHandle get_node_from_association(RMF::FileConstHandle nh,
                                                      base::Object *oi) {
  AssociationType o(oi);
  return nh.get_node_from_association(o);
}

inline RMF::NodeHandle get_node_from_association(RMF::FileHandle nh,
                                                 base::Object *oi) {
  AssociationType o(oi);
  return nh.get_node_from_association(o);
}



template <class O>
class SimpleLoadLink: public LoadLink {
  base::Vector<base::Pointer<O> > os_;
  RMF::NodeIDs nhs_;
 protected:
  virtual void do_load_one( RMF::NodeConstHandle nh,
                            O *o, unsigned int frame)=0;
  void do_load(RMF::FileConstHandle fh, unsigned int frame) {
    for (unsigned int i=0; i< os_.size();  ++i) {
      do_load_one(fh.get_node_from_id(nhs_[i]), os_[i], frame);
    }
  }
  virtual void do_add_link(O *, RMF::NodeConstHandle) {}
  void add_link(O *o, RMF::NodeConstHandle nh) {
    os_.push_back(o);
    nhs_.push_back(nh.get_id());
    set_association(nh, o, true);
  }
  virtual bool get_is(RMF::NodeConstHandle nh) const=0;
  virtual O* do_create(RMF::NodeConstHandle nh) =0;
  SimpleLoadLink(std::string name): LoadLink(name){}
public:
  base::Vector<base::Pointer<O> > create(RMF::NodeConstHandle rt) {
    IMP_OBJECT_LOG;
    RMF::NodeConstHandles ch= rt.get_children();
    unsigned int old=os_.size();
    for (unsigned int i=0; i< ch.size(); ++i) {
      IMP_LOG(VERBOSE, "Checking " << ch[i] << std::endl);
      if (get_is(ch[i])) {
        IMP_LOG(VERBOSE, "Adding " << ch[i] << std::endl);
        base::Pointer<O> o=do_create(ch[i]);
        add_link(o, ch[i]);
      }
    }
    return base::Vector<base::Pointer<O> >(os_.begin()+old,
                                           os_.end());
  }
  void link(RMF::NodeConstHandle rt,
            const base::Vector<base::Pointer<O> > &ps) {
    IMP_OBJECT_LOG;
    set_was_used(true);
    RMF::NodeConstHandles ch= rt.get_children();
    int links=0;
    for (unsigned int i=0; i< ch.size(); ++i) {
      IMP_LOG(VERBOSE, "Checking " << ch[i] << std::endl);
      if (get_is(ch[i])) {
        IMP_LOG(VERBOSE, "Linking " << ch[i] << std::endl);
        add_link(ps[links], ch[i]);
        do_add_link(ps[links], ch[i]);
        ++links;
      }
    }
    IMP_USAGE_CHECK(os_.size()==nhs_.size(),
                    "Didn't find enough matching things.");
    IMP_USAGE_CHECK(links==static_cast<int>(ps.size()),
                    "Didn't find enough matching things. Found "
                    << links << " wanted " << ps.size());
  }

};


template <class O>
class SimpleSaveLink: public SaveLink {
  base::Vector<base::Pointer<O> > os_;
  RMF::NodeIDs nhs_;
 protected:
  virtual void do_save_one(O *o,
                           RMF::NodeHandle nh,
                           unsigned int frame)=0;
  void do_save(RMF::FileHandle fh, unsigned int frame) {
    for (unsigned int i=0; i< os_.size();  ++i) {
      IMP_LOG(VERBOSE, "Saving " << Showable(os_[i]) << std::endl);
      do_save_one(os_[i], fh.get_node_from_id(nhs_[i]), frame);
    }
  }
  virtual void do_add(O *, RMF::NodeHandle) {}
  virtual RMF::NodeType get_type(O*o) const =0;
  SimpleSaveLink(std::string name): SaveLink(name){}
public:
  void add(RMF::NodeHandle parent,
           const base::Vector<base::Pointer<O> > &os) {
    IMP_OBJECT_LOG;
    for (unsigned int i=0; i< os.size(); ++i) {
      std::string nicename= RMF::get_as_node_name(os[i]->get_name());
      RMF::NodeHandle c= parent.add_child(nicename,
                                          get_type(os[i]));
      do_add(os[i], c);
      os_.push_back(os[i]);
      nhs_.push_back(c.get_id());
      set_association(c, os[i]);
    }
  }
};

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_SIMPLE_LINKS_H */
