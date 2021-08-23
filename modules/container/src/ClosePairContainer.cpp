/**
 *  \file ClosePairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ClosePairContainer.h"
#include "IMP/core/internal/close_pairs_helpers.h"
#include <IMP/log.h>
#include <algorithm>
#include <boost/timer.hpp>
#include <vector>

IMPCONTAINER_BEGIN_NAMESPACE

ClosePairContainer::ClosePairContainer(SingletonContainerAdaptor c,
                                       double distance, double slack,
                                       std::string name)
    : P(c, distance, core::internal::default_cpf(c->get_indexes().size()),
        slack, name) {
  c.set_name_if_default("ClosePairContainerInput%1%");
}

ClosePairContainer::ClosePairContainer(SingletonContainerAdaptor c,
                                       double distance,
                                       core::ClosePairsFinder *cpf,
                                       double slack, std::string name)
    : P(c, distance, cpf, slack, name) {
  c.set_name_if_default("ClosePairContainerInput%1%");
}

namespace {
struct Data {
  double slack;
  double lifetime;
  double rcost;
  double ccost;
};
}

double get_slack_estimate(Model *m, ParticleIndexes ps, double upper_bound,
                          double step, const RestraintsTemp &restraints,
                          bool derivatives, Optimizer *opt,
                          ClosePairContainer *cpc) {
  std::vector<Data> datas;
  for (double slack = 0; slack < upper_bound; slack += step) {
    IMP_LOG_VERBOSE("Computing for " << slack << std::endl);
    datas.push_back(Data());
    datas.back().slack = slack;
    {
      boost::timer imp_timer;
      int count = 0;
      SetLogState sl(opt->get_model(), SILENT);
      do {
        cpc->set_slack(slack);
        cpc->update();
        ++count;
      } while (imp_timer.elapsed() == 0);
      datas.back().ccost = imp_timer.elapsed() / count;
      IMP_LOG_VERBOSE("Close pair finding cost " << datas.back().ccost
                                                 << std::endl);
    }
    {
      boost::timer imp_timer;
      double score = 0;
      int count = 0;
      int iters = 1;
      SetLogState sl(opt->get_model(), SILENT);
      do {
        for (int j = 0; j < iters; ++j) {
          for (unsigned int i = 0; i < restraints.size(); ++i) {
            score += restraints[i]->evaluate(derivatives);
            // should restore
          }
        }
        count += iters;
        iters *= 2;
      } while (imp_timer.elapsed() == 0);
      datas.back().rcost = imp_timer.elapsed() / count;
      IMP_UNUSED(score);
      IMP_LOG_VERBOSE("Restraint evaluation cost " << datas.back().rcost
                                                   << std::endl);
    }
  }
  int ns = 100;
  int last_ns = 1;
  int opt_i = -1;
  std::vector<Floats> dists(1, Floats(1, 0.0));
  std::vector<std::vector<algebra::Vector3D> > pos(
      1, std::vector<algebra::Vector3D>(ps.size()));
  for (unsigned int j = 0; j < ps.size(); ++j) {
    pos[0][j] = core::XYZ(m, ps[j]).get_coordinates();
  }
  do {
    IMP_LOG_VERBOSE("Stepping from " << last_ns << " to " << ns << std::endl);
    dists.resize(ns, Floats(ns, 0.0));
    for (int i = 0; i < last_ns; ++i) {
      dists[i].resize(ns, 0.0);
    }
    pos.resize(ns, std::vector<algebra::Vector3D>(ps.size()));
    SetLogState sl(opt->get_model(), SILENT);
    for (int i = last_ns; i < ns; ++i) {
      opt->optimize(1);
      for (unsigned int j = 0; j < ps.size(); ++j) {
        pos[i][j] = core::XYZ(m, ps[j]).get_coordinates();
      }
    }
    for (int i = last_ns; i < ns; ++i) {
      for (int j = 0; j < i; ++j) {
        double md = 0;
        for (unsigned int k = 0; k < ps.size(); ++k) {
          md = std::max(md, algebra::get_distance(pos[i][k], pos[j][k]));
        }
        dists[i][j] = md;
        dists[j][i] = md;
      }
    }
    // estimate lifetimes from slack
    for (unsigned int i = 0; i < datas.size(); ++i) {
      Ints deaths;
      for (int j = 0; j < ns; ++j) {
        for (int k = j + 1; k < ns; ++k) {
          if (dists[j][k] > datas[i].slack) {
            deaths.push_back(k - j);
            break;
          }
        }
      }
      std::sort(deaths.begin(), deaths.end());
      // kaplan meier estimator
      double ml = 0;
      if (deaths.empty()) {
        ml = ns;
      } else {
        // double l=1;
        IMP_INTERNAL_CHECK(deaths.size() < static_cast<unsigned int>(ns),
                           "Too much death");
        double S = 1;
        for (unsigned int j = 0; j < deaths.size(); ++j) {
          double n = ns - j;
          double t = (n - 1.0) / n;
          ml += (S - t * S) * deaths[j];
          S *= t;
        }
      }
      datas[i].lifetime = ml;
      IMP_LOG_VERBOSE("Expected life of " << datas[i].slack << " is "
                                          << datas[i].lifetime << std::endl);
    }

    /**
       C(s) is cost to compute
       R(s) is const to eval restraints
       L(s) is lifetime of slack
       minimize C(s)/L(s)+R(s)
    */
    // smooth
    for (unsigned int i = 1; i < datas.size() - 1; ++i) {
      datas[i].rcost =
          (datas[i].rcost + datas[i - 1].rcost + datas[i + 1].rcost) / 3.0;
      datas[i].ccost =
          (datas[i].ccost + datas[i - 1].ccost + datas[i + 1].ccost) / 3.0;
      datas[i].lifetime =
          (datas[i].lifetime + datas[i - 1].lifetime + datas[i + 1].lifetime) /
          3.0;
    }
    double min = std::numeric_limits<double>::max();
    for (unsigned int i = 0; i < datas.size(); ++i) {
      double v = datas[i].rcost + datas[i].ccost / datas[i].lifetime;
      IMP_LOG_VERBOSE("Cost of " << datas[i].slack << " is " << v << " from "
                                 << datas[i].rcost << " " << datas[i].ccost
                                 << " " << datas[i].lifetime << std::endl);
      if (v < min) {
        min = v;
        opt_i = i;
      }
    }
    last_ns = ns;
    ns *= 2;
    IMP_LOG_VERBOSE("Opt is " << datas[opt_i].slack << std::endl);
    // 2 for the value, 2 for the doubling
    // if it more than 1000, just decide that is enough
  } while (datas[opt_i].lifetime > ns / 4.0 && ns < 1000);
  return datas[opt_i].slack;
}

double get_slack_estimate(const ParticlesTemp &ps, double upper_bound,
                          double step, const RestraintsTemp &restraints,
                          bool derivatives, Optimizer *opt,
                          ClosePairContainer *cpc) {
  IMPCONTAINER_DEPRECATED_FUNCTION_DEF(
        2.16, "Use the index-based function instead");
  Model *m = ps[0]->get_model();
  return get_slack_estimate(m, IMP::internal::get_index(ps),
                            upper_bound, step, restraints, derivatives,
                            opt, cpc);
}

IMPCONTAINER_END_NAMESPACE
