/**
 *  \file test_serialize.cpp
 *  \brief Test of serialize/deserialize functionality
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */
#include <sstream>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/core/Cosine.h>
#include <IMP/core/AngleTripletScore.h>
#include <IMP/internal/swig.h>
#include <IMP/core/RestraintsScoringFunction.h>
#include <IMP/core/Gaussian.h>
#include <IMP/flags.h>
#include <cereal/archives/binary.hpp>

namespace {

void test_triplet_score() {
  IMP_NEW(IMP::core::Cosine, uf, (0., 1, 0.));
  IMP_NEW(IMP::core::AngleTripletScore, ats, (uf));

  std::ostringstream oss;
  cereal::BinaryOutputArchive ba(oss);
  ba(*ats);
  std::string s = oss.str();
  std::cerr << "serialize done, written " << s.size() << " bytes" << std::endl;

  std::istringstream iss(s);
  cereal::BinaryInputArchive iba(iss);
  IMP_NEW(IMP::core::AngleTripletScore, newats, ());
  iba(*newats);
  std::cerr << "deserialize done, read in " << *newats << std::endl;
}

void test_scoring_function() {
  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::Particle, p, (m));
  IMP::ParticleIndexes pis;
  pis.push_back(p->get_index());
  IMP_NEW(IMP::internal::_ConstRestraint, cr, (m, pis, 1.0));
  IMP::RestraintsTemp rt;
  rt.push_back(cr);
  IMP_NEW(IMP::core::RestraintsScoringFunction, sf, (rt));

  std::ostringstream oss;
  cereal::BinaryOutputArchive ba(oss);
  ba(*sf);
  std::string s = oss.str();
  std::cerr << "serialize done, written " << s.size() << " bytes" << std::endl;

  std::istringstream iss(s);
  cereal::BinaryInputArchive iba(iss);
  IMP_NEW(IMP::core::RestraintsScoringFunction, newsf, ());
  iba(*newsf);
  std::cerr << "deserialize done, read in " << *newsf << std::endl;
}

void test_optimizer() {
  IMP_NEW(IMP::Model, m, ());
  IMP_NEW(IMP::Particle, p, (m));
  IMP::ParticleIndexes pis;
  pis.push_back(p->get_index());
  IMP_NEW(IMP::internal::_ConstRestraint, cr, (m, pis, 1.0));
  IMP::RestraintsTemp rt;
  rt.push_back(cr);
  IMP_NEW(IMP::core::RestraintsScoringFunction, sf, (rt));
  IMP_NEW(IMP::internal::_ConstOptimizer, opt, (m));
  opt->set_scoring_function(sf);

  std::ostringstream oss;
  cereal::BinaryOutputArchive ba(oss);
  ba(*opt);
  std::string s = oss.str();
  std::cerr << "serialize done, written " << s.size() << " bytes" << std::endl;

  std::istringstream iss(s);
  cereal::BinaryInputArchive iba(iss);
  IMP_NEW(IMP::internal::_ConstOptimizer, newopt, ());
  iba(*newopt);
  std::cerr << "deserialize done, read in " << *newopt << std::endl;
}

void test_matrix3d() {
  Eigen::Matrix3d mat;
  mat(0, 1) = 1;
  mat(0, 2) = 2;
  IMP_NEW(IMP::core::Matrix3D, m, (mat));

  std::ostringstream oss;
  cereal::BinaryOutputArchive ba(oss);
  ba(*m);
  std::string s = oss.str();
  std::cerr << "serialize done, written " << s.size() << " bytes" << std::endl;

  std::istringstream iss(s);
  cereal::BinaryInputArchive iba(iss);
  IMP_NEW(IMP::core::Matrix3D, newm, ());
  iba(*newm);
  std::cerr << "deserialize done, read in " << *newm << std::endl;

  assert(int(newm->get_mat()(0, 1)) == 1);
  assert(int(newm->get_mat()(0, 2)) == 2);
}

} // namespace

int main(int argc, char* argv[]) {
  IMP::setup_from_argv(argc, argv, "Test serialize");
  test_triplet_score();
  test_scoring_function();
  test_optimizer();
  test_matrix3d();
  return 0;
}
