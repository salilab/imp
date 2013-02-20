/**
 *  \file atom/SecondaryStructureResidue.cpp
 *  \brief A decorator for storing secondary structure probabilities.
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

SecondaryStructureResidue setup_coarse_secondary_structure_residue(
                                         const Particles &ssr_ps,
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

SecondaryStructureResidues setup_coarse_secondary_structure_residues(
                                           const Particles &ssr_ps,
                                           Model *mdl,
                                           int coarse_factor,
                                           int start_res_num,
                                           bool winner_takes_all_per_res){
  /* We're presuming that the coarsening starts from 0.
      So if start_res_num%coarse_factor<coarse_factor/2, this
       set starts with the majority of a node.
      Likewise if start_res_num%coarse_factor>=coarse_factor/2, this
       set starts with the minority of a node, so we ignore the first few
       particles.
      Same thing for the end! If has a little tail, ignore last few residues.
   */
  SecondaryStructureResidues ssrs;
  int start_idx=0,stop_idx=(int)ssr_ps.size();
  if (start_res_num%coarse_factor>=float(coarse_factor)/2){
    start_idx=coarse_factor-start_res_num%coarse_factor;
  }
  if ((stop_idx+start_res_num)%coarse_factor<float(coarse_factor)/2){
    stop_idx-=(stop_idx+start_res_num)%coarse_factor;
  }

  //now start grouping
  int prev_coarse=(start_idx+start_res_num)/coarse_factor;
  Particles tmp_ps;
  for (int nr=start_idx;nr<stop_idx;nr++){
    int this_coarse=(nr+start_res_num)/coarse_factor;
    if (this_coarse!=prev_coarse){
      ssrs.push_back(setup_coarse_secondary_structure_residue(
                                        tmp_ps,mdl,winner_takes_all_per_res));
      tmp_ps.clear();
    }
    tmp_ps.push_back(ssr_ps[nr]);
    prev_coarse=this_coarse;
  }
  if (tmp_ps.size()>0) {
    ssrs.push_back(setup_coarse_secondary_structure_residue(
                                        tmp_ps,mdl,winner_takes_all_per_res));
  }
  return ssrs;
}

Float get_secondary_structure_match_score(SecondaryStructureResidue ssr1,
                                          SecondaryStructureResidue ssr2){
  Floats f1=ssr1.get_all_probabilities();
  Floats f2=ssr2.get_all_probabilities();
  Float rmsd=0.0;
  for (int i=0;i<3;i++){
    rmsd+=(f1[i]-f2[i])*(f1[i]-f2[i]);
  }
  return std::sqrt(rmsd);
}

IMPATOM_END_NAMESPACE
