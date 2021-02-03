/**
 *  \file coarse_molecule.cpp
 *  \brief coarsen molecule by clustering
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/VQClustering.h>
#include <IMP/statistics/internal/DataPoints.h>
#include <IMP/multifit/DensityDataPoints.h>
#include <IMP/multifit/DataPointsAssignment.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/multifit/coarse_molecule.h>

IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
atom::Hierarchy create_molecule(const algebra::Vector3Ds vecs,
                                float bead_radius, float bead_mass,
                                Model *mdl) {
  atom::Hierarchy ret_prot =
      atom::Hierarchy::setup_particle(new Particle(mdl));
  for (unsigned int i = 0; i < vecs.size(); i++) {
    core::XYZR bead_child = core::XYZR::setup_particle(
        new Particle(mdl), algebra::Sphere3D(vecs[i], bead_radius));
    atom::Residue residue_child =
        atom::Residue::setup_particle(new Particle(mdl), atom::ALA, i);
    atom::Hierarchy::setup_particle(bead_child);
    atom::Atom::setup_particle(bead_child, atom::AT_CA);
    atom::Mass(bead_child).set_mass(bead_mass);
    // todo - mass should be calculated
    residue_child.add_child(atom::Hierarchy(bead_child));
    ret_prot.add_child(atom::Hierarchy(residue_child));
  }
  return ret_prot;
}
}
atom::Hierarchy create_coarse_molecule_from_density(em::DensityMap *dmap,
                                                    float dens_threshold,
                                                    int num_beads,
                                                    Model *mdl,
                                                    float bead_radius) {

  IMP_NEW(DensityDataPoints, ddp, (dmap, dens_threshold));
  IMP_LOG_VERBOSE("initialize calculation of initial centers" << std::endl);
  IMP::statistics::internal::VQClustering vq(ddp, num_beads);
  vq.run();
  multifit::DataPointsAssignment assignment(ddp, &vq);
  algebra::Vector3Ds vecs;
  for (int i = 0; i < num_beads; i++) {
    IMP::statistics::internal::Array1DD xyz =
        assignment.get_cluster_engine()->get_center(i);
    vecs.push_back(algebra::Vector3D(xyz[0], xyz[1], xyz[2]));
  }
  // todo - mass should be a parameter
  atom::Hierarchy ret_prot = create_molecule(vecs, bead_radius, 3, mdl);
  return ret_prot;
}

IMPMULTIFIT_END_NAMESPACE
