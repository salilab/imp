/**
 *  \file isd/MarginalNOERestraint.cpp
 *  \brief Restrain a list of particle pairs with a lognormal+ISPA.
 *  NOTE: for now, the derivatives are written to all variables.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/MarginalNOERestraint.h>
#include <IMP/core/XYZ.h>
#include <math.h>
#include <IMP/PairContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container_macros.h>

IMPISD_BEGIN_NAMESPACE

//MarginalNOERestraint::MarginalNOERestraint() {}

// add a contribution: simple case
void MarginalNOERestraint::add_contribution(kernel::Particle *p1, kernel::Particle *p2,
                                            double Iexp)
{
    kernel::ParticlePair pc(p1,p2);
    kernel::ParticlePairsTemp pct(1,pc);
    IMP_NEW(container::ListPairContainer, cont, (pct));
    //container::ListPairContainer cont(pct);
    add_contribution(cont,Iexp);
}

//add a contribution: general case
void MarginalNOERestraint::add_contribution(PairContainer *pc, double Iexp)
{
    contribs_.push_back(pc);
    volumes_.push_back(Iexp);
}

double
MarginalNOERestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
    //compute gammahat and store distances
    double loggammahat=1;
    base::Vector<double> meandists; //mean distances^-6, length(volumes_)
    //store interparticle distances^-6
    base::Vector<base::Vector<double> > alldists;
    int ncontribs = volumes_.size();
    for (int i=0; i< ncontribs; ++i) //loop on all contributions
    {
        double mean=0;
        base::Vector<double> dists;
        IMP_CONTAINER_FOREACH(PairContainer, contribs_[i],
                              {
            core::XYZ d0(get_model(), _1[0]);
            core::XYZ d1(get_model(), _1[1]);
            double dist = (d1.get_coordinates()
                           - d0.get_coordinates()).get_squared_magnitude();
            dist = 1.0/cube(dist);
            mean += dist;
            if (accum) {
              dists.push_back(dist);
            }
                              });
        meandists.push_back(mean);
        if (accum) alldists.push_back(dists);
        loggammahat += log(volumes_[i]/mean);
    }
    loggammahat = loggammahat / ncontribs;
    const_cast<MarginalNOERestraint *>(this)->set_log_gammahat(loggammahat);

    //compute SS
    double SS = 0;
    base::Vector<double> logterms;
    for (int i=0; i < ncontribs; ++i)
    {
        double val = log(volumes_[i]/meandists[i])-loggammahat_;
        SS += square(val);
        logterms.push_back(val);
    }
    const_cast<MarginalNOERestraint *>(this)->set_SS(SS);
    double score = log(SS)*(ncontribs -1)/2.0;

    if (accum)
    {
        for (int i=0; i<ncontribs; ++i)
        {
            double deriv_mean = logterms[i]*6*(ncontribs - 1)/SS;
            IMP_CONTAINER_FOREACH(PairContainer, contribs_[i],
                                  {
                core::XYZ d0(get_model(), _1[0]);
                core::XYZ d1(get_model(), _1[1]);
                double deriv_pair = alldists[i][_2]/meandists[i];
                if (std::abs(deriv_pair) > 1e2) {
                  std::cout << "NOE derivative warning : deriv mean "
                            << deriv_mean << " pair " << deriv_pair
                            << std::endl;
                }
                algebra::Vector3D dev = (d1.get_coordinates()
                                         - d0.get_coordinates());
                double dist = dev.get_squared_magnitude();
                algebra::Vector3D deriv = deriv_mean * deriv_pair * dev / dist;
                d1.add_to_derivatives(deriv, *accum);
                d0.add_to_derivatives(-deriv, *accum);
                                  });
        }
    }
    return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp MarginalNOERestraint::do_get_inputs() const
{
  ModelObjectsTemp ret;
  for (unsigned i=0; i<volumes_.size(); ++i)
  {
    ret+= IMP::get_particles(get_model(),
                             contribs_[i]->get_all_possible_indexes());
  }
  for (unsigned i=0; i<volumes_.size(); ++i)
  {
      ret.push_back(contribs_[i]);
  }
  return ret;
}

IMPISD_END_NAMESPACE
