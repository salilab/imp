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
#include <IMP/flags.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

int main(int argc, char* argv[]) {
  IMP::setup_from_argv(argc, argv, "Test serialize");
  IMP_NEW(IMP::core::Cosine, uf, (0., 1, 0.));
  IMP_NEW(IMP::core::AngleTripletScore, ats, (uf));

  std::ostringstream oss;
  boost::archive::binary_oarchive ba(oss, boost::archive::no_header);
  ba << *ats;
  std::string s = oss.str();
  std::cerr << "serialize done, written " << s.size() << " bytes" << std::endl;

  std::istringstream iss(s);
  boost::archive::binary_iarchive iba(iss, boost::archive::no_header);
  IMP_NEW(IMP::core::AngleTripletScore, newats, ());
  iba >> *newats;
  std::cerr << "deserialize done, read in " << *newats << std::endl;
  return 0;
}
