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

#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif

namespace {
  VersionInfo get_module_version_info() {
    return benchmark::get_module_version_info();
  }

  void dummy_f_destructor(){}
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

template <class PR, class PS0, class PS1, class SS>
It create(PS0 *link, PS1 *lb, SS *bottom) {
  It ret;
  ret.m= new Model();
  PairFilters pfs;
  typedef GenericInContainerPairFilter<ConsecutivePairContainer> Filter;
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
      pfs.push_back(new Filter(cpc));
      IMP_NEW(PairsRestraint, pr, (link, cpc));
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
  IMP_NEW(PR, pr, (lb, cpc, "close pairs"));
  ret.m->add_restraint(pr);
  IMP_NEW(SingletonsRestraint, sr,
          (bottom, ret.lsc));
  ret.m->add_restraint(sr);
  ret.sp
    = SimulationParameters::setup_particle(new Particle(ret.m));
  ret.bd= new BrownianDynamics(ret.sp);
  double ts=Diffusion(ret.all[0]).get_time_step_from_sigma(sigma);
  ret.sp.set_maximum_time_step(640); // from test below
  ret.all.push_back(ret.sp);
  return ret;
}

void read(std::string name, It it) {
  IMP_CATCH_AND_TERMINATE(read_model(name, it.all));
  it.cpc->set_slack(it.sp->get_value(FloatKey("slack")));
  if (it.sp != it.bd->get_simulation_parameters()) {
    std::cerr << "Parameters don't match " << it.sp
              << " vs " << it.bd->get_simulation_parameters()
              << std::endl;
  }
}

double simulate(It it, int ns, bool verbose=false) {
  if (!verbose) it.bd->set_log_level(SILENT);
  return it.bd->optimize(ns);
}

void initialize(It it) {
  //double e=simulate(it, 1e6,true);
  //std::cout << "Time step is " << it.sp.get_maximum_time_step() << std::endl;
  it.m->set_gather_statistics(true);
  std::cout << "Relaxing from " << it.m->evaluate(false) << std::endl;
  it.bd->optimize(10);
  std::cout << "To ";
  it.m->show_restraint_score_statistics();
  it.m->set_gather_statistics(false);
  std::ofstream out("deps.dot");
  set_log_level(VERBOSE);
  double slack;
  {
    std::cout << "Estimating slack " << std::endl;
    SetLogState sl(IMP::VERBOSE);
    slack= get_slack_estimate(it.all,0, 100, 1,
                              Restraints(1, it.m->get_root_restraint_set()),
                              true,
                              it.bd,
                              it.cpc);
  }
  it.sp->add_attribute(FloatKey("slack"), slack, false);
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

template <int I, class PR, class PS0, class PS1, class SS>
void do_benchmark(std::string name, int argc, char *argv[], PS0 *link,
                  PS1 *lb, SS *bottom) ATTRIBUTES;

template <int I, class PR, class PS0, class PS1, class SS>
void do_benchmark(std::string name, int argc, char *argv[], PS0 *link,
                  PS1 *lb, SS *bottom) {
  It it= create<PR>(link, lb, bottom);
  std::string in;
  if (argc >0) {
    in =argv[0];
  } else {
    IMP_CATCH_AND_TERMINATE(in
                            =IMP::benchmark::get_data_path("brownian.imp"));
  }
  read(in, it);
  double total=0, runtime=0;
  int ns=1e3;
  if (argc >1) {
    ns=atoi(argv[1]);
  }
  IMP_TIME(
           {
             total+=simulate(it, ns);
           }, runtime);
  IMP::benchmark::report(std::string("bd ")+name, runtime, total);
  if (argc>2) {
    IMP_CATCH_AND_TERMINATE(write_model(it.all, argv[2]));
  }
}

//new LowerBound(kk)

int main(int argc , char **argv) {
  // shorten lines, ick
  typedef HarmonicLowerBound HLB;
  FloatKey xk=  XYZ::get_xyz_keys()[0];
  if (argc>=3 && std::string(argv[1])=="-s") {
    It it= create<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                  new SphereDistancePairScore(new HLB(0,kk)),
                                  new AttributeSingletonScore(new HLB(0,kk),
                                                             xk));
    initialize(it);
    write_model(it.all, argv[2]);
  } else if (argc>=4 && std::string(argv[1])=="-d") {
    It it= create<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                  new SphereDistancePairScore(new HLB(0,kk)),
                                  new AttributeSingletonScore(new HLB(0,kk),
                                                              xk));
    read(argv[2], it);
    do_display(it, std::string(argv[3]));
  } else if (argc >=2 && std::string(argv[1])=="-t") {
      It it= create<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                  new SphereDistancePairScore(new HLB(0,kk)),
                                  new AttributeSingletonScore(new HLB(0,kk),
                                                             xk));
      if (argc <=2) {
        read(IMP::benchmark::get_data_path("brownian.imp"), it);
      } else {
        read(argv[2], it);
      }
      double ts= get_maximum_time_step_estimate(it.bd);
      std::cout << "Maximum time step is " << ts;
      //it.sp.set_maximum_time_step(ts);
  } else if (argc >=2 && std::string(argv[1])=="-p") {
      typedef ContainerRestraint<SoftSpherePairScore, ClosePairContainer> PR;
      do_benchmark<1, PR >("custom", argc-2, argv+2,
                           new HarmonicDistancePairScore(len, kk),
                           new SoftSpherePairScore(kk),
                           new AttributeSingletonScore(new HLB(0,kk),
                                                       XYZ::get_xyz_keys()[0]));
  } else {
    {
      typedef ContainerRestraint<SoftSpherePairScore, ClosePairContainer> PR;
      do_benchmark<1, PR >("custom", argc-1, argv+1,
                           new HarmonicDistancePairScore(len, kk),
                           new SoftSpherePairScore(kk),
                           new AttributeSingletonScore(new HLB(0,kk),
                                                       XYZ::get_xyz_keys()[0]));
       do_benchmark<1, PairsRestraint>("scores", argc-1, argv+1,
                                      new HarmonicDistancePairScore(len, kk),
                                      new SoftSpherePairScore(kk),
                                      new AttributeSingletonScore(new HLB(0,kk),
                                                                  xk));
     do_benchmark<0, PairsRestraint>("generic", argc-1, argv+1,
                                  new DistancePairScore(new Harmonic(len,kk)),
                                  new SphereDistancePairScore(new HLB(0,kk)),
                                      new AttributeSingletonScore(new HLB(0,kk),
                                                                  xk));
    }
  }
  return 0;
}
