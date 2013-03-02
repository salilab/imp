/**
 *  \file proteomics_em_alignment_atomic.cpp
 *  \brief align proteomics graph to em density map
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/RadiusOfGyrationRestraint.h>
#include <IMP/container/CloseBipartitePairContainer.h>
#include <IMP/core/ExcludedVolumeRestraint.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/multifit/proteomics_em_alignment_atomic.h>
#include <IMP/multifit/merge_tree_utils.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/multifit/fitting_states.h>
#include <IMP/multifit/fitting_solutions_reader_writer.h>
#include <IMP/domino/particle_states.h>
#include <IMP/domino/BranchAndBoundSampler.h>
#include <IMP/domino/assignment_tables.h>
#include <IMP/domino/subset_filters.h>
#include <IMP/domino/particle_states.h>
#include <IMP/atom/Selection.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/em/PCAFitRestraint.h>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace {
#if 0
  Restraint * add_core_ev_restraint(atom::Hierarchy mh1,
                               atom::Hierarchy mh2,
                               const AlignmentParams &params) {
    Model *mdl=mh1->get_model();
    IMP_NEW(container::ListSingletonContainer,lsc,(mdl));
    atom::AtomTypes atom_types;
    atom_types.push_back(atom::AtomType("CA"));
    //make a container of ca atoms of the first molecule
    atom::Hierarchies mh1_res=atom::get_by_type(mh1,atom::RESIDUE_TYPE);
    atom::Selection s1(mh1_res);
    s1.set_atom_types(atom_types);
    lsc->add_particles(s1.get_selected_particles());
    //make a container of ca atoms of the second molecule
    atom::Hierarchies mh2_res=atom::get_by_type(mh2,atom::RESIDUE_TYPE);
    atom::Selection s2(mh2_res);
    s2.set_atom_types(atom_types);
    lsc->add_particles(s2.get_selected_particles());
    float slack=params.get_ev_params().pair_slack_;
    float k=params.get_ev_params().hlb_k_;
    IMP_NEW(core::ExcludedVolumeRestraint,evr,(lsc,k,slack));
    return evr.release();
    }
#endif

#if 0
  Restraint * add_ev_restraint(atom::Hierarchy mh1,
                               atom::Hierarchy mh2,
                               const AlignmentParams &params) {
    Model *mdl=mh1->get_model();
    atom::AtomTypes atom_types;
    atom_types.push_back(atom::AtomType("CA"));
    //make a container of ca atoms of the first molecule
    atom::Hierarchies mh1_res=atom::get_by_type(mh1,atom::RESIDUE_TYPE);
    atom::Selection s1(mh1_res);
    s1.set_atom_types(atom_types);
    IMP_NEW(container::ListSingletonContainer,lsc1,(mdl));
    lsc1->add_particles(s1.get_selected_particles());
    //make a container of ca atoms of the second molecule
    atom::Hierarchies mh2_res=atom::get_by_type(mh2,atom::RESIDUE_TYPE);
    atom::Selection s2(mh2_res);
    s2.set_atom_types(atom_types);
    IMP_NEW(container::ListSingletonContainer,lsc2,(mdl));
    lsc2->add_particles(s2.get_selected_particles());
    std::cout<<"Number of particles for "<<mh1->get_name()<<"is "
             <<s1.get_selected_particles().size()<<std::endl;
    std::cout<<"Number of particles for "<<mh2->get_name()<<"is "
             <<s2.get_selected_particles().size()<<std::endl;
    float distance=params.get_ev_params().pair_distance_;
    float slack=params.get_ev_params().pair_slack_;
    // IMP_NEW(container::CloseBipartitePairContainer,nbl,
    //         (lsc1,lsc2,distance, slack));

    //====== use this container although it is clear this is the not
    //       the right one
    IMP_NEW(container::ListSingletonContainer,lsc3,(mdl));
    lsc3->add_particles(s1.get_selected_particles());
    lsc3->add_particles(s2.get_selected_particles());
    IMP_NEW(container::ClosePairContainer,nbl,
            (lsc3,distance, slack));
    //======

    float mean=params.get_ev_params().hlb_mean_;
    float k=params.get_ev_params().hlb_k_;
    /*the score value is:
      distance<mean: 0
      distance>mean: 0.5*k*(distance-mean)*(distance-mean)
    */
    IMP_NEW(core::HarmonicLowerBound,h,(mean,k));
    //distance is defined as the distance between the surfaces of the spheres
    IMP_NEW(core::SphereDistancePairScore,sd,(h));
    //use the lower bound on the inter-sphere distance to push the spheres apart
    IMP_NEW(container::PairsRestraint,nbr,(sd,nbl));
    return nbr.release();
  }
#endif

  std::string get_pair_key(int ind1,int ind2) {
    std::stringstream ss;
    ss<<std::min(ind1,ind2)<<"_"<<std::max(ind1,ind2);
    return ss.str();
  }

  // infer using heap table container
  domino::HeapAssignmentContainer* get_assignments(
      const domino::MergeTree &jt,
      int vertex_ind,
      domino::DominoSampler *ds,
      unsigned int k,
      domino::RestraintCache *rc,
      domino::RestraintScoreSubsetFilterTable *rssft){
    std::cout<<"======== getting assignments for vertex:"<<vertex_ind
             <<std::endl;
    typedef boost::graph_traits<domino::MergeTree>::adjacency_iterator
      NeighborIterator;
    std::cout<<"======== 1"<<std::endl;
    typedef boost::property_map<domino::MergeTree,
                                boost::vertex_name_t>::const_type SubsetMap;
    std::cout<<"======== 2"<<std::endl;
    SubsetMap subset_map= boost::get(boost::vertex_name, jt);
    std::cout<<"======== 3"<<std::endl;
    std::pair<NeighborIterator, NeighborIterator> be
      = boost::adjacent_vertices(vertex_ind, jt);
    std::cout<<"======== 4"<<std::endl;
    IMP_NEW(domino::HeapAssignmentContainer,hac,
            (boost::get(subset_map,vertex_ind),k,rc));
    std::cout<<"======== 5"<<std::endl;
    if (std::distance(be.first, be.second)==0) {
      //the vertex is a leaf
      std::cout<<"======== vertex:"<<vertex_ind
               <<" is a leaf. loading assignments"<<std::endl;

      ds->load_vertex_assignments(vertex_ind,hac);
    } else {
      int firsti=*be.first;
      int secondi=*(++be.first);
      if (firsti>secondi) {
        int temp=firsti;
        firsti=secondi;
        secondi=temp;
      }
      //recurse on the two children
      Pointer<domino::HeapAssignmentContainer> a0=
        get_assignments(jt,firsti,ds,k,rc,rssft);
      Pointer<domino::HeapAssignmentContainer> a1=
        get_assignments(jt,secondi,ds,k,rc,rssft);
      if ((a0->get_number_of_assignments()==0) ||
          (a1->get_number_of_assignments()==0)) {
        std::cout<<"========== For vertex "<< vertex_ind
                 <<" one of the children has 0 assignments, returning "
                 <<"empty container"<<std::endl;
        return hac.release();
      }
      ds->load_vertex_assignments(vertex_ind, a0, a1, hac);
    }
    std::cout<<"========== For vertex "<< vertex_ind
             <<" number of assignments "<<hac->get_number_of_assignments()
             <<std::endl;
    return hac.release();
  }


  algebra::ReferenceFrame3Ds get_reference_frames(
                const multifit::FittingSolutionRecords &fits,
                core::RigidBody rb) {
    algebra::ReferenceFrame3Ds ret;
    std::cout<<"get reference frames for particle:"
             <<rb.get_particle()->get_name()<<std::endl;
    for(int i=0;i<(int)fits.size();i++) {
      std::cout<<"i:"<<i<<" ";
      fits[i].get_fit_transformation().show();
      std::cout<<std::endl;
      core::transform(rb,fits[i].get_fit_transformation());
      ret.push_back(rb.get_reference_frame());
      core::transform(rb,fits[i].get_fit_transformation().get_inverse());
    }
    return ret;
  }
}//end anonymous namespace

//TODO - do not use ProteinsAnchorsSamplingSpace.
//you are not going to use the paths here
ProteomicsEMAlignmentAtomic::ProteomicsEMAlignmentAtomic(
                   const ProteinsAnchorsSamplingSpace &mapping_data,
                   multifit::SettingsData *asmb_data,
                   const AlignmentParams &align_param) :
  base::Object("ProteomicsEMAlignmentAtomic%1%"),
  mapping_data_(mapping_data),
  params_(align_param),
  order_key_(IntKey("order")),
  asmb_data_(asmb_data){
  fast_scoring_=false;
  std::cout<<"start"<<std::endl;

  std::cout<<"here0.2\n";
  //initialize everything
  mdl_=new Model();
  IMP_LOG_VERBOSE("get proteomics data\n");
  std::cout<<"get proteomics data\n";
  prot_data_=mapping_data_.get_proteomics_data();
  fit_state_key_ = IntKey("fit_state_key");
  load_atomic_molecules();
  std::cout<<"here1"<<std::endl;
  IMP_LOG_VERBOSE("set NULL \n");
  pst_=nullptr;
  restraints_set_=false;states_set_=false;filters_set_=false;
  ev_thr_=0.001;//TODO make a parameter
  IMP_LOG_VERBOSE("end initialization\n");
}

void ProteomicsEMAlignmentAtomic::load_atomic_molecules(){
  IMP_LOG_TERSE("load atomic molecules \n");
    IMP_NEW(atom::ATOMPDBSelector,sel,());
  //  IMP_NEW(atom::CAlphaPDBSelector,sel,());
  for(int i=0;i<prot_data_->get_number_of_proteins();i++) {
    IMP_LOG_TERSE("going to load molecule "<<
            asmb_data_->get_component_header(i)->get_filename()<<"\n");
    //            prot_data_.get_protein_filename(i)<<"|\n";
    atom::Hierarchy mh =
      atom::read_pdb(asmb_data_->get_component_header(i)->get_filename(),
                     mdl_,sel);
    mh->set_name(asmb_data_->get_component_header(i)->get_name());
    mh->set_was_used(true);
    mhs_.push_back(mh);
    std::cout<<"create pdb"<<std::endl;
    std::cout<<"are subunits rigid?"
             <<params_.get_fragments_params().subunit_rigid_<<std::endl;
    if (params_.get_fragments_params().subunit_rigid_) {
      std::cout<<"create rigid body"<<std::endl;
      rbs_.push_back(atom::create_rigid_body(mh));
      rbs_[rbs_.size()-1]->set_name(mh->get_name());
      rbs_[rbs_.size()-1]->add_attribute(fit_state_key_,-1);
      rbs_[rbs_.size()-1]->add_attribute(order_key_,i);

    }
  }
}

domino::ParticleStatesTable*
  ProteomicsEMAlignmentAtomic::set_particle_states_table(
                                domino::SubsetFilterTables &/*filters*/) {
  IMP_NEW(domino::ParticleStatesTable,pst,());
  for(int i=0;i<prot_data_->get_number_of_proteins();i++){
    IMP_LOG_TERSE(
            "working on protein:"<<prot_data_->get_protein_name(i)<<std::endl);
    multifit::FittingSolutionRecords all_fits =
      multifit::read_fitting_solutions(asmb_data_->get_component_header(i)
                                          ->get_transformations_fn().c_str());
    IMP_LOG_VERBOSE(
            "number of fitting solutions:"<<all_fits.size()<<std::endl);
    //get relevant fits
    multifit::FittingSolutionRecords fits;
    IntsList fit_inds =
      mapping_data_.get_paths_for_protein(prot_data_->get_protein_name(i));
    IMP_LOG_VERBOSE(
            "number of relevant fits found:"<<fit_inds.size()<<std::endl);
    std::cout<<"The number of fits found for :"
             <<prot_data_->get_protein_name(i)<<" is "<< fit_inds.size()
             <<std::endl;
    for(int j=0;j<(int)fit_inds.size();j++) {
      fits.push_back(all_fits[fit_inds[j][0]]);
    }
    IMP_NEW(FittingStates,rb_states,
            (get_reference_frames(fits,rbs_[i]),fit_state_key_));
    pst->set_particle_states(rbs_[i],rb_states);
  }
  return pst.release();
}


RestraintsTemp ProteomicsEMAlignmentAtomic::get_alignment_restraints() const {
  RestraintsTemp ret;
  /*
  domino::Subset s(pst_->get_particles());
  domino::Subsets ss;
  domino::RestraintScoreSubsetFilter *rssf=
    all_rs_filt_->get_subset_filter(s,ss);
  RestraintsTemp rs = rssf->get_restraints();
  for(int i=0;i<(int)rs.size();i++){
    RestraintSet *rset=dynamic_cast<RestraintSet*>(rs[i]);
    if (rset) {
      for(Restraints::iterator it = rset->restraints_begin();
          it != rset->restraints_end();it++) {
        ret.push_back(*it);
      }
    }
    else {
      ret.push_back(rs[i]);
    }
    }*/
  return ret;
}

void ProteomicsEMAlignmentAtomic::show_scores_header(std::ostream& out) const {
  RestraintsTemp rs = get_alignment_restraints();
  for(int i=0;i<(int)rs.size();i++){
      out<<rs[i]->get_name()<<"|";
  }
  out<<std::endl;
}
/*
void ProteomicsEMAlignmentAtomic::show_scores(const domino::Assignment &a,
                                              std::ostream& out) const {
  int num_violations=0;
  domino::Subset s(pst_->get_particles());
  domino::Subsets ss;
  domino::RestraintScoreSubsetFilter *rssf
                    =all_rs_filt_->get_subset_filter(s,ss);
  RestraintsTemp rs=get_alignment_restraints();
  Floats scores = rssf->get_scores(a);
  for(int i=0;i<(int)scores.size();i++){
    out<<scores[i]<<"|";
    if (scores[i]>rs[i]->get_maximum_score()) {
      num_violations+=1;
    }
  }
  out<<"|"<<num_violations<<"|"<<std::accumulate(scores.begin(),
                                                 scores.end(),0.)<<std::endl;
}
*/


void ProteomicsEMAlignmentAtomic::show_domino_merge_tree() const {
  std::cout<<"domino merge tree"<<std::endl;
  domino::SubsetGraph jt =
    domino::get_junction_tree(
                              domino::get_interaction_graph(jt_rs_,
                                                            pst_));
  DependencyGraph dg = get_dependency_graph(mdl_);
  domino::MergeTree mt
    = domino::get_balanced_merge_tree(jt);
  base::internal::show_as_graphviz(mt,std::cout);
}


void ProteomicsEMAlignmentAtomic::align(){
  std::cout<<"=============1"<<std::endl;
  IMP_USAGE_CHECK(states_set_&&filters_set_&&restraints_set_,
                  "restraints, filters and states are not set \n");
  algebra::ReferenceFrame3Ds orig_rf;
  std::cout<<"=============2"<<std::endl;
  for (int i=0;i<(int)rbs_.size();i++) {
    orig_rf.push_back(rbs_[i].get_reference_frame());
  }
  std::cout<<"=============3"<<std::endl;
  IMP_NEW(domino::DominoSampler,ds,(mdl_,pst_));
  ds->set_was_used(true);
  std::cout<<"=============4"<<std::endl;
  //  IMP_NEW(domino::BranchAndBoundSampler,ds,(mdl_,pst));
  IMP_LOG_VERBOSE("going to sample\n");
  Particles ps;
  for(int i=0;i<(int)mhs_.size();i++){
    ParticlesTemp temp=core::get_leaves(mhs_[i]);
    ps.insert(ps.end(),temp.begin(),temp.end());
  }
  std::cout<<"=============5 number of restraints:"<<jt_rs_.size()<<std::endl;
  //filter by ev
  //first create all the additional restraints
  domino::SubsetGraph jt =
    domino::get_junction_tree(
                              domino::get_interaction_graph(jt_rs_,
                                                            pst_));
  std::cout<<"=============6"<<std::endl;
  base::internal::show_as_graphviz(jt,std::cout);
  DependencyGraph dg = get_dependency_graph(mdl_);
  base::internal::show_as_graphviz(dg,std::cout);
  std::cout<<"merge tree"<<std::endl;
  domino::MergeTree mt
    = domino::get_balanced_merge_tree(jt);
  base::internal::show_as_graphviz(mt,std::cout);
  ds->set_merge_tree(mt);//remove for non interative
  ds->set_subset_filter_tables(filters_);
  std::cout<<"Number of filters:"<<filters_.size()<<std::endl;
  domino::Subset s(pst_->get_particles());
  std::cout<<"Particles in subset:"<<std::endl;
  for(unsigned int i=0;i<s.size();i++) {
    std::cout<<s[i]->get_name()<<std::endl;
  }
  std::cout<<"=======1"<<std::endl;
  //  sampled_assignments_ =
  //    ds->DiscreteSampler::get_sample_assignments(s);//pst_->get_particles());
  all_rs_filt_ = new domino::RestraintScoreSubsetFilterTable(mdl_,pst_);
  std::cout<<"=======2"<<std::endl;
  //  all_rs_filt_->set_use_caching(true);
  std::cout<<"============3"<<std::endl;
  //  all_rs_filt_->set_maximum_number_of_cache_entries(
  //                            params_.get_domino_params().cache_size_);
  std::cout<<"before get assignments"<<std::endl;
  //  IMP_NEW(domino::RestraintCache, rc_, (pst_));
  RestraintsTemp mdl_rs;
  for(unsigned i=0;i<mdl_->get_number_of_restraints();i++) {
    mdl_rs.push_back(mdl_->get_restraint(i));
  }
  rc_->add_restraints(mdl_rs);
  Pointer<domino::HeapAssignmentContainer> all= get_assignments(
                 mt,boost::num_vertices(mt)-1,
                 ds,params_.get_domino_params().heap_size_,rc_,all_rs_filt_);
  all->set_was_used(true);

  // not in the correct order
  domino::Assignments sampled_assignments_temp=all->get_assignments();
  std::cout<<"number of found assignments:"<<sampled_assignments_temp.size()
           <<std::endl;
  sampled_assignments_.clear();
  for(int i=0;i<(int)sampled_assignments_temp.size();i++){
    Ints vals(sampled_assignments_temp[i].size());//the correct order
    for(unsigned int j=0;j<sampled_assignments_temp[i].size();j++) {
      //TODO - add order key to the particles
      vals[s[j]->get_value(order_key_)]=sampled_assignments_temp[i][j];
    }
    sampled_assignments_.push_back(domino::Assignment(vals));
  }

  std::cout<<"Number of found assignments :"<<sampled_assignments_.size()
           <<std::endl;

  //go back to original configuration
  for (int i=0;i<(int)rbs_.size();i++) {
    rbs_[i].set_reference_frame(orig_rf[i]);
  }

  IMP_LOG_TERSE("done alignment\n");
}

void ProteomicsEMAlignmentAtomic::add_states_and_filters(){
  //set proteins map
  std::map<int,Particle *> prot_ind_to_particle_map;
  for(int i=0;i<prot_data_->get_number_of_proteins();i++) {
    prot_ind_to_particle_map[
      prot_data_->find(prot_data_->get_protein_name(i))]=mhs_[i];
  }
  IMP_LOG_VERBOSE("going to set the states\n");
  //set the states
  pst_ = set_particle_states_table(filters_);
  rc_=new domino::RestraintCache(pst_);
  IMP_LOG_VERBOSE("number pf particles in table:"
          <<pst_->get_particles().size()<<std::endl);
  //  IMP_NEW(domino::BranchAndBoundSampler,ds,(mdl_,pst));
  std::cout<<"maximum number of states:"<<
    params_.get_domino_params().max_num_states_for_subset_<<std::endl;
  //  s->set_maximum_number_of_states(params_.get_domino_params()
  //                                           .max_num_states_for_subset_);
  //  s->set_log_level(IMP::VERBOSE);
  //set the restraints that will be used to generate the
  //subset graph
  //filters
  IMP_LOG_VERBOSE("settings filters\n");
  // two particles cannot
  //    be in the same state if they have the same ParticleStates,
  IMP_NEW(domino::ExclusionSubsetFilterTable,dist_filt,(pst_));
  filters_.push_back(dist_filt);
  // IMP_NEW(domino::RestraintScoreSubsetFilterTable,rs_filt,(mdl_,pst_));
  // filters_.push_back(rs_filt);
  states_set_=true;filters_set_=true;
}

void ProteomicsEMAlignmentAtomic::add_all_restraints(){
  IMP_USAGE_CHECK(params_.get_fragments_params().frag_len_==1,
       "In atomic mode the fragment length can not be higher than one!\n");
  //====== initialize the two restraint sets
  conn_rs_=new RestraintSet("connectivity");
  mdl_->add_restraint(conn_rs_);
  conn_rs_with_filter_=new RestraintSet("connectivity_filtered");
  mdl_->add_restraint(conn_rs_with_filter_);
  xlink_rs_=new RestraintSet("xlinks");
  mdl_->add_restraint(xlink_rs_);
  xlink_rs_with_filter_=new RestraintSet("xlinks_filtered");
  mdl_->add_restraint(xlink_rs_with_filter_);
  em_rs_=new RestraintSet("em");
  mdl_->add_restraint(em_rs_);
  ev_rs_=new RestraintSet("ev");
  mdl_->add_restraint(ev_rs_);
  dummy_rs_=new RestraintSet("dummy");
  mdl_->add_restraint(dummy_rs_);
  //====== set proteins map
  std::map<int,Particle *> prot_ind_to_particle_map;
  for(int i=0;i<prot_data_->get_number_of_proteins();i++) {
    prot_ind_to_particle_map[
      prot_data_->find(prot_data_->get_protein_name(i))]=mhs_[i];
  }
  //====== set the merge tree builder
  MergeTreeBuilder mtb(mhs_);
  //add connectivity restraints
  IMP_LOG_VERBOSE("setting connectivity restraints\n");
  std::cout<<"Number of interactions:"
           <<prot_data_->get_number_of_interactions()<<std::endl;
  for(int i=0;i<prot_data_->get_number_of_interactions();i++) {
    //get all of the relevant rigid bodies
    Ints prot_inds=prot_data_->get_interaction(i);
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG_VERBOSE("creating interaction bewteen:\n");
      for( int ii=0;ii<(int)prot_inds.size();ii++){
        IMP_LOG_VERBOSE(prot_inds[i]<<" ");
      }
      IMP_LOG_VERBOSE(std::endl);
    }
    std::cout<<"creating interaction bewteen:\n";
    for( int ii=0;ii<(int)prot_inds.size();ii++){
      std::cout<<prot_inds[ii]<<"|";
    }std::cout<<std::endl;
    atom::Selections sel;
    std::stringstream ss;
    ss<<"conn";
    for(unsigned int j=0;j<prot_inds.size();j++) {
      atom::Hierarchy mh
                = atom::Hierarchy(prot_ind_to_particle_map[prot_inds[j]]);
      sel.push_back(atom::Selection(mh));
      ss<<"."<<mh->get_name();
    }
    int k=1;//todo - make this a parameter!!
    Restraint *r = atom::create_connectivity_restraint(sel,k);
    if (r != NULL){
      r->set_name(ss.str());
      //only allow the particles to penetrate or separate by 1 angstrom
      std::cout<<"Max Score for the restraint:"<<ss.str()<<" "<<
        params_.get_connectivity_params().max_conn_rest_val_<<std::endl;
      mdl_->set_maximum_score(
                  r,
                  params_.get_connectivity_params().max_conn_rest_val_);
      if (prot_data_->get_interaction_part_of_filter(i)){
        std::cout<<"Adding restraint "<<r->get_name()<<" to conn with filter"
                 <<std::endl;
        conn_rs_with_filter_->add_restraint(r);
      }
      else {
        std::cout<<"Adding restraint "<<r->get_name()
                 <<" to conn without filter"<<std::endl;
        conn_rs_->add_restraint(r);
      }
      //add pairs to merge tree builder
      for(unsigned int k1=0;k1<prot_inds.size();k1++) {
        atom::Hierarchy mh1 =
          atom::Hierarchy(prot_ind_to_particle_map[prot_inds[k1]]);
        for(unsigned int k2=k1+1;k2<prot_inds.size();k2++) {
          atom::Hierarchy mh2 =
            atom::Hierarchy(prot_ind_to_particle_map[prot_inds[k2]]);
          mtb.increase_edge(mh1,mh2);
        }
      }
    }else {std::cout<<"restraint is NULL"<<std::endl;}
  }

  //add xlink restraints
  IMP_LOG_VERBOSE("setting xlink restraints\n");
  std::cout<<"Number of xlinks"<<prot_data_->get_number_of_cross_links()
           <<std::endl;
  for(int i=0;i<prot_data_->get_number_of_cross_links();i++) {
    Pointer<Restraint> rx;
    //get all of the relevant rigid bodies
    std::pair<IntPair,IntPair> xpair=prot_data_->get_cross_link(i);
    std::stringstream ss1;
    Particles pairx(2);
    ss1<<"xlink";
    std::cout<<"First key:"<<xpair.first.first<<" Second key:"
             <<xpair.second.first<<std::endl;
    atom::Hierarchy mh1 =
      atom::Hierarchy(prot_ind_to_particle_map[xpair.first.first]);
    ss1<<"."<<mh1->get_name()<<"."<<xpair.first.second;
    atom::Hierarchy mh2 =
      atom::Hierarchy(prot_ind_to_particle_map[xpair.second.first]);
    ss1<<"."<<mh2->get_name()<<"."<<xpair.second.second;
    std::cout<<"working on restraint:"<<ss1.str()<<std::endl;

    if (params_.get_xlink_params().treat_between_residues_) {
      double extra_len=0.;
      atom::AtomTypes atom_types;
      atom_types.push_back(atom::AtomType("CA"));
      std::cout<<"treat as xlink"<<std::endl;
      //create a restraint between CA atoms of the relevant residue
      atom::Hierarchies mh1_res=atom::get_by_type(mh1,atom::RESIDUE_TYPE);
      atom::Hierarchies mh2_res=atom::get_by_type(mh2,atom::RESIDUE_TYPE);
      int mh1_start_res_ind=atom::Residue(mh1_res[0]).get_index();
      int mh2_start_res_ind=atom::Residue(mh2_res[0]).get_index();
      int mh1_end_res_ind=atom::Residue(mh1_res[mh1_res.size()-1]).get_index();
      int mh2_end_res_ind=atom::Residue(mh2_res[mh2_res.size()-1]).get_index();
      //      pairx[0]=mh1_res[xpair.first.second-mh1_start_res_ind]
      //      pairx[1]=atom::get_residues(mh2)[xpair.second.second
      //                                       -mh2_start_res_ind].get_ca();
      //check that the xlink is within the residue range
      bool within_range=true;
      std::cout<<"====== "<< ss1.str()<< " " << xpair.first.second << " ["
               << mh1_start_res_ind<<","<<mh1_end_res_ind<<" ] second: "
               <<xpair.second.second<<" ["<<mh2_start_res_ind<<","
               <<mh2_end_res_ind<<"]"<<std::endl;
      if (xpair.first.second<mh1_start_res_ind) within_range=false;
      if (xpair.first.second>=mh1_end_res_ind) within_range=false;
      if (xpair.second.second<mh2_start_res_ind) within_range=false;
      if (xpair.second.second>=mh2_end_res_ind) within_range=false;
      if (!within_range) {
        std::cout<<"XLINK"<<ss1.str()
                 <<" is out of range given the models, adjusting length"
                 <<std::endl;
        IMP_WARN("XLINK"<<ss1.str()
                 <<" is out of range given the models, adjusting length"
                 <<std::endl);
        //change the linker length
        if (xpair.first.second<mh1_start_res_ind) {
          extra_len+=
            multifit::get_approximated_radius(
                             mh1_start_res_ind-xpair.first.second+1);
          xpair.first.second=mh1_start_res_ind;
        }
        else if (xpair.first.second>=mh1_end_res_ind) {
          extra_len+=multifit::get_approximated_radius(
                             xpair.first.second-mh1_end_res_ind+1);
          xpair.first.second=mh1_end_res_ind;
        }
        if (xpair.second.second<mh2_start_res_ind) {
          extra_len+=multifit::get_approximated_radius(
                             mh2_start_res_ind-xpair.second.second+1);
          xpair.second.second=mh2_start_res_ind;
        }
        else if (xpair.second.second>=mh2_end_res_ind) {
          extra_len+=multifit::get_approximated_radius(
                             xpair.second.second-mh2_end_res_ind+1);
          xpair.second.second=mh2_end_res_ind;
        }
      }
      atom::Hierarchy h1,h2;bool found=false;
      //find CA of the right residue ind.
      //can not simply do mh1_res[xpair.first.second-mh1_start_res_ind
      //because some residues may be missing.
      for (unsigned int kk=0;kk<mh1_res.size();kk++) {
        if (atom::Residue(mh1_res[kk]).get_index()==xpair.first.second) {
          found=true;
          h1=mh1_res[kk];
          break;
        }
      }
      IMP_USAGE_CHECK(found,"residue with index "<<xpair.first.second
                  <<" was not found in protein"<<mh1<<std::endl);
      found=true;
      for (unsigned int kk=0;kk<mh2_res.size();kk++) {
        if (atom::Residue(mh2_res[kk]).get_index()==xpair.second.second) {
          found=true;
          h2=mh2_res[kk];
          break;
        }
      }
      IMP_USAGE_CHECK(found,"residue with index "<<xpair.second.second
                <<" was not found in protein"<<mh2<<std::endl);
      atom::Selection s1(h1);
      s1.set_atom_types(atom_types);
      atom::Selection s2(h2);
      s2.set_atom_types(atom_types);
      pairx[0]=s1.get_selected_particles()[0];
      pairx[1]=s2.get_selected_particles()[0];
      std::cout<<"creaeting restraint between: "
        << pairx[0]->get_name()<<" and " << pairx[1]->get_name()<<std::endl;
      IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore,hub_updated,
              //(params_.get_xlink_params().upper_bound_+extra_len,
              (prot_data_->get_cross_link_length(i)+extra_len,
               params_.get_xlink_params().k_));

      rx = IMP::create_restraint(hub_updated.get(),
                                 ParticlePair(pairx[0],pairx[1]));
    }
    else { //treat as a connectivity restraint
      std::cout<<"treat as connectivity restraint"<<std::endl;
      atom::Selections sel;
      sel.push_back(atom::Selection(mh1));
      sel.push_back(atom::Selection(mh2));
      rx = atom::create_connectivity_restraint(
                        sel,
                        //params_.get_xlink_params().upper_bound_,
                        prot_data_->get_cross_link_length(i),
                        params_.get_xlink_params().k_);
    }
    if (rx != nullptr){
      if (prot_data_->get_cross_link_part_of_filter(i)){
        std::cout<<"Adding restraint "<<rx->get_name()<<" of length: "
                 << prot_data_->get_cross_link_length(i)
                 <<" to xlinks with filter"<<std::endl;
        xlink_rs_with_filter_->add_restraint(rx);
      }
      else {
        std::cout<<"Adding restraint "<<rx->get_name()<<" of length: "
                 << prot_data_->get_cross_link_length(i)
                 <<" to xlinks without filter"<<std::endl;
        xlink_rs_->add_restraint(rx);
      }
      rx->set_name(ss1.str());
      //only allow the particles to penetrate or separate by 1 angstrom
      std::cout<<"Max Score for the restraint:"<<ss1.str()<<" "<<
        params_.get_xlink_params().max_xlink_val_<<std::endl;
      mdl_->set_maximum_score(rx,
                              params_.get_xlink_params().max_xlink_val_);
      // if (prot_data_.get_xlink_used_to_build_jt(i)){
      //   jt_rs_.push_back(rx);
      // }
      //add pairs to merge tree builder
      mtb.increase_edge(mh1,mh2);
    }else {std::cout<<"restraint is NULL"<<std::endl;}
  }//end xlink restraints iteration

  //===========add EV filters
  if (params_.get_ev_params().scoring_mode_>0) {
    IMP_USAGE_CHECK(params_.get_fragments_params().subunit_rigid_,
                    "Logic error, EV operates on rigid bodies\n");
    IMP_LOG_VERBOSE("Add excluded volume restraint"<<std::endl);
    std::cout<<"Add excluded volume restraint"<<std::endl;
    //collect protein names and surface names
    Strings prot_names;
    for(int i=0;i<prot_data_->get_number_of_proteins();i++) {
      prot_names.push_back(asmb_data_->get_component_header(i)->get_filename());
    }
    std::cout<<"END initialize docking surfaces"<<std::endl;
    std::map<std::string,IntPair> pairs_map;
    for(int i=0;i<prot_data_->get_number_of_ev_pairs();i++) {
      IntPair ev_pair=prot_data_->get_ev_pair(i);
      int ind1=ev_pair.first;
      int ind2=ev_pair.second;
      pairs_map[get_pair_key(ind1,ind2)]=ev_pair;
    }
    for(unsigned int i=0;i<mhs_.size();i++){
      for(unsigned int j=i+1;j<mhs_.size();j++){
        if ((pairs_map.find(get_pair_key(i,j)) == pairs_map.end()) &&
            (params_.get_ev_params().scoring_mode_==2)) {
            IMP_WARN("EV: "<<get_pair_key(i,j)<<" will not be added"<<
                     " because EV scoring mode is 2"<<std::endl);
            std::cout<<"EV: "<<get_pair_key(i,j)<<" will not be added"<<
                     " because EV scoring mode is 2"<<std::endl;
            continue;
        }
        //get leaves of the mhs pair
        IMP_NEW(multifit::ComplementarityRestraint,evr,
                (atom::get_leaves(mhs_[i]),
                 atom::get_leaves(mhs_[j])));
        std::stringstream name;
        name<<"EV_"<<mhs_[i]->get_name()<<"_"<<mhs_[j]->get_name();
        evr->set_name(name.str());
        ComplementarityParams comp_param=params_.get_complementarity_params();
        evr->set_boundary_coefficient(comp_param.boundary_coef_);
        evr->set_complementarity_coefficient(comp_param.comp_coef_);
        evr->set_penetration_coefficient(comp_param.penetration_coef_);
        evr->set_interior_layer_thickness(comp_param.interior_layer_thickness_);
        float voxel_size=evr->get_voxel_size();
        float max_penetration=comp_param.max_penetration_;
        evr->set_maximum_penetration_score(
           max_penetration*voxel_size*voxel_size*voxel_size);
        mdl_->set_maximum_score(evr,comp_param.max_score_);
        ev_rs_->add_restraint(evr);
        mtb.increase_edge(mhs_[i],mhs_[j]);
      }
    }//end adding all wev restraints
  }
  else {
    std::cout<<"No EV restraints will be added"<<std::endl;
  }
  //add fitting restraint
  if (dmap_!=nullptr) {
    std::cout<<"Add fitting restraint"<<std::endl;
    std::cout<<"=====map is:"<<std::endl;
    dmap_->get_header()->show();
    std::cout<<"================="<<std::endl;
    ParticlesTemp all_leaves,all_ca;
    atom::AtomTypes atom_types;
    atom_types.push_back(atom::AtomType("CA"));
    for(unsigned int i=0;i<mhs_.size();i++) {
      ParticlesTemp mol_leaves=atom::get_leaves(mhs_[i]);
      all_leaves.insert(all_leaves.end(),mol_leaves.begin(),mol_leaves.end());
      atom::Hierarchies mh_res=atom::get_by_type(mhs_[i],atom::RESIDUE_TYPE);
      atom::Selection s1(mh_res);
      s1.set_atom_types(atom_types);
      ParticlesTemp pt = s1.get_selected_particles();
      all_ca.insert(all_ca.end(),pt.begin(),pt.end());
    }
    std::cout<<"after adding leaves"<<std::endl;
    //calculcate normalization factors
    if (fast_scoring_) {
      std::cout<<"going to use fast scoring"<<std::endl;
      //create a decomposition version of the fit restraint
      IMP_NEW(em::SampledDensityMap,full_sampled_map,(*(dmap_->get_header())));
      full_sampled_map->set_particles(all_leaves);
      //move all proteins to the center of the map
      algebra::Transformation3Ds cen_ts;
      for(unsigned int i=0;i<mhs_.size();i++){
        algebra::Transformation3D cen_t= algebra::Transformation3D(
                     algebra::get_identity_rotation_3d(),
                     dmap_->get_centroid()-
                     core::get_centroid(core::XYZs(core::get_leaves(mhs_[i]))));
        core::transform(rbs_[i],cen_t);
        cen_ts.push_back(cen_t);
      }
      full_sampled_map->resample();
      for(unsigned int i=0;i<mhs_.size();i++) {
        core::transform(rbs_[i],cen_ts[i].get_inverse());
      }
      full_sampled_map->calcRMS();
      dmap_->calcRMS();
      double upper=(dmap_->get_number_of_voxels()
                    *dmap_->get_header()->dmean
                    *full_sampled_map->get_header()->dmean)/mhs_.size();
      double lower=dmap_->get_number_of_voxels()*dmap_->calcRMS()
                   *full_sampled_map->calcRMS();
      FloatPair norm_factors=std::make_pair(upper,lower);
      std::cout<<"norm factors:"<<upper<<" "<<lower<<std::endl;
      //    double scale=100.*mhs_.size()*(mhs_.size()-1);
      double scale=1.;
      for(unsigned int i=0;i<mhs_.size();i++){
        IMP_NEW(em::FitRestraint,fitr,(core::get_leaves(mhs_[i]),dmap_,
                                       norm_factors,
                                       atom::Mass::get_mass_key(),1,
                                       false));
        fitr->set_scale_factor(scale);//TODO - make parameter
        std::cout<<"EM fit scale "<<scale<<std::endl;
        fitr->set_name("fitting");
        em_rs_->add_restraint(fitr);
      }
      double max_em_val=(params_.get_fitting_params().max_asmb_fit_score_
                         +(mhs_.size()-1))*scale;
      em_rs_->set_maximum_score(max_em_val);
      std::cout<<"SET MAX score on fitting restraint set:"<<max_em_val
               <<std::endl;

  //add total score filter
  //  mdl_->set_maximum_score(params_.get_domino_params().max_value_threshold_);
    }
    else {
      /*
      std::cout<<"creating fit restraint with "<< all_ca.size()
               <<" CA atoms "<<std::endl;
      IMP_NEW(em::FitRestraint,fitr,(all_ca,dmap_,
          FloatPair(0.,0.),
          atom::Mass::get_mass_key(),1,
          false,em::SPHERE));
      std::cout<<"end add restraint"<<std::endl;
      em_rs_->add_restraint(fitr);
      //      float scale=100*mhs_.size()*(mhs_.size()-1);
      float scale=1.;
      std::cout<<"using regular em restraint with maximum score of "
               <<params_.get_domino_params().em_max_value_threshold_
               <<"*"<<scale<<std::endl;
      fitr->set_maximum_score(
                 params_.get_domino_params().em_max_value_threshold_*scale);
      fitr->set_scale_factor(scale);//TODO - make parameter
      fitr->set_name("fitting");
      */
      std::cout<<"creating pcafit restraint with "<< all_ca.size()
               <<" CA atoms "<<std::endl;
      //float max_angle_diff=15;//make parameter todo
      IMP_NEW(em::PCAFitRestraint,fitr,(all_ca,dmap_,
        asmb_data_->get_assembly_header()->get_threshold(),
        params_.get_fitting_params().pca_max_size_diff_,
        params_.get_fitting_params().pca_max_angle_diff_,
        params_.get_fitting_params().pca_max_cent_dist_diff_));
      std::cout<<"end add restraint"<<std::endl;
      em_rs_->add_restraint(fitr);
      //      float scale=100*mhs_.size()*(mhs_.size()-1);
      fitr->set_maximum_score(0.01);
      fitr->set_name("fitting");
    }
  }
  restraints_set_=true;
  //add all filters
  std::cout<<"before adding filters"<<std::endl;
  RestraintsTemp conn_rst;
  for(unsigned ii=0;ii<conn_rs_with_filter_->get_number_of_restraints();ii++) {
    conn_rst.push_back(conn_rs_with_filter_->get_restraint(ii));
  }
  std::cout<<"========1"<<std::endl;
  IMP_NEW(domino::MinimumRestraintScoreSubsetFilterTable,min_conn_filter,
          (conn_rst,rc_,params_.get_filters_params().max_num_violated_conn_));
  min_conn_filter->set_name("minimum_conn_filter");
  //  min_conn_filter->set_use_caching(true);
  //  min_conn_filter->set_maximum_number_of_cache_entries(
  //                         params_.get_domino_params().cache_size_);
  filters_.push_back(min_conn_filter);
  std::cout<<"Create filter on connectivity restraints, number of "
           <<"allowed violations is:"
           <<min_conn_filter->get_maximum_number_of_violated_restraints()
           <<std::endl;
  IMP_NEW(domino::RestraintScoreSubsetFilterTable,conn_filter,
          (conn_rs_,pst_));
  conn_filter->set_name("conn_filter");
  //  conn_filter->set_use_caching(true);
  //  conn_filter->set_maximum_number_of_cache_entries(
  //                          params_.get_domino_params().cache_size_);
  filters_.push_back(conn_filter);
  std::cout<<"========2"<<std::endl;
  RestraintsTemp xlink_rst;
  for(unsigned ii=0;ii<xlink_rs_with_filter_->get_number_of_restraints();ii++) {
    xlink_rst.push_back(xlink_rs_with_filter_->get_restraint(ii));
  }
  IMP_NEW(domino::MinimumRestraintScoreSubsetFilterTable,min_xlink_filter,
          (xlink_rst,rc_,params_.get_filters_params().max_num_violated_xlink_));
  min_xlink_filter->set_name("minimum_xlink_filter");
  //  min_xlink_filter->set_use_caching(true);
  //  min_xlink_filter->set_maximum_number_of_cache_entries(
  //                        params_.get_domino_params().cache_size_);
  filters_.push_back(min_xlink_filter);
  std::cout<<"Create filter for xlink restraints, number of allowed "
           <<"violations is:"
           <<min_xlink_filter->get_maximum_number_of_violated_restraints()
           <<std::endl;
  IMP_NEW(domino::RestraintScoreSubsetFilterTable,xlink_filter,
          (xlink_rs_,pst_));
  xlink_filter->set_name("xlink_filter");
  std::cout<<"========3"<<std::endl;
  //  xlink_filter->set_use_caching(true);
  //  xlink_filter->set_maximum_number_of_cache_entries(
  //                         params_.get_domino_params().cache_size_);
  filters_.push_back(xlink_filter);

  RestraintsTemp ev_rst;
  for(unsigned ii=0;ii<ev_rs_->get_number_of_restraints();ii++) {
    ev_rst.push_back(ev_rs_->get_restraint(ii));
  }
  IMP_NEW(domino::MinimumRestraintScoreSubsetFilterTable,ev_filter,
          (ev_rst,rc_,params_.get_filters_params().max_num_violated_ev_));
  ev_filter->set_name("ev_filter");
  //  ev_filter->set_use_caching(true);
  //  ev_filter->set_maximum_number_of_cache_entries(
  //                               params_.get_domino_params().cache_size_);
  filters_.push_back(ev_filter);
  std::cout<<"Create filter on ev restraints, number of allowed violations is:"
           <<ev_filter->get_maximum_number_of_violated_restraints()<<std::endl;

  IMP_NEW(domino::RestraintScoreSubsetFilterTable,em_filter,
          (em_rs_,pst_));
  //  em_filter->set_use_caching(true);
  //  em_filter->set_maximum_number_of_cache_entries(
  //                           params_.get_domino_params().cache_size_);
  filters_.push_back(em_filter);
  //=====create jt_rs:
  std::cout<<"========4"<<std::endl;
  mtb.show();
  ParticlePairsTemp pps=mtb.get_mst_dependency();
  for(ParticlePairsTemp::iterator it = pps.begin(); it != pps.end(); it++){
    std::stringstream name;
    name<<"dummy."<<(*it)[0]->get_name()<<"."<<(*it)[1]->get_name();
    std::cout<<"Adding dummy restraint: "<< name.str()<<std::endl;
    IMP_NEW(DummyRestraint,dr,((*it)[0],(*it)[1]));
    dr->set_name(name.str());
    jt_rs_.push_back(dr);
    dummy_rs_->add_restraint(dr);
  }
  IMP_NEW(domino::RestraintScoreSubsetFilterTable,dummy_filter,
          (dummy_rs_,pst_));
  std::cout<<"========5"<<std::endl;
  filters_.push_back(dummy_filter);
}

// void ProteomicsEMAlignmentAtomic::sort_configurations() {
//   if (cg_==NULL) return;
//   //sort by score
//   IMP_LOG_VERBOSE("sorting:"<<cg_->get_number_of_configurations()
//                   <<" configurations\n");
//   for(int i=0;i<(int)cg_->get_number_of_configurations();i++) {
//     cg_->load_configuration(i);
//     cg_sorted_.push_back(std::pair<int,float>(i,
//                                               conn_rs_->evaluate(NULL)+
//                                               rog_rs_->evaluate(NULL)));
//   }
//   std::sort(cg_sorted_.begin(),cg_sorted_.end(),
//             configuration_sorter);
// }

domino::Assignments
ProteomicsEMAlignmentAtomic::get_combinations(bool /*uniques*/) const {
  return sampled_assignments_;
  /*
  //set the right order of the assignment.
  IntsList ret(sampled_assignments_.size());
  std::map<Particle *,FittingStates*> mps;
  for (core::RigidBodies::const_iterator it = rbs_.begin();
       it != rbs_.end();it++) {
    mps[*it]=FittingStates::get_from(pst_->get_particle_states(*it));
  }
  std::cout<<"Get sampled assignments for particles:"<<std::endl;
  ParticlesTemp ps=pst_->get_particles();
  std::map<Particle*,int> ps_order_map;
  std::cout<<"Correct order:"<<std::endl;
  for(int i=0;i<(int)rbs_.size();i++) {
    ps_order_map[rbs_[i].get_particle()]=i;
    std::cout<<rbs_[i].get_particle()->get_name()<<"|";
  }std::cout<<std::endl;
  Ints ps_order(ps.size());
  std::cout<<"Original order:"<<std::endl;
  for (int i=0;i<(int)ps.size();i++) {
    ps_order[ps_order_map[ps[i]]]=i;
    std::cout<<ps[i]->get_name()<<"|";
  }std::cout<<"Order:"<<std::endl;
  for(int i=0;i<(int)ps_order.size();i++){
    std::cout<<"["<<i<<","<<ps_order[i]<<"] ";
  }
  std::cout<<std::endl;
  for(int i=0;i<(int)sampled_assignments_.size();i++) {
    Ints vals(ps_order.size());
    for (int j=0;j<(int)ps_order.size();j++) {
      vals[j]=sampled_assignments_[i][ps_order[j]];
    }
    ret[i]=vals;
  }
  return ret;*/
}

void ProteomicsEMAlignmentAtomic::load_combination_of_states(
  const Ints &state4particles) {
    IMP_USAGE_CHECK(state4particles.size()==mhs_.size(),
                    "comb size is not euqal to the number of molecules \n");
    IMP_USAGE_CHECK(states_set_,
                    "states were not initialized \n");
    for (int i=0;i<(int)mhs_.size();i++){
      //      std::cout<<"Molecule "<<i<<" is: "<<mhs_[i]->get_name()
      //               <<std::endl;
      IMP_USAGE_CHECK(pst_->get_has_particle(mhs_[i].get_particle()),
                      "Particle "<<mhs_[i]->get_name()
                      <<" does not have states\n");
      pst_->get_particle_states(mhs_[i])->load_particle_state(
                            state4particles[i],mhs_[i].get_particle());
    }
  }

IMPMULTIFIT_END_NAMESPACE
