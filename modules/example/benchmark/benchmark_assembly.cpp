/**
 * \file benchmark_assembly.cpp for silly standards test
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/example.h>
#include <IMP/benchmark.h>
#include <IMP/algebra.h>
#include <IMP/display.h>
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/container.h>

using namespace IMP;
using namespace algebra;
using namespace core;
using namespace container;
using namespace example;
using namespace display;
using namespace atom;

namespace {
double threshold=10;

Sphere3Ds get_residues(std::string fname) {
  IMP_NEW(Model, m, ());
  atom::Hierarchy h= read_pdb(fname, m);
  atom::Hierarchy hs
    = create_simplified_along_backbone(Chain(get_by_type(h, CHAIN_TYPE)[0]),
                                       1);
  ParticlesTemp ps= get_as<ParticlesTemp>(atom::get_leaves(hs));
  Sphere3Ds ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    ret.push_back(XYZR(ps[i]).get_sphere());
  }
  return ret;
}


std::pair<Restraints, PairPredicates>
create_restraints(Model *m,
                       const ParticlesTemp &ps,
                       double threshold) {
  IMP_NEW(GridClosePairsFinder, cpf, ());
  cpf->set_distance(threshold);
  ParticlePairsTemp close= cpf->get_close_pairs(ps);
  Restraints ret;
  for (unsigned int i=0; i< close.size(); ++i) {
    double d= get_distance(XYZ(close[i][0]),
                           XYZ(close[i][1]));
    IMP_NEW(HarmonicDistancePairScore, hdps, (d, 10));
    Pointer<Restraint> r
      = IMP::create_restraint(hdps.get(), close[i]);
    m->add_restraint(r);
    ret.push_back(r);
  }
  IMP_NEW(ListPairContainer, lpc, (close));
  Pointer<PairPredicate> filter
    =new container::InContainerPairFilter(lpc.get());
  return std::make_pair(ret, PairPredicates(1, filter));
}

ParticlesTemp create_particles(Model *m, const Sphere3Ds &input) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< input.size(); ++i) {
    IMP_NEW(Particle, p, (m));
    std::ostringstream oss;
    oss << "P" << i;
    p->set_name(oss.str());
    XYZR::setup_particle(p, input[i]);
    XYZ(p).set_coordinates_are_optimized(true);
    ret.push_back(p);
  }
  return ret;
}

void display_model(const Sphere3Ds &input,
                   const ParticlesTemp &output,
                   const Restraints &rs,
                   std::string fname) {
  IMP_NEW(PymolWriter, w, (fname));
  for (unsigned int i=0; i< input.size(); ++i) {
    double f= static_cast<double>(i)/input.size();
    IMP_NEW(SphereGeometry, sg, (input[i]));
    sg->set_name("input");
    sg->set_color(get_jet_color(f));
    w->add_geometry(sg);
  }
  for (unsigned int i=0; i< output.size(); ++i) {
    double f= static_cast<double>(i)/output.size();
    IMP_NEW(SphereGeometry, sg, (XYZR(output[i]).get_sphere()));
    sg->set_name("output");
    sg->set_color(get_jet_color(f));
    w->add_geometry(sg);
  }
  for (unsigned int i=0; i< rs.size(); ++i) {
    IMP_NEW(RestraintGeometry, rg, (rs[i]));
    rg->set_name("restraint");
    w->add_geometry(rg);
  }
}

void align_input_and_output(const ParticlesTemp &output,
                            const Sphere3Ds &input) {
  Transformation3D tr= get_transformation_aligning_first_to_second(output,
                                                                   input);
  std::cout << "Transform " << tr << std::endl;
  for (unsigned int i=0; i< output.size(); ++i) {
    XYZ(output[i])
      .set_coordinates(tr.get_transformed(XYZ(output[i]).get_coordinates()));
  }
  std::cout << "RMSD " << get_rmsd(input, output) << std::endl;
}
}


int main(int, char *[]) {
  // not finished
  return 0;
  Sphere3Ds s3= get_residues(benchmark::get_data_path("small_protein.pdb"));

  BoundingBox3D bb;
  for (unsigned int i=0; i< s3.size(); ++i) {
    bb+= get_bounding_box(s3[i]);
  }

  IMP_NEW(Model, m, ());
  ParticlesTemp ps= create_particles(m, s3);
  Restraints rs;
  PairPredicates interactions;
  boost::tie(rs, interactions)= create_restraints(m, ps, threshold);
  display_model(s3, ps, rs, "in.pym");

  IMP_NEW(SoftSpherePairScore, ssps, (10));
  IMP_IF_CHECK(USAGE) {
    double tot=0;
    for (unsigned int i=0; i< rs.size(); ++i) {
      double cur=rs[i]->evaluate(false);
      IMP_USAGE_CHECK(cur < .01, "Huh? " << cur );
      tot+=cur;
    }
    for (unsigned int i=0; i< ps.size(); ++i) {
      for (unsigned int j=0; j < i; ++j) {
        ParticlePair pp(ps[i], ps[j]);
        if (!interactions[0]->get_value(pp)) {
          double cur= ssps->evaluate(pp, nullptr);
          IMP_USAGE_CHECK(cur < .01, "Huf? " << cur);
          tot+=cur;
        }
      }
    }
    std::cout << tot << std::endl;
  }
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZ(ps[i]).set_coordinates(get_zero_vector_d<3>());
  }
  try {
    example::optimize_assembly(m, ps, get_as<RestraintsTemp>(rs),
                               RestraintsTemp(),
                      bb+20, ssps, 20000*s3.size(),
                      interactions);
  } catch (Exception e) {
    std::cerr << e.what() << std::endl;
    //align_input_and_output(ps, s3);
    display_model(s3, ps, rs, "done.pym");
    return 1;
  }
  align_input_and_output(ps, s3);
  display_model(s3, ps, rs, "done.pym");
  double tot0=0;
  for (unsigned int i=0; i< rs.size(); ++i) {
    double cur=rs[i]->evaluate(false);
    tot0+=cur;
  }
  double tot1=0;
  for (unsigned int i=0; i< ps.size(); ++i) {
    for (unsigned int j=0; j < i; ++j) {
      ParticlePair pp(ps[i], ps[j]);
      if (!interactions[0]->get_value(pp)) {
        double cur= ssps->evaluate(pp, nullptr);
        tot1+=cur;
      }
    }
  }
  std::cout << "scores are " << tot0 << " " << tot1 << std::endl;
  return 0;
}
