/**
 *  \file IMP/rmf/link_macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/rmf/internal/link_helpers.h>
#include <boost/foreach.hpp>

IMPRMF_BEGIN_INTERNAL_NAMESPACE
namespace {
  base::map<std::string, int> known_load_linkers, known_save_linkers;
}

unsigned int get_load_linker_index(std::string st) {
  if (known_load_linkers.find(st) == known_load_linkers.end()) {
    int cur = known_load_linkers.size() + known_save_linkers.size();
    known_load_linkers[st] = cur;
    return cur;
  } else {
    return known_load_linkers.find(st)->second;
  }
}

unsigned int get_save_linker_index(std::string st) {
  if (known_save_linkers.find(st) == known_save_linkers.end()) {
    int cur = known_load_linkers.size() + known_save_linkers.size();
    known_save_linkers[st] = cur;
    return cur;
  } else {
    return known_save_linkers.find(st)->second;
  }
}



LoadLinks get_load_linkers(RMF::FileConstHandle fh) {
  LoadLinks ret;
  typedef std::pair<std::string, int> P;
  BOOST_FOREACH(P kl, known_load_linkers) {
    if (fh.get_has_associated_data(kl.second)) {
      ret.push_back(fh.get_associated_data<LoadLinkAssociationType>(kl.second));
    }
  }
  IMP_LOG_VERBOSE("Found " << ret.size() << " load linkers" << std::endl);
  return ret;
}

SaveLinks get_save_linkers(RMF::FileHandle fh) {
  SaveLinks ret;
  typedef std::pair<std::string, int> P;
  BOOST_FOREACH(P kl, known_save_linkers) {
    if (fh.get_has_associated_data(kl.second)) {
      ret.push_back(fh.get_associated_data<SaveLinkAssociationType>(kl.second));
    }
  }
  IMP_LOG_VERBOSE("Found " << ret.size() << " save linkers" << std::endl);
  return ret;
}


bool get_has_linker(RMF::FileConstHandle fh, unsigned int id) {
  return fh.get_has_associated_data(id);
}

void set_linker(RMF::FileConstHandle fh, unsigned int id,
                SaveLinkAssociationType l) {
  fh.add_associated_data(id, l);
}

void set_linker(RMF::FileConstHandle fh, unsigned int id,
                LoadLinkAssociationType l) {
  fh.add_associated_data(id, l);
}

SaveLinkAssociationType get_save_linker(RMF::FileConstHandle fh,
                                        unsigned int id) {
  return fh.get_associated_data<SaveLinkAssociationType>(id);
}

LoadLinkAssociationType get_load_linker(RMF::FileConstHandle fh,
                                        unsigned int id) {
  return fh.get_associated_data<LoadLinkAssociationType>(id);
}

IMPRMF_END_INTERNAL_NAMESPACE
