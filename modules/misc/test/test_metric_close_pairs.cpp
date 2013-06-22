/**
 *  \file test_cache.cpp
 *  \brief A nullptr-initialized pointer to an \imp Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/misc/MetricClosePairsFinder.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <IMP/base/flags.h>
#include <IMP/test/test_macros.h>

namespace {
struct LowerBound {
  double operator()(IMP::kernel::Model *m,
                    const IMP::kernel::ParticleIndexPair &pip) const {
    return IMP::core::get_distance(IMP::core::XYZR(m, pip[0]),
                                   IMP::core::XYZR(m, pip[1]));
  }
};

struct UpperBound {
  double operator()(IMP::kernel::Model *m,
                    const IMP::kernel::ParticleIndexPair &pip) const {
    return IMP::core::get_distance(IMP::core::XYZ(m, pip[0]),
                                   IMP::core::XYZ(m, pip[1]))
      + IMP::core::XYZR(m, pip[0]).get_radius()
      + IMP::core::XYZR(m, pip[1]).get_radius();
  }
};

  void canonicalize(IMP::kernel::ParticleIndexPairs &pip) {
    for (unsigned int i = 0; i< pip.size(); ++i) {
      if (pip[i][0] > pip[i][1]) {
        pip[i] = IMP::kernel::ParticleIndexPair(pip[i][1], pip[i][0]);
      }
    }
  }
}

boost::int64_t num_particles = 10;
IMP::base::AddIntFlag npf("number_of_particles",
                          "The number of particles to use",
                      &num_particles);
double radius = .1;
IMP::base::AddFloatFlag rpf("radius",
                          "The radius",
                      &radius);

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "Test of base caches in C++");
  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::kernel::ParticleIndexes pis;
  IMP::algebra::BoundingBox3D bb = IMP::algebra::get_unit_bounding_box_d<3>();
  for (unsigned int i = 0; i < num_particles; ++i) {
    pis.push_back(m->add_particle("P%1%"));
    IMP::core::XYZR::setup_particle(m, pis.back(),
            IMP::algebra::Sphere3D(IMP::algebra::get_random_vector_in(bb),
                                radius));
  }
  IMP_NEW(IMP::core::GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(.1);
  IMP::base::OwnerPointer<IMP::core::ClosePairsFinder> mcpf
    = IMP::misc::create_metric_close_pairs_finder(LowerBound(), UpperBound());
  mcpf->set_distance(.1);

  IMP::kernel::ParticleIndexPairs gcp = gcpf->get_close_pairs(m, pis);
  canonicalize(gcp);
  IMP::kernel::ParticleIndexPairs mcp = mcpf->get_close_pairs(m, pis);
  canonicalize(mcp);
  std::sort(gcp.begin(), gcp.end());
  std::sort(mcp.begin(), mcp.end());
  std::cout << "Lists are " << gcp << " and " << mcp << std::endl;
  IMP::kernel::ParticleIndexPairs out;
  boost::set_intersection(gcp, mcp, std::back_inserter(out));
  IMP_TEST_EQUAL(out.size(), mcp.size());
  IMP_TEST_EQUAL(out.size(), gcp.size());
  return 0;
}
