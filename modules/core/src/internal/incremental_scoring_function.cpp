/**
 *  \file MonteCarlo.cpp  \brief Simple Monte Carlo optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/incremental_scoring_function.h>
#include <IMP/kernel/internal/StaticListContainer.h>
#include <IMP/kernel/SingletonModifier.h>
#include <IMP/kernel/SingletonContainer.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/kernel/generic.h>
#include <IMP/kernel/internal/ContainerRestraint.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <numeric>
IMPCORE_BEGIN_INTERNAL_NAMESPACE
namespace {
class DummyPairContainer
    : public kernel::internal::ListLikeContainer<kernel::PairContainer> {
  IMP::base::PointerMember<SingletonContainer> c_;
  IMP::base::PointerMember<ClosePairsFinder> cpf_;

 public:
  DummyPairContainer(SingletonContainer *c, ClosePairsFinder *cpf);
  virtual kernel::ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  void do_score_state_before_evaluate() {}
  virtual kernel::ParticleIndexPairs get_range_indexes() const IMP_OVERRIDE;

  IMP_CLANG_PRAGMA(diagnostic push)
  IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-member-function")
  SingletonContainer *get_singleton_container() const { return c_; }
  ClosePairsFinder *get_close_pairs_finder() const { return cpf_; }
  kernel::Restraints create_decomposition(kernel::PairScore *ps) const {
    kernel::ParticleIndexPairs all = get_range_indexes();
    kernel::Restraints ret(all.size());
    for (unsigned int i = 0; i < all.size(); ++i) {
      ret[i] = new PairRestraint(
          ps, IMP::internal::get_particle(get_model(), all[i]));
    }
    return ret;
  }
  IMP_CLANG_PRAGMA(diagnostic pop)
  template <class PS>
  kernel::Restraints create_decomposition_t(PS *ps) const {
    kernel::ParticleIndexPairs all = get_range_indexes();
    kernel::Restraints ret(all.size());
    for (unsigned int i = 0; i < all.size(); ++i) {
      ret[i] = IMP::create_restraint(
          ps, IMP::internal::get_particle(get_model(), all[i]),
          "IncrementalClosePairs%1%");
    }
    return ret;
  }
  IMP_OBJECT_METHODS(DummyPairContainer)
};

DummyPairContainer::DummyPairContainer(SingletonContainer *c,
                                       ClosePairsFinder *cpf)
    : kernel::internal::ListLikeContainer<kernel::PairContainer>(
          c->get_model(), "ClosePairContainer") {
  c_ = c;
  cpf_ = cpf;
}

ModelObjectsTemp DummyPairContainer::do_get_inputs() const {
  kernel::ModelObjectsTemp ret =
      cpf_->get_inputs(get_model(), c_->get_indexes());
  ret.push_back(c_);
  return ret;
}

ParticleIndexPairs DummyPairContainer::get_range_indexes() const {
  kernel::ParticleIndexes pis = c_->get_range_indexes();
  kernel::ParticleIndexPairs ret;
  ret.reserve(pis.size() * (pis.size() - 1) / 2);
  for (unsigned int i = 0; i < pis.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      ret.push_back(kernel::ParticleIndexPair(pis[i], pis[j]));
    }
  }
  return ret;
}

ParticleIndexes DummyPairContainer::get_all_possible_indexes() const {
  kernel::ParticleIndexes ret = c_->get_all_possible_indexes();
  kernel::ModelObjectsTemp mos =
      cpf_->get_inputs(get_model(), c_->get_indexes());
  for (unsigned int i = 0; i < mos.size(); ++i) {
    kernel::ModelObject *o = mos[i];
    kernel::Particle *p = dynamic_cast<kernel::Particle *>(o);
    if (p) ret.push_back(p->get_index());
  }
  return ret;
}
}

/** to handle good/max evaluate, add dummy restraints for each
    restraint set that return 0 or inf if the last scores for the
    set are bad.*/

NBLScoring::NBLScoring(PairScore *ps, double distance,
                       const kernel::ParticleIndexes &to_move,
                       const kernel::ParticlesTemp &particles,
                       const PairPredicates &filters, double weight, double max)
    : cache_(IMP::internal::get_index(particles),
             NBGenerator(IMP::internal::get_model(particles),
                         IMP::internal::get_index(particles), ps, distance,
                         filters),
             NBChecker(IMP::internal::get_model(particles),
                       IMP::internal::get_index(particles), ps, distance,
                       filters)) {
  weight_ = weight;
  max_ = max;
  to_move_ = to_move;
  DependencyGraph dg = get_dependency_graph(particles[0]->get_model());
  DependencyGraphVertexIndex dgi = get_vertex_index(dg);
  update_dependencies(dg, dgi);
  dummy_restraint_ = create_restraint();
}

void NBLScoring::update_dependencies(const DependencyGraph &dg,
                                     const DependencyGraphVertexIndex &index) {
  for (unsigned int i = 0; i < to_move_.size(); ++i) {
    kernel::Particle *p = cache_.get_generator().m_->get_particle(to_move_[i]);
    kernel::ParticlesTemp ps =
        get_dependent_particles(p, kernel::ParticlesTemp(), dg, index);
    // intersect that with the ones I care about
    kernel::ParticleIndexes pis = IMP::internal::get_index(ps);
    std::sort(pis.begin(), pis.end());
    kernel::ParticleIndexes deps;
    std::set_intersection(
        pis.begin(), pis.end(), cache_.get_generator().pis_.begin(),
        cache_.get_generator().pis_.end(), std::back_inserter(deps));
    controlled_[to_move_[i]] = deps;
    IMP_LOG_TERSE("Particle " << Showable(p) << " controls "
                              << IMP::internal::get_particle(
                                     cache_.get_generator().m_, deps)
                              << std::endl);
  }
}
namespace {
struct NBSum {
  double value;
  NBSum() : value(0) {}
  template <class T>
  void operator()(const T &t) {
    IMP_LOG_TERSE("adding " << t << std::endl);
    value += t.score;
  }
};
struct NBShow {
  template <class T>
  void operator()(const T &t) {
    IMP_LOG_VARIABLE(t);
    IMP_LOG_TERSE(t << std::endl);
  }
};
}

void NBLScoring::set_moved(const kernel::ParticleIndexes &moved) {
  IMP_FUNCTION_LOG;

  for (unsigned int i = 0; i < moved.size(); ++i) {
    kernel::ParticleIndexes c = controlled_.find(moved[i])->second;
    IMP_LOG_TERSE("Got input particle " << moved[i] << " that controls " << c
                                        << std::endl);
    for (unsigned int i = 0; i < c.size(); ++i) {
      cache_.remove(c[i]);
    }
  }
  IMP_LOG_TERSE("Cleared state is ");
  // must not do apply so we don't fill it up again
  cache_.apply_to_current_contents(NBShow());
  IMP_LOG_TERSE(std::endl);
}

double NBLScoring::get_score() {
  IMP_FUNCTION_LOG;
  return weight_ * cache_.apply(NBSum()).value;
}

Restraint *NBLScoring::create_restraint() const {
  IMP_NEW(kernel::internal::StaticListContainer<kernel::SingletonContainer>,
          lsc, (cache_.get_generator().m_, "NBLInput Container %1%"));
  lsc->set(cache_.get_generator().pis_);
  IMP_NEW(DummyPairContainer, cpc, (lsc, default_cpf(1000)));

  base::Pointer<kernel::Restraint> ret =
      kernel::create_restraint(cache_.get_generator().score_.get(), cpc.get());
  return ret.release();
}

NBGenerator::NBGenerator(kernel::Model *m, const kernel::ParticleIndexes &pis,
                         PairScore *ps, double distance,
                         const PairPredicates &pfs) {
  m_ = m;
  score_ = ps;
  pis_ = pis;
  std::sort(pis_.begin(), pis_.end());
  filters_ = pfs;
  double maxr = 0;
  for (unsigned int i = 0; i < pis_.size(); ++i) {
    maxr = std::max(maxr, core::XYZR(m, pis_[i]).get_radius());
  }
  distance_ = distance + 2 * maxr;
  filters_ = pfs;

  algebra::Vector3Ds vs(pis_.size());
  for (unsigned int i = 0; i < pis_.size(); ++i) {
    vs[i] = XYZ(m_, pis_[i]).get_coordinates();
    to_dnn_[pis_[i]] = i;
  }
  dnn_ = new algebra::DynamicNearestNeighbor3D(vs, distance_);
  dnn_->set_log_level(IMP::base::SILENT);
}

NBGenerator::result_type NBGenerator::operator()(argument_type a) const {
  std::sort(a.begin(), a.end());
  // a.erase(std::unique(a.begin(), a.end()), a.end());
  // IMP_LOG_TERSE( "Input of " << ia << " resolves to " << a << std::endl);
  IMP_LOG_TERSE("Generating pair scores from " << a << std::endl);
  result_type ret;
  for (unsigned int i = 0; i < a.size(); ++i) {
    int di = to_dnn_.find(a[i])->second;
    XYZ d(m_, a[i]);
    dnn_->set_coordinates(di, d.get_coordinates());
  }
  for (unsigned int i = 0; i < a.size(); ++i) {
    int di = to_dnn_.find(a[i])->second;
    Ints n = dnn_->get_in_ball(di, distance_);
    IMP_LOG_TERSE("Neighbors are " << n << std::endl);
    for (unsigned int j = 0; j < n.size(); ++j) {
      // if the partner is not in the list or is a lower index
      kernel::ParticleIndex ppi = pis_[n[j]];
      IMP_LOG_VERBOSE("Checking out pair " << a[i] << " " << ppi << std::endl);
      if (std::find(a.begin(), a.end(), ppi) == a.end() || ppi < a[i]) {
        kernel::ParticleIndexPair pp(a[i], ppi);
        bool filtered = false;
        for (unsigned int k = 0; k < filters_.size(); ++k) {
          if (filters_[k]->get_value_index(m_, pp)) {
            filtered = true;
            break;
          }
        }
        if (!filtered) {
          double score = score_->evaluate_index(m_, pp, nullptr);
          if (score != 0) {
            ret.push_back(single_result_type(a[i], ppi, score));
            IMP_LOG_VERBOSE("Score for " << pp << " is " << score << std::endl);
          } else {
            IMP_LOG_VERBOSE("scoreless" << std::endl);
          }
        } else {
          IMP_LOG_VERBOSE("filtered" << std::endl);
        }
      } else {
        IMP_LOG_VERBOSE("redundant" << std::endl);
      }
    }
  }
  return ret;
}

NBChecker::NBChecker(kernel::Model *m, const kernel::ParticleIndexes &pis,
                     PairScore *score, double d, const PairPredicates &filt)
    : m_(m), pis_(pis), score_(score), distance_(d), filt_(filt) {}
bool NBChecker::operator()(const NBGenerator::result_type &vals) const {
  IMP_NEW(GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(.9 * distance_);
  kernel::ParticleIndexPairs found = gcpf->get_close_pairs(m_, pis_);
  for (unsigned int i = 0; i < filt_.size(); ++i) {
    filt_[i]->remove_if_equal(m_, found, 1);
  }
  base::set<kernel::ParticleIndexPair> vals_index;
  for (unsigned int i = 0; i < vals.size(); ++i) {
    vals_index.insert(kernel::ParticleIndexPair(vals[i][0], vals[i][1]));
  }
  for (unsigned int i = 0; i < found.size(); ++i) {
    double score = score_->evaluate_index(m_, found[i], nullptr);
    if (score == 0) continue;
    bool has = vals_index.find(found[i]) != vals_index.end() ||
               vals_index.find(kernel::ParticleIndexPair(
                   found[i][1], found[i][0])) != vals_index.end();
    if (!has) {
      IMP_WARN("Can't find pair " << found[i] << " in list " << vals
                                  << " at distance "
                                  << get_distance(XYZR(m_, found[i][0]),
                                                  XYZR(m_, found[i][1]))
                                  << " with threshold " << distance_
                                  << std::endl);
      return false;
    }
  }
  for (unsigned int i = 0; i < vals.size(); ++i) {
    kernel::ParticleIndexPair pip(vals[i][0], vals[i][1]);
    double nscore = score_->evaluate_index(m_, pip, nullptr);
    if (std::abs(nscore - vals[i].score) > .1) {
      IMP_WARN("Scores don't match for " << pip << " had " << vals[i].score
                                         << " and got " << nscore << std::endl);
      return false;
    }
  }
  return true;
}

IMPCORE_END_INTERNAL_NAMESPACE
