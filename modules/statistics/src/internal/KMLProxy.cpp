/**
 *  \file KMLProxy.cpp  \brief proxy to k-mean algorithm
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/statistics/internal/KMLProxy.h>
#include <IMP/statistics/internal/KMRectangle.h>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

KMLProxy::KMLProxy() {
  set_default_values();
  is_init_ = false;
}
void KMLProxy::initialize(Model *m,const Particles &ps,
    const FloatKeys &atts,unsigned int num_centers){
  for(Particles::const_iterator it = ps.begin(); it != ps.end();it++)
  {ps_.push_back(*it);}
  for(FloatKeys::const_iterator it = atts.begin();
      it != atts.end();it++)
  {atts_.push_back(*it);}
  m_ = m;
  kcenters_=num_centers;
  dim_=atts.size();
  centroids_ = Particles();
  data_ = new KMData(dim_,ps_.size());
  for(unsigned int i=0;i<ps_.size(); i++) {
    for(unsigned int j=0;j<atts.size();j++){
      (*(*data_)[i])[j]=ps_[i]->get_value(atts[j]);
    }
  }
  is_init_=true;
}

void KMLProxy::run(Particles *initial_centers) {
  IMP_INTERNAL_CHECK(is_init_,"The proxy was not initialized");
  IMP_LOG_VERBOSE("KMLProxy::run start \n");
  //use the initial centers if provided
  KMPointArray *kmc=nullptr;
  if (initial_centers != nullptr)  {
    IMP_INTERNAL_CHECK(kcenters_ == initial_centers->size(),
    "the number of initial points differs from the number of required"
    <<" centers\n");
    IMP_LOG_VERBOSE("KMLProxy::run initial centers provided : \n");
    kmc = allocate_points(kcenters_,atts_.size());
    for (unsigned int i=0;i<kcenters_;i++){
      Particle *cen=(*initial_centers)[i];
      for(unsigned int j=0;j<atts_.size();j++) {
        (*(*kmc)[i])[j]=cen->get_value(atts_[j]);
       }
    }
  }
  IMP_LOG_VERBOSE("KMLProxy::run load initial guess \n");
  //load the initail guess
  KMFilterCenters ctrs(kcenters_, data_, kmc,damp_factor_);

  //apply lloyd search
  IMP_LOG_VERBOSE("KMLProxy::run load lloyd \n");
  lloyd_alg_ = new KMLocalSearchLloyd(&ctrs,&term_);
  log_header();
  IMP_CHECK_CODE(clock_t start = clock());
  IMP_LOG_VERBOSE("KMLProxy::run excute lloyd \n");
  lloyd_alg_->execute();
  IMP_LOG_VERBOSE("KMLProxy::run analyse \n");
  KMFilterCentersResults best_clusters = lloyd_alg_->get_best();
  IMP_CHECK_CODE(Float exec_time = elapsed_time(start));
  IMP_CHECK_CODE(IMP_LOG_WRITE(TERSE,log_summary(&best_clusters,exec_time)));
  IMP_LOG_WRITE(TERSE,best_clusters.show(IMP_STREAM));
  IMP_INTERNAL_CHECK(kcenters_
                     == (unsigned int) best_clusters.get_number_of_centers(),
             "The final number of centers does not match the requested one");
  IMP_INTERNAL_CHECK (dim_ == (unsigned int) best_clusters.get_dim(),
              "The dimension of the final clusters is wrong");
  //TODO clear the centroids list
  //set the centroids:
  Particle *p;
  IMP_LOG_VERBOSE("KMLProxy::run load best results \n");
  for (unsigned int ctr_ind = 0; ctr_ind < kcenters_; ctr_ind++) {
    KMPoint *kmp = best_clusters[ctr_ind];
    //create a new particle
    p = new Particle(m_);
    centroids_.push_back(p);
    for (unsigned int att_ind = 0; att_ind < dim_; att_ind++) {
      p->add_attribute(atts_[att_ind],(*kmp)[att_ind],false);
    }
  }
  //set the assignment of particles to centers
  //array of number of all points
  //TODO - return this
  IMP_LOG_VERBOSE("KMLProxy::run get assignments \n");
  const Ints *close_center = best_clusters.get_assignments();
  IMP_LOG_VERBOSE("KMLProxy::run get assignments 2\n");
  for (int i=0;i<data_->get_number_of_points();i++) {
    //std::cout<<"ps number i: " << i << " close center : "
    //<< (*close_center)[i] << std::endl;
    assignment_[ps_[i]]=(*close_center)[i];
  }
}

void KMLProxy::set_default_values() {
  dim_            = 3;
  data_size_      = 100;
  kcenters_       = 5;
  max_swaps_      = 1;
  damp_factor_    = 1.0;
  n_color_        = 5;
  new_clust_      = false;
  max_dim_        = 1;
  distr_          = UNIFORM;
  std_dev_        = 1.00;
  corr_coef_      = 0.05;
  clus_sep_       = 0.0;
  max_visit_      = 0;
  seed_           = 0;
  kc_build_time_  = 0.0;
  print_points_   = false;
  show_assign_    = false;
  validate_       = false;
  term_ = KMTerminationCondition(300,30,0.1,0.1);
}

void KMLProxy::log_header()  const{
  IMP_LOG_TERSE( "\n[Run_k-means:\n"
      << "  data_size       = " << data_->get_number_of_points() << "\n"
      << "  kcenters        = " << kcenters_ << "\n"
      << "  dim             = " << dim_ << "\n"
    //TODO - should we add this back ?
    //      << "  max_tot_stage   = " << term_.getMaxTotStage(kcenters_,
    //data_size_) << "\n"
      << "  max_run_stage   = " << term_.get_max_num_of_stages_for_run() << "\n"
      << "  min_accum_rdl   = " << term_.get_min_accumulated_rdl() << "\n");
}

void KMLProxy::log_summary(KMFilterCentersResults *ctrs,Float run_time,
                          std::ostream &out) const {
  int n_stages = lloyd_alg_->get_total_number_of_stages();
  Float total_time = run_time + kc_build_time_;
  out<<"\n[k-means completed:\n"
      << "  n_stages      = "  << n_stages << "\n"
      << "  total_time    = "  << total_time << " sec\n"
      << "  init_time     = "  << kc_build_time_ << " sec\n"
      << "  stage_time_    = " << Float(run_time)/Float(n_stages)
      << " sec/stage_(excl_init) " << Float(total_time)/Float(n_stages)
      << " sec/stage_(incl_init)\n"
      << "  average_distort = "
      << ctrs->get_distortion()/Float(ctrs->get_number_of_centers())
      << "\n";
}

std::string KMLProxy::get_cmm_string() const {
//   CMMLogOptimizerState cmm_log;
//   cmm_log.set_radius(IMP.StringKey("get_cmm_string(centroids_);
  //TODO - add the edges
  return std::string();
}
unsigned int KMLProxy::get_particle_assignment(Particle *p) const {
  return assignment_.find(p)->second;
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE
