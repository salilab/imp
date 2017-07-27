/**
 *  \file IMP/isd/distribution.h
 *  \brief Base class for probability distributions
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPISD_DISTRIBUTION_H
#define IMPISD_DISTRIBUTION_H

#include <IMP/isd/isd_config.h>
#include <IMP/macros.h>
#include <IMP/Model.h>

IMPISD_BEGIN_NAMESPACE

//! Base class for all distributions provided for common inheritance.
/** This base class exists to define a common parent for all distribution types.
    Children are expected to define one or more evaluate() and get_density()
    methods taking an arbitrary number of arguments to provide negative
    log-density and probability density, respectively.
 */
class IMPISDEXPORT Distribution : public Object {
  public:
    Distribution(std::string name = "Distribution %1%")
      : Object(name) {}

  IMP_OBJECT_METHODS(Distribution);
};
IMP_OBJECTS(Distribution, Distributions);


//! Base class for distributions that are passed a single random variable.
/** Children must overload do_evaluate().
 */
class IMPISDEXPORT OneDimensionalDistribution : public Distribution {
  protected:
    virtual double do_evaluate(double v) = 0;
    virtual double do_get_density(double v);

  public:
    OneDimensionalDistribution(
      std::string name = "OneDimensionalDistribution %1%"): Distribution(name) {
    }

    //! Get negative log-density for passed variable value.
    double evaluate(double v) { return do_evaluate(v); }

    //! Get probability density for passed variable value.
    double get_density(double v) { return do_get_density(v); }

  IMP_OBJECT_METHODS(OneDimensionalDistribution);
};
IMP_OBJECTS(OneDimensionalDistribution, OneDimensionalDistributions);


//! Base class for single-variate distributions that cache sufficient statistics.
/** Sufficient statistics are one or more statistics whose values are sufficient
    to describe the entire distribution. For the joint probability of many
    independent draws from a distribution, the set of all drawn values is
    a sufficient statistic, but minimal sufficient statistics (such as the
    mean and various for a normal distribution) often exist, permitting one-time
    calculation and efficient use of memory.

    Children must overload do_update_sufficient_statistics() and do_evaluate().
 */
class IMPISDEXPORT OneDimensionalSufficientDistribution : public Distribution {
  protected:
    virtual void do_update_sufficient_statistics(Floats data) = 0;
    virtual double do_evaluate() const = 0;
    virtual double do_get_density() const;

  public:
    //! Constructor
    OneDimensionalSufficientDistribution(
      std::string name = "OneDimensionalSufficientDistribution %1%")
    : Distribution(name) {}

    //! Update cached sufficient statistics from data.
    void update_sufficient_statistics(Floats vs) {
      do_update_sufficient_statistics(vs);
    }

    //! Get negative log-density using cached sufficient statistics.
    double evaluate() const { return do_evaluate(); }

    //! Get probability density using cached sufficient statistics.
    double get_density() const { return do_get_density(); }

  IMP_OBJECT_METHODS(OneDimensionalSufficientDistribution);
};
IMP_OBJECTS(OneDimensionalSufficientDistribution, OneDimensionalSufficientDistributions);

IMPISD_END_NAMESPACE

#endif /* IMPISD_DISTRIBUTION_H */
