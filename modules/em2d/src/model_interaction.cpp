/**
 *  \file model_interaction.cpp
 *  \brief operations implying interaction of models and the EM module
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/model_interaction.h"
#include "IMP/em2d/filenames_manipulation.h"
#include "IMP/em/MapReaderWriter.h"
#include "IMP/atom/Atom.h"
#include "IMP/atom/force_fields.h" // add_radii
#include "IMP/atom/pdb.h"
#include "IMP/atom/element.h"
#include "IMP/Pointer.h"

IMPEM2D_BEGIN_NAMESPACE

/** OBSOLETE
Pointer<em::SampledDensityMap> get_map_from_model(const Particles &ps,
                            double resolution,
                            double voxelsize) {
  Pointer<em::SampledDensityMap> map =
          new em::SampledDensityMap(ps,resolution,voxelsize);
//  map->get_header_writable()->set_spacing(voxelsize);
  return map;
}
**/
/** OBSOLETE
void get_map_from_model(String fn_model,
                             String fn_map,
                            double resolution,
                            double voxelsize,
                            em::MapReaderWriter &mrw) {
  IMP_NEW(Model, model, ());
  IMP::Pointer<atom::ATOMPDBSelector> sel= new atom::ATOMPDBSelector();

  atom::Hierarchy mh = atom::read_pdb(fn_model,model,sel,true);
  atom::add_radii(mh); // Adds radii to particles
  Particles ps = IMP::core::get_leaves(mh);
  Pointer<em::SampledDensityMap> map =
                    get_map_from_model(ps,resolution,voxelsize);
  em::write_map(map,fn_map.c_str(),mrw);
}
**/

void write_vector3Ds_as_pdb(const algebra::Vector3Ds vs,
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

void write_vector2Ds_as_pdb(const algebra::Vector2Ds vs,
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


atom::Hierarchies read_multiple_pdbs(const String &selection_file,
                               Model* model,
                               atom::PDBSelector* selector,
                               bool select_first_model,
                               bool no_radii) {
  Strings fn_pdbs=read_selection_file(selection_file);
  atom::Hierarchies mhs(fn_pdbs.size());
  for (unsigned int i=0;i<fn_pdbs.size();++i) {
    TextInput fn_pdb(fn_pdbs[i]);
    mhs[i]=atom::read_pdb(fn_pdbs[i],
                          model,
                          selector,
                          select_first_model,
                          no_radii);
  }
  return mhs;
};

IMPEM2D_END_NAMESPACE
