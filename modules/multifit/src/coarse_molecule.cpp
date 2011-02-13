/**
 *  \file coase_molecule.cpp
 *  \brief coarsen molecule by clustering
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/VQClustering.h>
#include <IMP/multifit/DataPoints.h>
#include <IMP/multifit/DataPointsAssignment.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/multifit/coarse_molecule.h>

IMPMULTIFIT_BEGIN_NAMESPACE

atom::Hierarchy create_coarse_molecule_from_molecule(
               const atom::Hierarchy &mh,int num_beads,
               Model *mdl,
               float bead_radius,
               bool add_conn_restraint) {
  multifit::ParticlesDataPoints ddp(core::get_leaves(mh));
  multifit::VQClustering vq(&ddp,num_beads);
  vq.run();
  multifit::DataPointsAssignment assignment(&ddp,&vq);
  atom::Hierarchy ret_prot=
    atom::Hierarchy::setup_particle(new Particle(mdl));
  atom::Selections sel;
  for (int i=0;i<num_beads;i++){
    Array1DD xyz = assignment.get_cluster_engine()->get_center(i);
    core::XYZR bead_child=core::XYZR::setup_particle(
             new Particle(mdl),
             algebra::Sphere3D(algebra::Vector3D(xyz[0],
                                                 xyz[1],
                                                 xyz[2]),
                               bead_radius));
    atom::Mass::setup_particle(bead_child,3);
    atom::Hierarchy::setup_particle(bead_child);
    atom::Atom::setup_particle(bead_child,atom::AT_CA);
    ret_prot.add_child(atom::Hierarchy(bead_child));
    sel.push_back(atom::Selection(atom::Hierarchy(bead_child)));
  }
  if (add_conn_restraint){
  int k=1;//todo - make this a parameter
  Restraint *r = atom::create_connectivity_restraint(sel,k);
  if (r != NULL){
    mdl->add_restraint(r);}
  }
  return ret_prot;
}
atom::Hierarchies create_coarse_molecules_from_molecules(
       const atom::Hierarchies &mhs,
       int frag_len,
       Model *mdl,
       float bead_radius,
       bool add_conn_restraint){
  atom::Hierarchies ret;
  for(int i=0;i<(int)mhs.size();i++) {
    //decide the number of beads for the molecule
    int num_beads = std::max(1,
        (int)(atom::get_by_type(mhs[i],atom::RESIDUE_TYPE).size())/frag_len);
    ret.push_back(create_coarse_molecule_from_molecule(
                    mhs[i],num_beads,
                    mdl,bead_radius,add_conn_restraint));
  }
  return ret;
}
atom::Hierarchy create_coarse_molecule_from_density(
    em::DensityMap* dmap,
    float dens_threshold,
    int num_beads,
    Model *mdl,
    float bead_radius){

  multifit::DensityDataPoints ddp(dmap,dens_threshold);
  IMP_LOG(VERBOSE,"initialize calculation of initial centers"<<std::endl);
  multifit::VQClustering vq(&ddp,num_beads);
  vq.run();
  multifit::DataPointsAssignment assignment(&ddp,&vq);
  atom::Hierarchy ret_prot=
    atom::Hierarchy::setup_particle(new Particle(mdl));
  atom::Selections sel;
  for (int i=0;i<num_beads;i++){
    Array1DD xyz = assignment.get_cluster_engine()->get_center(i);
    core::XYZR bead_child=core::XYZR::setup_particle(
             new Particle(mdl),
             algebra::Sphere3D(algebra::Vector3D(xyz[0],
                                                 xyz[1],
                                                 xyz[2]),
                               bead_radius));
    atom::Mass::setup_particle(bead_child,3);
    atom::Hierarchy::setup_particle(bead_child);
    atom::Atom::setup_particle(bead_child,atom::AT_CA);
    ret_prot.add_child(atom::Hierarchy(bead_child));
    sel.push_back(atom::Selection(atom::Hierarchy(bead_child)));
  }
  return ret_prot;
}

IMPMULTIFIT_END_NAMESPACE
