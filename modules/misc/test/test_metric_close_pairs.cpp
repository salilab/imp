/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/misc/MetricClosePairsFinder.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <algorithm>
#include <IMP/flags.h>
#include <IMP/test/test_macros.h>

namespace {
struct LowerBound {
  double operator()(IMP::Model *m,
                    const IMP::ParticleIndexPair &pip) const {
    return IMP::core::get_distance(IMP::core::XYZR(m, pip[0]),
                                   IMP::core::XYZR(m, pip[1]));
  }
};

struct UpperBound {
  double operator()(IMP::Model *m,
                    const IMP::ParticleIndexPair &pip) const {
    return IMP::core::get_distance(IMP::core::XYZ(m, pip[0]),
                                   IMP::core::XYZ(m, pip[1])) +
           IMP::core::XYZR(m, pip[0]).get_radius() +
           IMP::core::XYZR(m, pip[1]).get_radius();
  }
};

void canonicalize(IMP::ParticleIndexPairs &pip) {
  for (unsigned int i = 0; i < pip.size(); ++i) {
    if (pip[i][0] > pip[i][1]) {
      pip[i] = IMP::ParticleIndexPair(pip[i][1], pip[i][0]);
    }
  }
}
}

boost::int64_t num_particles = 10;
IMP::AddIntFlag npf("number_of_particles",
                          "The number of particles to use", &num_particles);
double radius = .1;
IMP::AddFloatFlag rpf("radius", "The radius", &radius);

int main(int argc, char *argv[]) {
  IMP::setup_from_argv(argc, argv, "Test of base caches in C++");
  IMP_NEW(IMP::Model, m, ());
  IMP::ParticleIndexes pis;
  IMP::algebra::BoundingBox3D bb = IMP::algebra::get_unit_bounding_box_d<3>();
  for (unsigned int i = 0; i < num_particles; ++i) {
    pis.push_back(m->add_particle("P%1%"));
    IMP::core::XYZR::setup_particle(
        m, pis.back(),
        IMP::algebra::Sphere3D(IMP::algebra::get_random_vector_in(bb), radius));
  }
  IMP_NEW(IMP::core::GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(.1);
  IMP::PointerMember<IMP::core::ClosePairsFinder> mcpf =
      IMP::misc::create_metric_close_pairs_finder(LowerBound(), UpperBound());
  mcpf->set_distance(.1);

  IMP::ParticleIndexPairs gcp = gcpf->get_close_pairs(m, pis);
  canonicalize(gcp);
  IMP::ParticleIndexPairs mcp = mcpf->get_close_pairs(m, pis);
  canonicalize(mcp);
  std::sort(gcp.begin(), gcp.end());
  std::sort(mcp.begin(), mcp.end());
  std::cout << "Lists are " << gcp << " and " << mcp << std::endl;
  IMP::ParticleIndexPairs out;
  std::set_intersection(gcp.begin(), gcp.end(), mcp.begin(), mcp.end(),
                        std::back_inserter(out));
  IMP_TEST_EQUAL(out.size(), mcp.size());
  IMP_TEST_EQUAL(out.size(), gcp.size());
  return 0;
}
