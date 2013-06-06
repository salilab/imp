/**
 *  \file model_interaction.cpp
 *  \brief operations implying interaction of models and the EM module
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/model_interaction.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em/MapReaderWriter.h"
#include "IMP/atom/Atom.h"
#include "IMP/atom/force_fields.h" // add_radii
#include "IMP/atom/pdb.h"
#include "IMP/atom/element.h"
#include "IMP/base/Pointer.h"

IMPEM2D_BEGIN_NAMESPACE

void write_vectors_as_pdb(const algebra::Vector3Ds vs,
                            const String filename) {
  std::string chains = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::ofstream out;
  out.open(filename.c_str(),std::ios::out);
  for( unsigned long i=0;i<vs.size();i++) {
    int j=i/10000;
    char chain = chains[j];
    int residue = i%10000;
    out << atom::get_pdb_string(vs[i],i,
                                atom::AT_CA,atom::ALA,chain,residue);
  }
  out.close();
}

void write_vectors_as_pdb(const algebra::Vector2Ds vs,
                            const  String filename) {
  std::string chains = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::ofstream out;
  out.open(filename.c_str(),std::ios::out);
  for( unsigned long i=0;i<vs.size();i++) {
    int j=i/10000;
    char chain = chains[j];
    int residue = i%10000;
    out << atom::get_pdb_string(algebra::Vector3D(vs[i][0],vs[i][1],0.0),
            i,atom::AT_CA,atom::ALA,chain,residue);
  }
  out.close();
}


IMPEM2D_END_NAMESPACE
