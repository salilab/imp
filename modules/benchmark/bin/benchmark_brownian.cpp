/**
 * Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP.h>
#include <IMP/core.h>
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/display.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/container.h>
#include <IMP/internal/graph_utility.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;
using namespace IMP::display;

const double r=10;
const double len=r;
const double kk=1000;
const double sigma=.1;
const double slack=100*sigma;

#define IMP_CATCH_AND_TERMINATE(expr)            \
  try {                                          \
    expr;                                        \
  } catch (const IMP::Exception &e) {            \
    std::cerr << e.what() << std::endl;          \
    exit(1);                                     \
  }

struct It {
  Pointer<Model> m;
  std::vector<ParticlesTemp> chains;
  ParticlesTemp all;
  Pointer<ListSingletonContainer> lsc;
  Pointer<ClosePairContainer> cpc;
  Pointer<BrownianDynamics> bd;
  SimulationParameters sp;
};

It create() {
  It ret;
  ret.m= new Model();
  PairFilters pfs;
  for (unsigned int i=0; i< 10; ++i) {
    for (unsigned int j=0; j< 10; ++j) {
      ret.chains.push_back(ParticlesTemp());
      for (unsigned int k=0; k< 10; ++k) {
        IMP_NEW(Particle, p, (ret.m));
        ret.chains.back().push_back(p);
        XYZR d= XYZR::setup_particle(p);
        d.set_radius(r);
        Diffusion dd= Diffusion::setup_particle(p);
        dd.set_D_from_radius();
        d.set_coordinates_are_optimized(true);
        d.set_coordinates(Vector3D(i*30.0, j*30.0, k*len));
      }
      IMP_NEW(ConsecutivePairContainer, cpc,(ret.chains.back()));
      pfs.push_back(new InContainerPairFilter(cpc));
      IMP_NEW(PairsRestraint, pr, (new DistancePairScore(new Harmonic(len,kk)),
                                   cpc));
      ret.m->add_restraint(pr);
      ret.all.insert(ret.all.end(),
                     ret.chains.back().begin(), ret.chains.back().end());
      XYZ(ret.chains.back()[0]).set_coordinates_are_optimized(false);
    }
  }
  ret.lsc=new ListSingletonContainer(ret.all);
  IMP_NEW(ClosePairContainer, cpc, (ret.lsc, 0, slack));
  cpc->add_pair_filters(pfs);
  ret.cpc=cpc;
  IMP_NEW(PairsRestraint, pr,
          (new SphereDistancePairScore(new HarmonicLowerBound(0,kk)),
           cpc));
  ret.m->add_restraint(pr);
  IMP_NEW(SingletonsRestraint, sr,
          (new AttributeSingletonScore(new HarmonicLowerBound(0,kk),
                                       XYZ::get_xyz_keys()[0]),
                                    ret.lsc));
  ret.m->add_restraint(sr);
  ret.sp
    = SimulationParameters::setup_particle(new Particle(ret.m));
  ret.bd= new BrownianDynamics(ret.sp);
  double slack= get_slack_estimate(ret.all,0, 30, 1,
                     Restraints(1, ret.m->get_root_restraint_set()),
                                   true,
                                   ret.bd,
                                   ret.cpc);
  ret.sp->add_attribute(FloatKey("slack"), slack, false);
  double ts=Diffusion(ret.all[0]).get_time_step_from_sigma(sigma);
  ret.sp.set_maximum_time_step(ts);
  ret.all.push_back(ret.sp);
  return ret;
}

void read(std::string name, It it) {
  IMP_CATCH_AND_TERMINATE(read_model(name, it.all));
  it.cpc->set_slack(it.sp->get_value(FloatKey("slack")));
}

double simulate(It it, int ns, bool verbose=false) {
  if (!verbose) it.bd->set_log_level(SILENT);
  return it.bd->optimize(ns);
}

void initialize(It it) {
  //double e=simulate(it, 1e6,true);
  std::cout << "Time step is " << it.sp.get_maximum_time_step() << std::endl;
  std::ofstream out("deps.dot");
  set_log_level(VERBOSE);
  //DependencyGraph dg=get_pruned_dependency_graph(
  //get_restraints(it.m->get_root_restraint_set()));
  //IMP::internal::show_as_graphviz(dg, out);
  simulate(it, 500);
  std::cout << "Energy is " << it.m->evaluate(false) << std::endl;
}


void do_display(It it, std::string str) {
  IMP_NEW(PymolWriter, pw, (str));
  for (unsigned int i=0; i< it.chains.size(); ++i) {
    Color c= get_display_color(i);
    std::ostringstream oss;
    oss << "chain " << i;
    for (unsigned int j=0; j< it.chains[i].size(); ++j) {
      IMP_NEW(SphereGeometry, sg, (XYZR(it.chains[i][j]).get_sphere()));
      sg->set_name(oss.str());
      sg->set_color(c);
      pw->add_geometry(sg);
    }
  }
}




int main(int argc , char **argv) {
  if (argc>=3 && std::string(argv[1])=="-s") {
    It it= create();
    initialize(it);
    write_model(it.all, argv[2]);
  } else if (argc>=4 && std::string(argv[1])=="-d") {
    It it= create();
    read(argv[2], it);
    do_display(it, std::string(argv[3]));
  } else {
    It it= create();
    std::string in;
    if (argc >1) {
      in =argv[1];
    } else {
      IMP_CATCH_AND_TERMINATE(in=IMP::benchmark::get_data_path("brownian.imp"));
    }
    read(in, it);
    double total=0, runtime=0;
    int ns=1e6;
    if (argc >2) {
      ns=atoi(argv[2]);
    }
    IMP_TIME(
             {
               total+=simulate(it, ns);
             }, runtime);
    IMP::benchmark::report("bd", runtime, total);
    if (argc>3) {
      IMP_CATCH_AND_TERMINATE(write_model(it.all, argv[3]));
    }
  }
  return 0;
}
