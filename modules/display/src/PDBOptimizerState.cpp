/**
 *  \file PDBOptimizerState.cpp
 *  \brief Write a set of molecules as a pdb file.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/PDBOptimizerState.h"
#include <IMP/atom/pdb.h>


IMPDISPLAY_BEGIN_NAMESPACE

PDBOptimizerState::PDBOptimizerState(
  const atom::Hierarchies &mh, std::string name ):
  name_template_(name),  mh_(mh){
}

void PDBOptimizerState::show(std::ostream &out) const {
  out << "PDBOptimizerState" << std::endl;
}

void PDBOptimizerState::do_update(unsigned int n) {
  char buf[1000];
  sprintf(buf, name_template_.c_str(), n);
  IMP_LOG(TERSE, "Writing file " << buf << std::endl);
  atom::write_pdb(mh_,buf);
}
IMPDISPLAY_END_NAMESPACE
