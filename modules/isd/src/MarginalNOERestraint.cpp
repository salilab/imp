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

IMPISD_BEGIN_NAMESPACE

//MarginalNOERestraint::MarginalNOERestraint() {}

// add a contribution: simple case
void MarginalNOERestraint::add_contribution(Particle *p1, Particle *p2,
                                            double Iexp)
{
    ParticlePair pc(p1,p2);
    ParticlePairsTemp pct(1,pc);
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
    std::vector<double> meandists; //mean distances^-6, length(volumes_)
    //store interparticle distances^-6
    std::vector<std::vector<double> > alldists;
    int ncontribs = volumes_.size();
    for (int i=0; i< ncontribs; ++i) //loop on all contributions
    {
        int npairs = contribs_[i]->get_number_of_particle_pairs();
        double mean=0;
        std::vector<double> dists;
        for (int p=0; p<npairs; ++p)//loop on all pairs of contribution i
        {
            ParticlePair pair = contribs_[i]->get_particle_pair(p);
            core::XYZ d0(pair[0]),d1(pair[1]);
            double dist = (d1.get_coordinates()
                           - d0.get_coordinates()).get_squared_magnitude();
            dist = 1.0/cube(dist);
            mean += dist;
            if (accum) dists.push_back(dist);
        }
        meandists.push_back(mean);
        if (accum) alldists.push_back(dists);
        loggammahat += log(volumes_[i]/mean);
    }
    loggammahat = loggammahat / ncontribs;
    const_cast<MarginalNOERestraint *>(this)->set_log_gammahat(loggammahat);

    //compute SS
    double SS = 0;
    std::vector<double> logterms;
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
            int npairs = contribs_[i]->get_number_of_particle_pairs();
            for (int p=0; p<npairs; ++p)
            {
                ParticlePair pair = contribs_[i]->get_particle_pair(p);
                double deriv_pair = alldists[i][p]/meandists[i];
                if (std::abs(deriv_pair) > 1e2) {
                  std::cout << "NOE derivative warning : deriv mean "
                            << deriv_mean << " pair " << deriv_pair
                            << std::endl;
                }
                core::XYZ d0(pair[0]),d1(pair[1]);
                algebra::Vector3D dev = (d1.get_coordinates()
                                         - d0.get_coordinates());
                double dist = dev.get_squared_magnitude();
                algebra::Vector3D deriv = deriv_mean * deriv_pair * dev / dist;
                d1.add_to_derivatives(deriv, *accum);
                d0.add_to_derivatives(-deriv, *accum);
            }
        }
    }
    return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ParticlesTemp MarginalNOERestraint::get_input_particles() const
{
  ParticlesTemp ret;
  for (unsigned i=0; i<volumes_.size(); ++i)
  {
      int npairs = contribs_[i]->get_number_of_particle_pairs();
      for (int j=0; j < npairs; ++j)
      {
          ret.push_back(contribs_[i]->get_particle_pair(j)[0]);
          ret.push_back(contribs_[i]->get_particle_pair(j)[1]);
      }
  }
  return ret;
}

ContainersTemp MarginalNOERestraint::get_input_containers() const
{
  ContainersTemp ret;
  for (unsigned i=0; i<volumes_.size(); ++i)
  {
      ret.push_back(contribs_[i]);
  }
  return ret;
}

void MarginalNOERestraint::do_show(std::ostream& out) const
{
  out << "Marginal NOE restraint with " << volumes_.size()
      << " contributions " << std::endl;
}

IMPISD_END_NAMESPACE
