/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */
#include <IMP/domino.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP/core.h>
#include <IMP/internal/pdb.h>
using namespace IMP;
using namespace IMP::domino;
using namespace IMP::algebra;
using namespace IMP::core;
using namespace IMP::container;

int main(int argc, char *[]) {
  IMP_NEW(Model, m, ());
  set_log_level(SILENT);
  m->set_log_level(SILENT);
  std::string path=IMP::benchmark::get_data_path("small_protein.pdb");
  ParticlesTemp ps
      = IMP::internal::create_particles_from_pdb(path,m);
  ReferenceFrame3Ds vs;
  while (ps.size() > 800) {
    ps.pop_back();
  }
  ParticlesTemp rs;
  for (unsigned int i=0; i< 80; ++i) {
    IMP_NEW(Particle, p, (m));
    ParticlesTemp leaves(ps.begin()+i*10, ps.begin()+(i+1)*10);
    RigidBody r= RigidBody::setup_particle(p, leaves);
    vs.push_back(ReferenceFrame3D(r.get_reference_frame()));
    rs.push_back(r);
  }
  IMP_NEW(ListSingletonContainer, lsc, (rs));
#ifdef IMP_USE_CGAL
  IMP_NEW(BoxSweepClosePairsFinder, cpf, ());
#else
  IMP_NEW(GridClosePairsFinder, cpf, ());
#endif
  cpf->set_distance(3);
  ParticlePairsTemp ppt= cpf->get_close_pairs(lsc);
  for (unsigned int i=0; i < ppt.size(); ++i) {
    double d= get_distance(XYZ(ppt[i][0]), XYZ(ppt[i][1]));
    Restraint *r=new DistanceRestraint(new Harmonic(d, 1),
                                       ppt[i][0], ppt[i][1]);
    std::ostringstream oss;
    oss << "Edge " << ppt[i][0]->get_name()
        << "-" << ppt[i][1]->get_name();
    r->set_name(oss.str());
    m->add_restraint(r);
  }
  IMP_NEW(RigidBodyStates, pstates, (vs));
  IMP_NEW(ParticleStatesTable, pst, ());
  for (unsigned int i=0; i< rs.size(); ++i) {
    pst->set_particle_states(rs[i], pstates);
  }
  IMP_NEW(DominoSampler, ds, (m, pst));
  m->set_maximum_score(1);
  double runtime, num=0;
  /*#ifndef NDEBUG
  ds->set_log_level(VERBOSE);
  #endif*/
  int n=1;
  if (argc>1) {
    n=10;
  }
  IMP_TIME(
             {
               for (int i=0; i< n; ++i) {
                 Pointer<ConfigurationSet> cs= ds->get_sample();
                 num+=cs->get_number_of_configurations();
               }
             }, runtime);
  IMP::benchmark::report("domino small", runtime, num);
  return IMP::benchmark::get_return_value();
}
