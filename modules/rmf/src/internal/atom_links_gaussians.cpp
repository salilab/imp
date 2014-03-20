/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_gaussians.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <IMP/core/Gaussian.h>
#include <IMP/core/rigid_bodies.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadGaussians::HierarchyLoadGaussians(RMF::FileConstHandle f)
    : gaussian_factory_(f) {}

void HierarchyLoadGaussians::setup_particle(
    RMF::NodeConstHandle n, kernel::Model *m, kernel::ParticleIndex p,
    const kernel::ParticleIndexes &rigid_bodies) {
  if (!gaussian_factory_.get_is(n)) return;
  if (!core::Gaussian::get_is_setup(m, p)) core::Gaussian::setup_particle(m, p);
  if (gaussian_factory_.get_is_static(n)) {
    RMF::Vector3 v = gaussian_factory_.get(n).get_variances();
    core::Gaussian(m, p).set_variances(algebra::Vector3D(v));
  }
  link_particle(n, m, p, rigid_bodies);
}

void HierarchyLoadGaussians::link_particle(RMF::NodeConstHandle n,
                                           kernel::Model *,
                                           kernel::ParticleIndex p,
                                           const kernel::ParticleIndexes &) {
  if (!gaussian_factory_.get_is_static(n)) return;
  gaussians_.push_back(Pair(n, p));
}

void HierarchyLoadGaussians::load(RMF::FileConstHandle fh, Model *m) {
  IMP_FOREACH(Pair pp, gaussians_) {
    IMP_LOG_VERBOSE("Loading global coordinates for "
                    << m->get_particle_name(pp.second) << std::endl);
    RMF::Vector3 sd =
        gaussian_factory_.get(fh.get_node(pp.first)).get_variances();
    core::Gaussian(m, pp.second)
        .set_variances(algebra::Vector3D(sd.begin(), sd.end()));
  }
}

HierarchySaveGaussians::HierarchySaveGaussians(RMF::FileHandle f)
    : gaussian_factory_(f) {}

void HierarchySaveGaussians::setup_node(kernel::Model *m,
                                        kernel::ParticleIndex p,
                                        RMF::NodeHandle n,
                                        const kernel::ParticleIndexes &) {
  if (!core::Gaussian::get_is_setup(m, p)) return;
  algebra::Vector3D st = core::Gaussian(m, p).get_gaussian().get_variances();
  gaussian_factory_.get(n).set_variances(RMF::Vector3(st));
  gaussians_.push_back(Pair(n, p));
}

void HierarchySaveGaussians::save(kernel::Model *m, RMF::FileHandle fh) {
  IMP_FOREACH(Pair pp, gaussians_) {
    gaussian_factory_.get(fh.get_node(pp.first))
        .set_frame_variances(RMF::Vector3(
             core::Gaussian(m, pp.second).get_gaussian().get_variances()));
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
