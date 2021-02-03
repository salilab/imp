/**
 *  \file IMP/misc/MetricClosePairsFinder.h
 *  \brief Decorator for a sphere-like particle.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMISC_METRIC_CLOSE_PAIRS_FINDER_H
#define IMPMISC_METRIC_CLOSE_PAIRS_FINDER_H

#include <IMP/misc/misc_config.h>
#include <IMP/core/ClosePairsFinder.h>
#include <IMP/Pointer.h>
#include <IMP/particle_index.h>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <limits>

IMPMISC_BEGIN_NAMESPACE

/** A close pairs finder that only depends on metrics on the underlying
    particles. As a result, it should be usable with weird metrics (eg ones
    that include symmetry).

    The LowerBound and UpperBound templates should be functors that bound the
    distance between two particles. For example the LowerBound
    distance between two balls is the center distance minus the radii,
    and the UpperBound distance is the center distance plus the
    radii. The signature should be `double operator()(Model *m, const
    ParticleIndexPair &pip) const`

    The algorithm works by building an index using `sqrt(n)` of the `n` input
    particles, assigning each particle to a bucket based on the closest index
    particle, and then checking for close pairs in buckets such that they can
    be close enough.

    If we need something more involved, we can try
    [this paper](https://pubs.acs.org/doi/abs/10.1021/ci034150f)
 */
template <class LowerBound, class UpperBound>
class MetricClosePairsFinder : public core::ClosePairsFinder {
  LowerBound lb_;
  UpperBound ub_;

  IMP_NAMED_TUPLE_2(Data, Datas, ParticleIndexes, indexes, double,
                    width, );

  typedef boost::unordered_map<ParticleIndex, Data> Index;
  Index get_index(Model *m, ParticleIndexes inputs) const {
    unsigned int index_size = std::min<unsigned int>(
        1U, std::sqrt(static_cast<double>(inputs.size())));
    std::random_shuffle(inputs.begin(), inputs.end());
    Index ret;
    ParticleIndexes indexes(inputs.begin(),
                                    inputs.begin() + index_size);
    for (unsigned int i = 0; i < index_size; ++i) {
      ret[inputs[i]] = Data(ParticleIndexes(), 0.0);
    }
    IMP_LOG_VERBOSE("Index points are " << indexes << std::endl);
    for (unsigned int i = 0; i < inputs.size(); ++i) {
      double min_dist = std::numeric_limits<double>::max();
      ParticleIndex min_index;
      for (unsigned int j = 0; j < indexes.size(); ++j) {
        IMP_USAGE_CHECK(
            lb_(m, ParticleIndexPair(inputs[i], indexes[j])) <=
                ub_(m, ParticleIndexPair(inputs[i], indexes[j])),
            "The bounds are not ordered.");
        double cur_dist =
            ub_(m, ParticleIndexPair(inputs[i], indexes[j]));
        if (cur_dist < min_dist) {
          min_index = indexes[j];
          min_dist = cur_dist;
        }
      }
      ret[min_index].access_indexes().push_back(inputs[i]);
      ret[min_index].set_width(std::min(min_dist, ret[min_index].get_width()));
    }
    for (typename Index::const_iterator it = ret.begin(); it != ret.end();
         ++it) {
      IMP_LOG_VERBOSE(it->first << ": " << it->second << std::endl);
    }
    return ret;
  }
  ParticleIndexPairs get_close_pairs_internal(
      Model *m, const ParticleIndexes &p) const {
    ParticleIndexPairs ret;
    for (unsigned int i = 0; i < p.size(); ++i) {
      for (unsigned int j = 0; j < i; ++j) {
        IMP_USAGE_CHECK(lb_(m, ParticleIndexPair(p[i], p[j])) <=
                            ub_(m, ParticleIndexPair(p[i], p[j])),
                        "The bounds are not ordered.");
        if (lb_(m, ParticleIndexPair(p[i], p[j])) < get_distance()) {
          ret.push_back(ParticleIndexPair(p[i], p[j]));
        }
      }
    }
    return ret;
  }
  ParticleIndexPairs get_close_pairs_internal(
      Model *m, const ParticleIndexes &pa,
      const ParticleIndexes &pb) const {
    ParticleIndexPairs ret;
    for (unsigned int i = 0; i < pa.size(); ++i) {
      for (unsigned int j = 0; j < pb.size(); ++j) {
        IMP_USAGE_CHECK(lb_(m, ParticleIndexPair(pa[i], pb[j])) <=
                            ub_(m, ParticleIndexPair(pa[i], pb[j])),
                        "The bounds are not ordered.");
        if (lb_(m, ParticleIndexPair(pa[i], pb[j])) < get_distance()) {
          ret.push_back(ParticleIndexPair(pa[i], pb[j]));
        }
      }
    }
    return ret;
  }
  ParticleIndexPairs get_close_pairs_internal(Model *m,
                                                      const Index &ia,
                                                      const Index &ib,
                                                      bool is_same) const {
    ParticleIndexPairs ret;
    for (typename Index::const_iterator ita = ia.begin(); ita != ia.end();
         ++ita) {
      for (typename Index::const_iterator itb = ib.begin(); itb != ib.end();
           ++itb) {
        if (is_same) {
          if (ita->first < itb->first) continue;
          if (ita->first == itb->first) {
            ret += get_close_pairs_internal(m, ita->second.get_indexes());
            continue;
          }
        }
        IMP_USAGE_CHECK(
            lb_(m, ParticleIndexPair(ita->first, itb->first)) <=
                ub_(m, ParticleIndexPair(ita->first, itb->first)),
            "The bounds are not ordered.");
        if (lb_(m, ParticleIndexPair(ita->first, itb->first)) -
            ita->second.get_width() - itb->second.get_width()) {
          ret += get_close_pairs_internal(m, ita->second.get_indexes(),
                                          itb->second.get_indexes());
        }
      }
    }
    return ret;
  }

 public:
  MetricClosePairsFinder(LowerBound lb, UpperBound ub,
                         std::string name = "MetricClosePairsFinder%1%")
      : core::ClosePairsFinder(name), lb_(lb), ub_(ub) {}
  /** Not supported.*/
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &) const
      IMP_OVERRIDE {
    IMP_FAILURE("Bounding boxes are not supported.");
  }
  /** Not supported.*/
  virtual IntPairs get_close_pairs(const algebra::BoundingBox3Ds &,
                                   const algebra::BoundingBox3Ds &) const
      IMP_OVERRIDE {
    IMP_FAILURE("Bounding boxes are not supported.");
  }

  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pc) const IMP_OVERRIDE {
    IMP_OBJECT_LOG;
    if (pc.empty()) return ParticleIndexPairs();
    Index index = get_index(m, pc);
    return get_close_pairs_internal(m, index, index, true);
  }
  virtual ParticleIndexPairs get_close_pairs(
      Model *m, const ParticleIndexes &pca,
      const ParticleIndexes &pcb) const IMP_OVERRIDE {
    IMP_OBJECT_LOG;
    if (pca.empty() || pcb.empty()) return ParticleIndexPairs();
    Index indexa = get_index(m, pca);
    Index indexb = get_index(m, pcb);
    return get_close_pairs_internal(m, indexa, indexb, false);
  }
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE {
    // for now assume we just read the particles
    return IMP::get_particles(m, pis);
  }

  IMP_OBJECT_METHODS(MetricClosePairsFinder);
};

/** Create a new MetricClosePairsFinder, handling types. */
template <class LowerBound, class UpperBound>
core::ClosePairsFinder *create_metric_close_pairs_finder(LowerBound lb,
                                                         UpperBound ub) {
  return new MetricClosePairsFinder<LowerBound, UpperBound>(lb, ub);
}

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_METRIC_CLOSE_PAIRS_FINDER_H */
