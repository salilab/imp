/**
 *  \file IMP/pmi/CompositeRestraint.h
 *  \brief A restraint for ambiguous cross-linking MS data and multiple
 *         state approach.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPPMI_COMPOSITE_RESTRAINT_H
#define IMPPMI_COMPOSITE_RESTRAINT_H
#include "pmi_config.h"
#include <IMP/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/tuple_macros.h>
//#include <IMP/container/CloseBipartitePairContainer.h>

IMPPMI_BEGIN_NAMESPACE
//! A restraint for ambiguous cross-linking MS data and multiple state approach.
/** It marginalizes the false positive rate and depends on the expected fpr and
    an uncertainty parameter beta.
 */
class IMPPMIEXPORT CompositeRestraint : public Restraint
{
    //particle indexes in the composite
    Vector<ParticleIndexes> pis_;
    ParticleIndexes handle_particle_indexes_;
    double coffd_;
    double l_;
    IMP_NAMED_TUPLE_2(CacheKey, CacheKeys,
                  Int, ipart, Ints, excluded, );

    IMP_NAMED_TUPLE_2(CacheKeyPot, CacheKeyPots,
                  Int, ipart, Int, kpart, );

    typedef boost::unordered_map<CacheKey, double> Cache;
    typedef boost::unordered_map<CacheKeyPot, double> CachePot;

    //variables needed to tabulate the exponential
    Floats prob_grid_;
    double invdx_;
    double argmax_;
    double argmin_;
    bool tabprob_;
    double plateau_;
    int exparg_grid_size_;

    inline double calc_prob (double dist) const{
      double argvalue=(dist-coffd_)/l_;
      double prob;
      if (tabprob_){
         //this prevents something being below the lower value of the array
         double maxarg=std::max(argvalue,argmin_);
         //this prevents something being above the upper value of the array
         double minarg=std::min(maxarg,argmax_);
         unsigned k = static_cast<unsigned>( std::floor(minarg*invdx_)+exparg_grid_size_);
         prob=prob_grid_[k];

      }
      else{
         prob=(1.0-plateau_)/(1.0+std::exp(-argvalue));
      }
      return prob;
    }


    //boost::unordered_map<std::tuple<unsigned int,unsigned int>,
    //          Pointer<container::CloseBipartitePairContainer>> map_cont_;

  /* call for probability */
  double get_probability_per_particle_excluding(unsigned int ipart,
                                    Ints excluded_ps, Cache& cache, CachePot& cachepot) const;

public:


  //! Create the restraint.
  CompositeRestraint(Model *m,
                     ParticleIndexesAdaptor handle_particle_indexes,
                     double coffd, double l, bool tabprob, double plateau,
                     std::string name="CompositeRestraint%1%");

  void add_composite_particle(ParticleIndexesAdaptor pi){pis_.push_back(pi);}


  unsigned int get_number_of_elements() const {return pis_.size();}



  //double get_probability() const {return 0.0;}

  virtual double
  unprotected_evaluate(DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(CompositeRestraint);

  virtual double get_probability() const
  {
    return exp(-unprotected_evaluate(nullptr));
  }


};

IMPPMI_END_NAMESPACE

#endif  /* IMPPMI_COMPOSITE_RESTRAINT_H */
