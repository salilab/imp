/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/MCCGSampler.h>

#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/BallMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/utility.h>
#include <IMP/log.h>
#include <IMP/dependency_graph.h>
#include <IMP/internal/InternalListSingletonContainer.h>
#include <IMP/algebra/vector_generators.h>
#include <boost/random/uniform_real.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <IMP/core/core_macros.h>
#include <IMP/random.h>
#include <boost/graph/reverse_graph.hpp>
#include <IMP/base/log.h>
#include <boost/progress.hpp>
#include <boost/scoped_ptr.hpp>
#include <IMP/base/vector_property_map.h>

IMPCORE_BEGIN_NAMESPACE

class CollectVisitor: public boost::default_dfs_visitor {
  const std::map<Particle*, int> &lu_;
  boost::property_map<DependencyGraph,
                      boost::vertex_name_t>::const_type vm_;
  Ints &vals_;
public:
  const Ints &get_collected() {
    std::sort(vals_.begin(), vals_.end());
    vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    return vals_;
  }
  CollectVisitor(const DependencyGraph &g,
                 const std::map<Particle*, int>&lu,
                 Ints &vals):
    lu_(lu),
    vm_(boost::get(boost::vertex_name, g)),
    vals_(vals){
  }
  template <class G>
  void discover_vertex( typename boost::graph_traits<G>::vertex_descriptor u,
                       const G& ) {
    Object *o= vm_[u];
    Particle *p=dynamic_cast<Particle*>(o);
    if (p) {
      //std::cout << "Checking particle " << p->get_name() << std::endl;
      typename std::map<Particle*, int>::const_iterator it= lu_.find(p);
      if (it != lu_.end()) {
        vals_.push_back(it->second);
      }
    }
  }
};


namespace {
class ScoreWeightedIncrementalBallMover :
    public MonteCarloMover
{
public:
  ScoreWeightedIncrementalBallMover(const ParticlesTemp &ps,
                                    unsigned int n,
                                    Float radius);
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ScoreWeightedIncrementalBallMover);
private:
  const ParticlesTemp ps_;
  unsigned int n_;
  Float radius_;
  ParticlesTemp moved_;
  algebra::Vector3Ds old_coords_;
  base::Vector<std::pair<Restraint*, Ints> > deps_;
};


ScoreWeightedIncrementalBallMover
::ScoreWeightedIncrementalBallMover(const ParticlesTemp& sc,
                                    unsigned int n,
                                    Float radius):
  MonteCarloMover(sc[0]->get_model(), "IncrementalBallMover%1%"),
  ps_(sc),
  n_(n),
  radius_(radius),
  moved_(n_)
{
  Model *m= sc[0]->get_model();
  const DependencyGraph dg
    = get_dependency_graph(m);
  typedef boost::graph_traits<DependencyGraph> DGTraits;
  typedef boost::property_map<DependencyGraph, boost::vertex_name_t>::const_type
    DGVMap;
  DGVMap vm= boost::get(boost::vertex_name, dg);
  std::map<Particle*, int> index;
  for (unsigned int i=0; i< ps_.size(); ++i) {
    index[ps_[i]]=i;
  }
  for (std::pair<DGTraits::vertex_iterator, DGTraits::vertex_iterator>
         be= boost::vertices(dg); be.first != be.second; ++be.first) {
    Restraint *r= dynamic_cast<Restraint*>(vm[*be.first]);
    if (r) {
      boost::vector_property_map<int> color(boost::num_vertices(dg));
      Ints out;
      CollectVisitor cp(dg, index, out);
      boost::depth_first_visit(boost::make_reverse_graph(dg),
                               *be.first, cp, color);
      deps_.push_back(std::make_pair(r, out));
      /*std::cout << "Restraint " << r->get_name()
                << " has particles ";
      for (unsigned int i=0; i< out.size(); ++i) {
        std::cout<< ps_[out[i]]->get_name() << " ";
      }
      std::cout << std::endl;*/
    }
  }
}

MonteCarloMoverResult ScoreWeightedIncrementalBallMover::do_propose() {
  // damnit, why didn't these functions make it into the standard
  /*std::random_sample(sc_->particles_begin(), sc_->particles_end(),
    moved_.begin(), moved_.end());*/
  IMP_CHECK_OBJECT(this);
  IMP_OBJECT_LOG;
  Floats weights(ps_.size(), 0);
  for (unsigned int i=0; i< deps_.size(); ++i) {
    double v= deps_[i].first->evaluate(false)/ deps_[i].second.size();
    /*std::cout << "Restraint " << deps_[i].first->get_name()
      << " contributes " << v << std::endl;*/
    for (unsigned int j=0; j < deps_[i].second.size(); ++j) {
      weights[deps_[i].second[j]]+= v;
    }
  }
  double total=std::accumulate(weights.begin(), weights.end(), 0.);
  /*IMP_IF_LOG(SILENT) {
    IMP_LOG(SILENT, "scores are ");
    for (unsigned int i=0; i< weights.size(); ++i) {
      IMP_LOG(SILENT, weights[i] << " ");
    }
    IMP_LOG(SILENT, std::endl);
    };*/
  // if the score is tiny, give up
  moved_.clear();
  old_coords_.clear();
  if (total < .0001) {
    return MonteCarloMoverResult(ParticleIndexes(), 1.0);
  }
  for (unsigned int i=0; i< weights.size(); ++i) {
    weights[i]/=(total/n_);
  }
  /*IMP_IF_LOG(SILENT) {
    IMP_LOG(SILENT, "Weights are ");
    for (unsigned int i=0; i< weights.size(); ++i) {
      IMP_LOG(SILENT, weights[i] << " ");
    }
    IMP_LOG(SILENT, std::endl);
    };*/
  while (true) {
    ::boost::uniform_real<> rand(0,1);
    for (unsigned int i=0; i< weights.size(); ++i) {
      if (rand(random_number_generator) < weights[i]) {
        moved_.push_back(ps_[i]);
        XYZ d(ps_[i]);
        old_coords_.push_back(d.get_coordinates());
        IMP_USAGE_CHECK(d.get_coordinates_are_optimized(),
                        "Particles passed to "
                        << "ScoreWeightedIncrementalBallMover must have "
                        << "optimized cartesian coordinates. "
                        << moved_[i]->get_name() << " does not.");
        d.set_coordinates(algebra::get_random_vector_in<3>
                          (algebra::Sphere3D(d.get_coordinates(),
                                             radius_)));
        IMP_LOG_VERBOSE( "Proposing move of particle " << d->get_name()
                << " to " << d.get_coordinates() << std::endl);
      }
    }
    if (moved_.empty()) {
      IMP_LOG_TERSE( "trying again to find particles to move "
              << total << std::endl);
    } else {
      break;
    }
  }
  /*std::cout << "Moving ";
  for (unsigned int i=0; i< moved_.size(); ++i) {
    std::cout <<moved_[i]->get_name() << " ";
  }
  std::cout << std::endl;*/
  return MonteCarloMoverResult(get_indexes(moved_), 1.0);
}


void ScoreWeightedIncrementalBallMover::do_reject() {
  for (unsigned int i=0; i< moved_.size(); ++i) {
    XYZ cd(moved_[i]);
    cd.set_coordinates(old_coords_[i]);
  }
}

kernel::ModelObjectsTemp
ScoreWeightedIncrementalBallMover::do_get_inputs() const {
  return kernel::ModelObjectsTemp(ps_.begin(), ps_.end());
}



}

#define XK XYZ::get_xyz_keys()[0]
#define YK XYZ::get_xyz_keys()[1]
#define ZK XYZ::get_xyz_keys()[2]

MCCGSampler::Parameters::Parameters(){
  cg_steps_=100;
  attempts_=1000;
  mc_steps_=100;
}

MCCGSampler::MCCGSampler(Model *m, std::string name): Sampler(m, name),
                                    is_refining_(false){
}

void MCCGSampler::set_bounding_box(const algebra::BoundingBox3D &bb) {
  FloatKeys xyzks= XYZ::get_xyz_keys();
  for (unsigned int i=0; i< xyzks.size(); ++i) {
    default_parameters_.bounds_[xyzks[i]]= std::make_pair(bb.get_corner(0)[i],
                                      bb.get_corner(1)[i]);
  }
}

void MCCGSampler::set_number_of_attempts(unsigned int at) {
  default_parameters_.attempts_=at;
}

void MCCGSampler::set_number_of_conjugate_gradient_steps(unsigned int cg) {
  default_parameters_.cg_steps_=cg;
}

void MCCGSampler::set_number_of_monte_carlo_steps(unsigned int cg) {
  default_parameters_.mc_steps_=cg;
}

void MCCGSampler::set_max_monte_carlo_step_size(double d) {
  default_parameters_.ball_sizes_[XK]=d;
  default_parameters_.ball_sizes_[YK]=d;
  default_parameters_.ball_sizes_[ZK]=d;
}

void MCCGSampler::set_max_monte_carlo_step_size(FloatKey k, double d) {
  default_parameters_.ball_sizes_[k]=d;
}

void MCCGSampler::set_is_refining(bool tf) {
  is_refining_=tf;
}

IMP::internal::InternalListSingletonContainer*
MCCGSampler::set_up_movers(const Parameters &pms,
                           MonteCarlo *mc) const {
  if (pms.opt_keys_[0] != XK
      && pms.opt_keys_[1] != YK && pms.opt_keys_[2] != ZK){
    IMP_THROW("Currently, the MCCGSampler can only handle "
              << "cartesian coordinates",
              ValueException);
  }
  ParticlesTemp ps;
  for (Model::ParticleIterator pit= mc->get_model()->particles_begin();
       pit != mc->get_model()->particles_end(); ++pit) {
    if (XYZ::particle_is_instance(*pit)
        && XYZ(*pit).get_coordinates_are_optimized()) {
      ps.push_back(*pit);
    }
  }
  IMP_NEW(IMP::internal::InternalListSingletonContainer, sc, (mc->get_model(),
                                                     "mccg particles"));
  sc->set_was_used(true);
  sc->set(IMP::internal::get_index(ps));
  IMP_NEW(ScoreWeightedIncrementalBallMover, bm,
          (ps, 2, pms.ball_sizes_.find(XK)->second));
  mc->add_mover(bm);
  return sc.release();
}

void MCCGSampler
::randomize(const Parameters &pms,
            IMP::internal::InternalListSingletonContainer *sc) const {
  algebra::BoundingBox3D
    bb(algebra::Vector3D(pms.bounds_.find(XK)->second.first,
                         pms.bounds_.find(YK)->second.first,
                         pms.bounds_.find(ZK)->second.first),
       algebra::Vector3D(pms.bounds_.find(XK)->second.second,
                         pms.bounds_.find(YK)->second.second,
                         pms.bounds_.find(ZK)->second.second));
  IMP_CONTAINER_FOREACH(
                        IMP::internal::InternalListSingletonContainer,
                        sc,
                        {
                          // _1 is the ParticleIndex for a singleton container
                          IMP::core::XYZ d(get_model(), _1);
                          d.set_coordinates(algebra::get_random_vector_in(bb));
                        }
                        );
}


MCCGSampler::Parameters MCCGSampler::fill_in_parameters() const {
  Parameters pms= default_parameters_;
  pms.opt_keys_= XYZ::get_xyz_keys();
  if (pms.bounds_.empty()) {
    pms.bounds_[XK]= get_model()->get_range(XK);
    pms.bounds_[YK]= get_model()->get_range(YK);
    pms.bounds_[ZK]= get_model()->get_range(ZK);
  }
  // we have mc_steps_ steps to explore the whole space
  // each time we move a constant number of the particles
  // (r/l)^(na*np) is the number of voxels. this number
  // should be similar to number of steps
  // so l= r* pow(cg_steps_, 1/(na*np))
  // for now na=3, assume half the particles are optimized
  for (Parameters::Bounds::const_iterator it= pms.bounds_.begin();
       it != pms.bounds_.end(); ++it) {
    if (pms.ball_sizes_.find(it->first) == pms.ball_sizes_.end()) {
      double r= it->second.second-it->second.first;
      double l= r*std::pow(static_cast<double>(pms.cg_steps_), 1.0/(3*1000));
      pms.ball_sizes_[it->first]=l;
    }
  }
  if (! pms.local_opt_) {
    pms.local_opt_= new ConjugateGradients(get_model());
    pms.local_opt_->set_scoring_function(get_scoring_function());
    pms.local_opt_->set_stop_on_good_score(true);
  }
  return pms;
}
void MCCGSampler::set_local_optimizer(Optimizer *opt) {
  default_parameters_.local_opt_=opt;
}

void MCCGSampler::set_save_rejected_configurations(bool tf) {
  if (tf && !rejected_) {
    rejected_= new ConfigurationSet(get_model(), "Rejected");
  } else if (!tf) {
    rejected_=static_cast<ConfigurationSet*>(nullptr);
  }
}

ConfigurationSet* MCCGSampler::get_rejected_configurations() const {
  return rejected_;
}


ConfigurationSet *MCCGSampler::do_sample() const {
  IMP_OBJECT_LOG;
  base::LogLevel mll(static_cast<base::LogLevel>(std::max(0,
                                                IMP::base::get_log_level()-1)));
  set_was_used(true);
  //get_model()->set_is_incremental(true);
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  Parameters pms= fill_in_parameters();
  IMP_NEW(MonteCarloWithLocalOptimization, mc, (pms.local_opt_,
                                             pms.cg_steps_));
  mc->set_scoring_function(get_scoring_function());
  //mc->set_log_level(mll);
  mc->set_stop_on_good_score(true);
  mc->add_optimizer_states(OptimizerStatesTemp(optimizer_states_begin(),
                                               optimizer_states_end()));
  pms.local_opt_->set_log_level(mll);
  mc->set_return_best(true);
  Pointer<IMP::internal::InternalListSingletonContainer> sc
    =set_up_movers(pms, mc);
  IMP_IF_CHECK(base::USAGE) {
    if (sc->get_indexes().size()==0) {
      IMP_WARN("There are no particles with optimized cartesian coordinates."
               << std::endl);
      return nullptr;
    }
  }
  IMP_CHECK_OBJECT(sc);
  int failures=0;
  boost::scoped_ptr<boost::progress_display> progress;
  if (IMP::base::get_log_level() == base::PROGRESS) {
    progress.reset(new boost::progress_display(pms.attempts_));
  }
  for (unsigned int i=0; i< pms.attempts_; ++i) {
    ret->load_configuration(-1);
    if (!is_refining_) {
      randomize(pms,sc);
      IMP_LOG_TERSE( "Randomized configuration" << std::endl);
    }
    try {
      mc->optimize(pms.mc_steps_);
    } catch (base::ModelException) {
      IMP_LOG_TERSE( "Optimization ended by exception" << std::endl);
      ++failures;
      continue;
    }
    if (mc->get_scoring_function()->get_had_good_score()) {
      IMP_LOG_TERSE( "Found configuration with score "
              << get_model()->evaluate(false) << std::endl);
      ret->save_configuration();
      IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
        double oe= get_model()->evaluate(false);
        ret->load_configuration(-1);
        ret->load_configuration(ret->get_number_of_configurations()-1);
        double ne= get_model()->evaluate(false);
        if (0) std::cout << oe<< ne;
        IMP_INTERNAL_CHECK(std::abs(ne-oe) < (ne+oe)*.1+.1,
                           "Energies to not match before and after save."
                           << "Expected " << oe << " got " << ne
                           << std::endl);
      }
    } else {
      IMP_LOG_TERSE( "Rejected configuration with score "
              << get_model()->evaluate(false) << std::endl);
      if (rejected_) {
        rejected_->save_configuration();
      }
    }
    if (progress) {
      ++(*progress);
    }
  }
  if (failures != 0) {
    IMP_LOG(WARNING, "The optimization failed " << failures
            << " times due to invalid attribute values or derivatives."
            << std::endl);
  }
  IMP_CHECK_OBJECT(mc);
  IMP_CHECK_OBJECT(sc);
  return ret.release();
}

void MCCGSampler::do_show(std::ostream &out) const {
  out << "attempts " << default_parameters_.attempts_ << std::endl;
}


IMP_LIST_IMPL(MCCGSampler, OptimizerState, optimizer_state,
              OptimizerState*, OptimizerStates);



IMPCORE_END_NAMESPACE
