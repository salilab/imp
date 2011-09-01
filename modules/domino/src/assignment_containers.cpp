/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/assignment_containers.h>
#include <IMP/domino/Subset.h>


IMPDOMINO_BEGIN_NAMESPACE

AssignmentContainer::AssignmentContainer(std::string name): Object(name){}

AssignmentContainer::~AssignmentContainer(){}

ListAssignmentContainer::ListAssignmentContainer(std::string name):
  AssignmentContainer(name){}


void ListAssignmentContainer::do_show(std::ostream &out) const {
  out << "size: " << get_number_of_assignments() << std::endl;
}



PackedAssignmentContainer::PackedAssignmentContainer(std::string name):
  AssignmentContainer(name), width_(-1){}


void PackedAssignmentContainer::do_show(std::ostream &out) const {
  out << "size: " << get_number_of_assignments() << std::endl;
  out << "width: " << width_ << std::endl;
}



SampleAssignmentContainer::SampleAssignmentContainer(unsigned int k,
                                                     std::string name):
  AssignmentContainer(name), width_(-1), k_(k), i_(0), select_(0,1),
  place_(0, k_-1) {}


void SampleAssignmentContainer::do_show(std::ostream &out) const {
  out << "size: " << get_number_of_assignments() << std::endl;
  out << "width: " << width_ << std::endl;
}

void SampleAssignmentContainer::add_assignment(const Assignment& a) {
  IMP_USAGE_CHECK(width_==-1 || static_cast<int>(a.size())== width_,
                  "Sizes don't match " << width_
                  << " vs " << a.size());
  if (width_==-1) {
    width_=a.size();
  }
  ++i_;
  if (get_number_of_assignments() < k_) {
    d_.insert(d_.end(), a.begin(), a.end());
  } else {
    double prob= static_cast<double>(k_)/i_;
    if (select_(random_number_generator) < prob) {
      int replace= place_(random_number_generator);
      std::copy(a.begin(), a.end(), d_.begin()+width_*replace);
    }
  }
}

#ifdef IMP_DOMINO_USE_IMP_RMF

Ints get_order(const Subset &s,
               const ParticlesTemp &all_particles);

HDF5AssignmentContainer::HDF5AssignmentContainer(RMF::HDF5Group parent,
                                                 const Subset &s,
                                           const ParticlesTemp &all_particles,
                                                 std::string name):
  AssignmentContainer(name), ds_(parent.add_child_index_data_set_2d(name)),
  init_(false), order_(get_order(s, all_particles)) {}


HDF5AssignmentContainer
::HDF5AssignmentContainer(RMF::HDF5IndexDataSet2D dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name):
  AssignmentContainer(name), ds_(dataset),
  init_(true), order_(get_order(s, all_particles)) {}


unsigned int HDF5AssignmentContainer::get_number_of_assignments() const {
  return ds_.get_size()[0];
}

Assignment HDF5AssignmentContainer::get_assignment(unsigned int i) const {
  RMF::Ints is= ds_.get_row(Ints(1,i));
  Ints ret(is.size());
  IMP_USAGE_CHECK(ret.size()== order_.size(), "Wrong size assignment");
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[order_[i]]=is[i];
  }
  return Assignment(ret);
}

void HDF5AssignmentContainer::add_assignment(const Assignment& a) {
  if (!init_) {
    Ints sz(2);
    sz[0]=0; sz[1]=a.size();
    ds_.set_size(sz);
    init_=true;
  }
  Ints is(a.size());
  IMP_USAGE_CHECK(a.size()== order_.size(), "Wrong size assignment");
  for (unsigned int i=0; i< a.size(); ++i) {
    is[i]= a[order_[i]];
  }
  RMF::HDF5DataSetIndexD<2> sz= ds_.get_size();
  ++sz[0];
  ds_.set_size(sz);
  IMP_USAGE_CHECK(ds_.get_size()[1]==a.size(),
                  "Sizes don't match: " << a.size() << " vs "
                  << ds_.get_size()[1]);
  ds_.set_row(Ints(1,sz[0]-1), is);
}



void HDF5AssignmentContainer::do_show(std::ostream &) const {
  //out << "dataset: " << ds_.get_name() << std::endl;
}

#endif



////////////////////////// HEAP ASSIGNMENT CONTAINER

inline unsigned int
HeapAssignmentContainer::get_number_of_assignments() const {
  return d_.size();
}

inline Assignment
HeapAssignmentContainer::get_assignment(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Invalid assignment requested: " << i);
  return d_[i].first;
}
HeapAssignmentContainer::HeapAssignmentContainer(
                               unsigned int k,
                               RestraintScoreSubsetFilter *rssf,
                               std::string name):
  AssignmentContainer(name), k_(k), rssf_(rssf) {}

void HeapAssignmentContainer::do_show(std::ostream &out) const {
  out << "number of assignments: " << get_number_of_assignments();
  out << ", max heap size: " << k_ << std::endl;
}



void HeapAssignmentContainer::add_assignment(const Assignment& a) {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< get_number_of_assignments(); ++i) {
      IMP_INTERNAL_CHECK(get_assignment(i) != a,
                         "Assignment " << a << " already here.");
    }
  }
  //rssf_ may be null if no restraints are assigned to the particles
  double score=INT_MAX;
  if (rssf_!=NULL){
    score=rssf_->get_score(a);
  }
  d_.push_back(AP(a,score));
  std::push_heap(d_.begin(), d_.end(), GreaterSecond());
  while (d_.size() > k_){
    std::pop_heap(d_.begin(), d_.end(),
                  GreaterSecond());
    d_.pop_back();
  }
  // if (d_.size()%1000000 == 0) {
  //   std::cout<<"Current subset size:"<<d_.size()<<" : "<<a<<std::endl;
  // }
}

//////////// CLUSTERED
ClusteredAssignmentContainer
::ClusteredAssignmentContainer(unsigned int k,
                               Subset s,
                               ParticleStatesTable *pst):
  k_(k), s_(s), pst_(pst), r_(0) {}


inline unsigned int
ClusteredAssignmentContainer::get_number_of_assignments() const {
  return d_.size();
}

inline Assignment
ClusteredAssignmentContainer::get_assignment(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Invalid assignment requested: " << i);
  return d_[i].second;
}

void ClusteredAssignmentContainer::do_show(std::ostream &out) const {
  out << "number of assignments: " << get_number_of_assignments();
  out << ", max size: " << k_ << std::endl;
}

algebra::VectorKD
ClusteredAssignmentContainer::get_embedding(const Assignment &a) const {
  Floats embed;
  for (unsigned int i=0; i< s_.size(); ++i) {
    algebra::VectorKD cur
      = pst_->get_particle_states(s_[i])->get_embedding(a[i], s_[i]);
    embed.insert(embed.end(), cur.coordinates_begin(),
                 cur.coordinates_end());
  }
  return embed;
}

bool ClusteredAssignmentContainer
::get_in_cluster(const algebra::VectorKD &v) const {
  for (unsigned int i=0; i< d_.size(); ++i) {
    double dist=get_distance_if_smaller_than(d_[i].first, v, r_);
    if (dist < r_) {
      IMP_LOG(VERBOSE, v << " is in cluster with center " << d_[i].first
              << " with radius " << r_ << std::endl);
      return true;
    }
  }
  return false;
}

double ClusteredAssignmentContainer::get_minimum_distance() const {
  double md= std::numeric_limits<double>::max();
  for (unsigned int i=0; i< d_.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      double cd= get_distance_if_smaller_than(d_[i].first,
                                              d_[j].first,
                                              md);
      if (cd <md) md=cd;
    }
  }
  return 2*md+.1;
}

void ClusteredAssignmentContainer::recluster() {
  using IMP::operator<<;
  IMP_LOG(VERBOSE, "Reclustering from " << d_ << std::endl);
  compatibility::checked_vector<AP> nd_;
  std::swap(nd_, d_);
  for (unsigned int i=0; i< nd_.size(); ++i) {
    if (!get_in_cluster(nd_[i].first)) {
      IMP_LOG(VERBOSE, "Adding state " << nd_[i] << std::endl);
      d_.push_back(nd_[i]);
    }
  }
  IMP_LOG(VERBOSE, "Reclustered to " << d_ << std::endl);
}

double ClusteredAssignmentContainer::
get_distance_if_smaller_than(const algebra::VectorKD &a,
                             const algebra::VectorKD &b,
                             double max) const {
  IMP_USAGE_CHECK(a.get_dimension()==b.get_dimension(),
                  "Dimensions of embeddings don't match.");
  double d=0;
  for (unsigned int i=0; i< a.get_dimension(); ++i) {
    d+= square(a[i]-b[i]);
    if (d > square(max)) {
      IMP_LOG(VERBOSE, "Returning " << std::sqrt(d) << " > " << max
              << std::endl);
      return std::sqrt(d);
    }
  }
  IMP_LOG(VERBOSE, "Distance between " << a << " and "
          << b << " is " << std::sqrt(d) << std::endl);
  return std::sqrt(d);
}

void ClusteredAssignmentContainer::add_assignment(const Assignment& a) {
  IMP_OBJECT_LOG;
  AP ap(get_embedding(a), a);
  if (r_==0) {
    IMP_LOG(VERBOSE, "Adding state to list" << std::endl);
    d_.push_back(ap);
  } else {
    IMP_INTERNAL_CHECK(r_ > 0,
                       "R is not initialized");
    if (get_in_cluster(ap.first)) {
      IMP_LOG(VERBOSE, "State covered by existing cluster with radius " << r_
              << std::endl);
      return;
    } else {
      IMP_LOG(VERBOSE, "State added to new cluster "
              << std::endl);
      // perhaps update search structure
      d_.push_back(ap);
    }
  }
  if (d_.size() >k_) {
    if (r_==0) {
      r_= get_minimum_distance();
      IMP_LOG(VERBOSE, "Initial distance is " << r_ << std::endl);
    } else {
      r_*=2;
    }
    recluster();
  }
}



IMPDOMINO_END_NAMESPACE
