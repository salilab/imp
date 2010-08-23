/**
 *  \file model_interaction.cpp
 *  \brief operations implying interaction of models and the EM module
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2D/model_interaction.h"
#include "IMP/em/MRCReaderWriter.h"
#include "IMP/atom/Atom.h"
#include "IMP/atom/force_fields.h" // add_radii
#include "IMP/atom/pdb.h"
#include "IMP/atom/element.h"

IMPEM2D_BEGIN_NAMESPACE

Pointer<em::SampledDensityMap> generate_map_from_model(atom::Hierarchy &mh,
                            double resolution,
                            double voxelsize) {
  atom::add_radii(mh); // Adds radii to particles
  Particles ps = IMP::core::get_leaves(mh);
  Pointer<em::SampledDensityMap> map =
          new em::SampledDensityMap(ps,resolution,voxelsize);
//  map->get_header_writable()->set_spacing(voxelsize);
  return map;
}

void generate_map_from_model(String fn_model,
                             String fn_map,
                            double resolution,
                            double voxelsize,
                            em::MapReaderWriter &mrw) {
  IMP_NEW(Model, model, ());
  atom::ATOMPDBSelector sel;

  atom::Hierarchy mh = atom::read_pdb(fn_model,model,sel,true);
  Pointer<em::SampledDensityMap> map =
                    generate_map_from_model(mh,resolution,voxelsize);
  em::write_map(map,fn_map.c_str(),mrw);
}


void Vector3Ds_to_pdb(const algebra::Vector3Ds vs, const String filename) {
  std::string chains = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::ofstream out;
  out.open(filename.c_str(),std::ios::out);
  for( unsigned long i=0;i<vs.size();i++) {
    int j=i/10000;
    char chain = chains[j];
    int residue = i%10000;
    out << pdb_string(vs[i],i,atom::AT_CA,atom::ALA,chain,residue,' ',atom::C);
  }
  out.close();
}


void Vector2Ds_to_pdb(const algebra::Vector2Ds vs,const  String filename) {
  std::string chains = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::ofstream out;
  out.open(filename.c_str(),std::ios::out);
  for( unsigned long i=0;i<vs.size();i++) {
    int j=i/10000;
    char chain = chains[j];
    int residue = i%10000;
    out << pdb_string(algebra::Vector3D(vs[i][0],vs[i][1],0.0),
            i,atom::AT_CA,atom::ALA,chain,residue,' ',atom::C);
  }
  out.close();
}


IMPEM2D_END_NAMESPACE
