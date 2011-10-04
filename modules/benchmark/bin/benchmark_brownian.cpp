/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/benchmark/benchmark_config.h>

#ifdef IMP_BENCHMARK_USE_IMP_RMF

#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/atom/BrownianDynamics.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/particle_io.h>
#include <IMP/container/ConsecutivePairContainer.h>
#include <IMP/atom/Mass.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/container/generic.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/container/SingletonsRestraint.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/AttributeSingletonScore.h>

using namespace IMP;
using namespace IMP::core;
using namespace IMP::algebra;
using namespace IMP::atom;
using namespace IMP::container;

const double r=10;
const double len=r;
const double kk=1000;
const double sigma=.1;
const double slack=6;

#ifdef __GNUC__
#define ATTRIBUTES __attribute ((__noinline__))
#else
#define ATTRIBUTES
#endif

namespace {
#ifndef __clang__
  void dummy_f_destructor(){}
#endif
}


struct It {
  Pointer<Model> m;
  atom::Hierarchies chains;
  Pointer<Particle> sp;

  Pointer<ListSingletonContainer> lsc;
  Pointer<ClosePairContainer> cpc;
  Pointer<BrownianDynamics> bd;
};

namespace {

  FloatKey tsk("time step");
  FloatKey sk("slack");

  It create_particles() {
    It ret;
    ret.m= new Model();
    for (unsigned int i=0; i< 10; ++i) {
      for (unsigned int j=0; j< 10; ++j) {
        atom::Hierarchy parent
          = atom::Hierarchy::setup_particle(new Particle(ret.m));
        std::ostringstream oss;
        oss << "chain " << i << " " << j;
        parent->set_name(oss.str());
        ret.chains.push_back(parent);
        for (unsigned int k=0; k< 10; ++k) {
          IMP_NEW(Particle, p, (ret.m));
          std::ostringstream oss;
          oss << "bead " << k;
          p->set_name(oss.str());
          atom::Hierarchy cur= atom::Hierarchy::setup_particle(p);
          ret.chains.back().add_child(cur);
          XYZR d= XYZR::setup_particle(p);
          d.set_radius(r);
          Diffusion dd= Diffusion::setup_particle(p);
          d.set_coordinates_are_optimized(k!=0);
          d.set_coordinates(Vector3D(i*30.0, j*30.0, k*len));
          atom::Mass::setup_particle(p, 1);
        }
      }
    }
    ret.sp= new Particle(ret.m);
    ret.sp->set_name("parameters");
    ret.sp->add_attribute(sk, 4);
    ret.sp->add_attribute(tsk, 50000);
    return ret;
  }

  It create_particles(std::string name) {
    RMF::RootHandle r= RMF::open_rmf_file_read_only(name);
    It ret;
    ret.m= new Model();
    ret.chains= IMP::rmf::create_hierarchies(r, ret.m);
    ret.sp= IMP::rmf::create_particles(r, ret.m)[0];
    return ret;
  }

  void write_particles(It cur, RMF::RootHandle rh, int frame) {
    if (frame==0) {
      for (unsigned int i=0; i< cur.chains.size(); ++i) {
        IMP::rmf::add_hierarchy(rh, cur.chains[i]);
      }
      IMP::rmf::add_particle(rh, cur.sp);
    } else {
      for (unsigned int i=0; i< cur.chains.size(); ++i) {
        IMP::rmf::save_frame(rh, frame, cur.chains[i]);
      }
    }
  }

  template <class PR, class PS0, class PS1, class SS>
  It create_restraints(PS0 *link, PS1 *lb, SS *bottom, It in) {
    It ret=in;

    PairFilters pfs;
    ParticlesTemp all;
    typedef GenericInContainerPairFilter<ConsecutivePairContainer> Filter;
    for (unsigned int i=0; i< ret.chains.size(); ++i) {
      ParticlesTemp cur=ret.chains[i].get_children();
      all.insert(all.end(), cur.begin(), cur.end());
      IMP_NEW(ConsecutivePairContainer, cpc,(cur, true));
      // since they all use the same key
      if (i==0) pfs.push_back(new Filter(cpc));
      ret.m->add_restraint(container::create_restraint(link, cpc));
    }
    ret.lsc=new ListSingletonContainer(all);
    IMP_NEW(ClosePairContainer, cpc, (ret.lsc, 0, ret.sp->get_value(sk)));
    cpc->add_pair_filters(pfs);
    ret.cpc=cpc;
    IMP_NEW(PR, pr, (lb, cpc, "close pairs"));
    ret.m->add_restraint(pr);
    IMP_NEW(SingletonsRestraint, sr,
            (bottom, ret.lsc));
    ret.m->add_restraint(sr);
    ret.bd= new BrownianDynamics(ret.m);
    //double ts=Diffusion(ret.all[0]).get_time_step_from_sigma(sigma);
    ret.bd->set_maximum_time_step(ret.sp->get_value(tsk));
    //std::cout << ret.sp->get_value(tsk) << std::endl;
    ret.bd->set_maximum_move(10);
    // from test below is 640
    return ret;
  }


  double simulate(It it, int ns, bool verbose=false) {
    if (!verbose) it.bd->set_log_level(SILENT);
    return it.bd->optimize(ns);
  }

  void initialize(It it) {
    //double e=simulate(it, 1e6,true);
    //std::cout << "Time step is " << it.sp.get_maximum_time_step()
    // << std::endl;
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
      slack= get_slack_estimate(it.lsc->get_particles(), 100, 1,
           get_restraints(RestraintsTemp(1, it.m->get_root_restraint_set())),
                                true,
                                it.bd,
                                it.cpc);
    }
    it.sp->set_value(FloatKey("slack"), slack);
    //DependencyGraph dg=get_pruned_dependency_graph(
    //get_restraints(it.m->get_root_restraint_set()));
    //IMP::internal::show_as_graphviz(dg, out);
    simulate(it, 500);
    std::cout << "Energy is " << it.m->evaluate(false) << std::endl;
  }


  template <int I, class PR, class PS0, class PS1, class SS>
  void do_benchmark(std::string name, int argc, char *argv[], PS0 *link,
                    PS1 *lb, SS *bottom) ATTRIBUTES;

  template <int I, class PR, class PS0, class PS1, class SS>
  void do_benchmark(std::string name, int argc, char *argv[], PS0 *link,
                    PS1 *lb, SS *bottom) {
    std::string in;
    if (argc >0) {
      in =argv[0];
    } else {
      IMP_CATCH_AND_TERMINATE(in
                              =IMP::benchmark::get_data_path("brownian.rmf"));
    }
    It o= create_particles(in);
    It it= create_restraints<PR>(link, lb, bottom, o);
    double total=0, runtime=0;
    int ns=1000;
    if (argc >1) {
      ns=atoi(argv[1]);
    }
    IMP_TIME(
             {
               total+=simulate(it, ns);
             }, runtime);
    IMP::benchmark::report(std::string("bd ")+name, runtime, total);
    if (argc>2) {
      IMP_CATCH_AND_TERMINATE(write_particles(it,
                                        RMF::create_rmf_file(argv[2]),
                                              0));
    }
  }

  template <int I, class PR, class PS0, class PS1, class SS>
  void do_long_run(std::string name, int argc, char *argv[], PS0 *link,
                   PS1 *lb, SS *bottom) {
    std::string in;
    if (argc >0) {
      in =argv[0];
    } else {
      IMP_CATCH_AND_TERMINATE(in
                              =IMP::benchmark::get_data_path("brownian.rmf"));
    }
    It o= create_particles(in);
    It it= create_restraints<PR>(link, lb, bottom, o);

    double total=0, runtime=0;
    int ns=10000;
    if (argc >1) {
      ns=atoi(argv[1]);
    }
    for (unsigned int i=0; i< 1000; ++i) {
      double cur=0;
      IMP_TIME(
               {
                 cur+=simulate(it, ns);
               }, runtime);
      IMP::benchmark::report(std::string("bd long ")+name, runtime, total);
      it.m->set_gather_statistics(true);
      it.m->evaluate(false);
      it.m->show_restraint_score_statistics();
      it.m->set_gather_statistics(false);
      std::ostringstream oss;
      oss << "build/tmp/sim." << i << ".pym";
    }

    if (argc>2) {
      IMP_CATCH_AND_TERMINATE(write_particles(it,
                                            RMF::create_rmf_file(argv[2]),
                                              0));
    }
  }
}
//new LowerBound(kk)

int main(int argc , char **argv) {
  try {
  // shorten lines, ick
  typedef HarmonicLowerBound HLB;
  FloatKey xk=  XYZ::get_xyz_keys()[0];
  if (argc>=3 && std::string(argv[1])=="-s") {
    It o= create_particles();
    It it=
  create_restraints<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                    new SphereDistancePairScore(new HLB(0,kk)),
                                    new AttributeSingletonScore(new HLB(0,kk),
                                                                xk), o);
    {
      RMF::RootHandle fh= RMF::create_rmf_file(argv[2]);
      write_particles(it, fh, 0);
    }
    std::cout << it.m->evaluate(false) << " is the score " << std::endl;
    initialize(it);
    RMF::RootHandle fh= RMF::create_rmf_file(argv[2]);
    write_particles(it, fh, 0);
  } else if (argc >=2 && std::string(argv[1])=="-t") {
    It cur;
    if (argc <=2) {
      cur= create_particles(IMP::benchmark::get_data_path("brownian.rmf"));
    } else {
      cur= create_particles(argv[2]);
    }
    It it=
  create_restraints<PairsRestraint>(new DistancePairScore(new Harmonic(len,kk)),
                                    new SphereDistancePairScore(new HLB(0,kk)),
                                    new AttributeSingletonScore(new HLB(0,kk),
                                                                  xk),
                                             cur);
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
  } else if (argc >=2 && std::string(argv[1])=="-l") {
    typedef ContainerRestraint<SoftSpherePairScore, ClosePairContainer> PR;
    do_long_run<1, PR >("long", argc-2, argv+2,
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
  } catch (IMP::Exception e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (std::exception e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return IMP::benchmark::get_return_value();
}

#else
int main(int, const char*) {
  return 1;
}
#endif
