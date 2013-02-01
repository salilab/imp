/**
 *  \file atom/SecondaryStructureResidue.cpp
 *  \brief A decorator for SSE Residues.
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/SecondaryStructureResidue.h>
#include <math.h>

IMPATOM_BEGIN_NAMESPACE

FloatKey SecondaryStructureResidue::get_prob_helix_key(){
  static FloatKey k("prob_helix");
  return k;
}
FloatKey SecondaryStructureResidue::get_prob_strand_key(){
  static FloatKey k("prob_strand");
  return k;
}
FloatKey SecondaryStructureResidue::get_prob_coil_key(){
  static FloatKey k("prob_coil");
  return k;
}

void SecondaryStructureResidue::show(std::ostream &out) const {
  out << "SecondaryStructureResidue with:\n"
      <<"prob helix: "<<get_prob_helix()
      <<" prob strand: "<<get_prob_strand()
      <<" prob coil: "<<get_prob_coil();
}

SecondaryStructureResidue get_coarse_ssr(const Particles &ssr_ps,
                                         Model *mdl,
                                         bool winner_takes_all_per_res){
  Floats scores;
  scores.push_back(0.0);
  scores.push_back(0.0);
  scores.push_back(0.0);
  int count=0;
  for (Particles::const_iterator p=ssr_ps.begin();p!=ssr_ps.end();++p){
    IMP_USAGE_CHECK(SecondaryStructureResidue::particle_is_instance(*p),
                    "all particles must be SecondaryStructureResidues");
    SecondaryStructureResidue ssr(*p);
    Floats tmp_scores;
    tmp_scores.push_back(ssr.get_prob_helix());
    tmp_scores.push_back(ssr.get_prob_strand());
    tmp_scores.push_back(ssr.get_prob_coil());
    int max_i=0;
    Float max=0.0;
    for (int i=0;i<3;i++){
      if (tmp_scores[i]>max){
        max=tmp_scores[i];
        max_i=i;
      }
      if (!winner_takes_all_per_res) scores[i]+=tmp_scores[i];
    }
    if (winner_takes_all_per_res) scores[max_i]+=1.0;
    count++;
  }
  IMP_NEW(Particle,coarse_p,(mdl));
  SecondaryStructureResidue ssres=
    SecondaryStructureResidue::setup_particle(coarse_p,
                                              scores[0]/count,
                                              scores[1]/count,
                                              scores[2]/count);
  return ssres;
}

Float get_match_score(Particle * ssr1, Particle * ssr2){
  IMP_USAGE_CHECK(SecondaryStructureResidue::particle_is_instance(ssr1),
                    "both particles must be SecondaryStructureResidues");
  IMP_USAGE_CHECK(SecondaryStructureResidue::particle_is_instance(ssr2),
                    "both particles must be SecondaryStructureResidues");
  Floats f1=SecondaryStructureResidue(ssr1).get_all_probabilities();
  Floats f2=SecondaryStructureResidue(ssr2).get_all_probabilities();
  Float sum1=0.0,sum2=0.0,sum12=0.0,sum11=0.0,sum22=0.0;
  for (int i=0;i<3;i++){
    sum1+=f1[i];
    sum2+=f2[i];
    sum12+=f1[i]*f2[i];
    sum11+=f1[i]*f1[i];
    sum22+=f2[i]*f2[i];
  }
  Float pearson_corr=3*sum12-sum1*sum2;
  pearson_corr/=std::sqrt(3*sum11-sum1*sum1);
  pearson_corr/=std::sqrt(3*sum22-sum2*sum2);
  return pearson_corr;
}

IMPATOM_END_NAMESPACE
