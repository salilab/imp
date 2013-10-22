/**
 *  \file IMP/rmf/link_macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPRMF_LINK_HELPERS_H
#define IMPRMF_LINK_HELPERS_H
#include <IMP/rmf/rmf_config.h>
#include "../links.h"
#include <IMP/base/Pointer.h>
#include <RMF/FileHandle.h>
#include <RMF/SetCurrentFrame.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE
typedef base::Pointer<SaveLink> SaveLinkAssociationType;
typedef base::Pointer<LoadLink> LoadLinkAssociationType;

IMPRMFEXPORT unsigned int get_load_linker_index(std::string st);
IMPRMFEXPORT unsigned int get_save_linker_index(std::string st);

IMPRMFEXPORT LoadLinks get_load_linkers(RMF::FileConstHandle fh);

IMPRMFEXPORT SaveLinks get_save_linkers(RMF::FileHandle fh);

IMPRMFEXPORT bool get_has_linker(RMF::FileConstHandle nh, unsigned int id);

IMPRMFEXPORT void set_linker(RMF::FileConstHandle nh, unsigned int id,
                             SaveLinkAssociationType l);

IMPRMFEXPORT void set_linker(RMF::FileConstHandle nh, unsigned int id,
                             LoadLinkAssociationType l);

IMPRMFEXPORT SaveLinkAssociationType
    get_save_linker(RMF::FileConstHandle nh, unsigned int id);

IMPRMFEXPORT LoadLinkAssociationType
    get_load_linker(RMF::FileConstHandle nh, unsigned int id);

template <class LinkType>
inline LinkType* get_save_link(RMF::FileHandle fh) {
  int index = get_save_linker_index(LinkType::get_name());
  if (!fh.get_has_associated_data(index)) {
    SaveLinkAssociationType psl = new LinkType(fh);
    set_linker(fh, index, psl);
  }
  SaveLinkAssociationType ln = get_save_linker(fh, index);
  return dynamic_cast<LinkType*>(ln.get());
}

template <class LinkType>
inline LinkType* get_load_link(RMF::FileConstHandle fh) {
  int index = get_load_linker_index(LinkType::get_name());
  if (!fh.get_has_associated_data(index)) {
    LoadLinkAssociationType psl = new LinkType(fh);
    set_linker(fh, index, psl);
  }
  LoadLinkAssociationType pt = get_load_linker(fh, index);
  return dynamic_cast<LinkType*>(pt.get());
}

template <class LinkType, class OutTypes>
void add_helper(RMF::FileHandle fh, const OutTypes& hs) {
  if (hs.empty()) return;
  LinkType* hsl = get_save_link<LinkType>(fh);
  RMF::SetCurrentFrame scf(fh, RMF::ALL_FRAMES);
  hsl->add(fh.get_root_node(), hs);
}

template <class LinkType, class InTypes>
void link_helper(RMF::FileConstHandle fh, const InTypes& hs) {
  if (hs.empty()) return;
  base::Pointer<LinkType> pll = get_load_link<LinkType>(fh);
  RMF::SetCurrentFrame scf(fh, RMF::ALL_FRAMES);
  pll->link(fh.get_root_node(), hs);
}

template <class LinkType, class Result>
Result create_helper(RMF::FileConstHandle fh) {
  LinkType* rsl = get_load_link<LinkType>(fh);
  /* to catch the type of everything.*/
  RMF::SetCurrentFrame scf(fh, RMF::FrameID(0));
  Result ret = rsl->create(fh.get_root_node());
  rsl->load(fh);
  return ret;
}

template <class LinkType, class Result, class Arg>
Result create_helper(RMF::FileConstHandle fh, const Arg& arg) {
  LinkType* rsl = get_load_link<LinkType>(fh);
  /* to catch the type of everything.*/
  RMF::SetCurrentFrame scf(fh, RMF::FrameID(0));
  Result ret = rsl->create(fh.get_root_node(), arg);
  rsl->load(fh);
  return ret;
}

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_LINK_HELPERS_H */
