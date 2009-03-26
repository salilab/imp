/**
 *  \file PDBOptimizerState.cpp
 *  \brief Write a set of molecules as a pdb file.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/PDBOptimizerState.h"
#include <IMP/atom/pdb.h>


IMPDISPLAY_BEGIN_NAMESPACE

PDBOptimizerState::PDBOptimizerState(
  const atom::MolecularHierarchyDecorators &mh, std::string name ):
  mh_(mh), step_(0), skip_steps_(0), name_template_(name){
}

PDBOptimizerState::~PDBOptimizerState(){}

void PDBOptimizerState::show(std::ostream &out) const {
  out << "PDBOptimizerState" << std::endl;
}

void PDBOptimizerState::update() {
  ++step_;
  if (step_%(skip_steps_+1)==0) {
    unsigned int n= step_;
    n/= (skip_steps_+1);
    --n;
    char buf[1000];
    sprintf(buf, name_template_.c_str(), n);
    IMP_LOG(TERSE, "Writing file " << buf << std::endl);
    atom::write_pdb(mh_,buf);
  }
}
IMPDISPLAY_END_NAMESPACE
