/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/assignment_containers.h>
#include <IMP/domino/Subset.h>
#include <IMP/domino/utility.h>
#include <IMP/base/warning_macros.h>
#include <fcntl.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <sys/stat.h>

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

#if IMP_DOMINO_HAS_RMF
WriteHDF5AssignmentContainer
::WriteHDF5AssignmentContainer(RMF::HDF5::Group parent,
                               const Subset &s,
                               const ParticlesTemp &all_particles,
                               std::string name):
  AssignmentContainer(name), ds_(parent.add_child_index_data_set_2d(name)),
  order_(s, all_particles),
  max_cache_(10000) {
  RMF::HDF5::IndexDataSet2D::Index sz;
  sz[0]=0; sz[1]=s.size();
  ds_.set_size(sz);
}


WriteHDF5AssignmentContainer
::WriteHDF5AssignmentContainer(RMF::HDF5::IndexDataSet2D dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name):
  AssignmentContainer(name), ds_(dataset),
  order_(s, all_particles),
  max_cache_(10000) {
  if (ds_.get_size()[1] != s.size()) {
    RMF::HDF5::IndexDataSet2D::Index sz;
    sz[0]=0; sz[1]=s.size();
    ds_.set_size(sz);
  }
}


unsigned int WriteHDF5AssignmentContainer::get_number_of_assignments() const {
  return ds_.get_size()[0]+cache_.size()/order_.size();
}

Assignment WriteHDF5AssignmentContainer::get_assignment(unsigned int) const {
  IMP_NOT_IMPLEMENTED;
  return Assignment();
}

void WriteHDF5AssignmentContainer::flush() {
  if (cache_.empty()) return;
  RMF::HDF5::IndexDataSet2D::Index size= ds_.get_size();
  RMF::HDF5::IndexDataSet2D::Index nsize=size;
  int num_items=cache_.size()/order_.size();
  IMP_LOG_VERBOSE( "Flushing cache of size "
          << num_items << " to disk"
          << std::endl);
  nsize[0]+= num_items;
  ds_.set_size(nsize);
  RMF::HDF5::IndexDataSet2D::Index write_size;
  write_size[0]=num_items;
  write_size[1]=order_.size();
  size[1]=0;
  ds_.set_block(size, write_size, cache_);
  cache_.clear();
  cache_.reserve(max_cache_);
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    unsigned int num=cache_.size()/order_.size();
    Assignments n(num);
    for (unsigned int i=0; i< num;++i) {
      n[i]=Assignment(cache_.begin()+i*order_.size(),
                      cache_.begin()+(i+1)*order_.size());
    }
    IMP_INTERNAL_CHECK(ds_.get_size()[0] >= num,
                       "Not enough on disk: " << ds_.get_size()[0]
                       << " vs " << num);
    for (unsigned int i=0; i< num; ++i) {
      Assignment read=get_assignment(get_number_of_assignments()-num+i);
      IMP_INTERNAL_CHECK(read==n[i], "Mismatch on read: " << read
                         << " vs " << n[i]);
    }
  }
  ds_.get_file().flush();
}

void WriteHDF5AssignmentContainer::set_cache_size(unsigned int words) {
  max_cache_=words;
  if (cache_.size()>max_cache_) flush();
}

void WriteHDF5AssignmentContainer::add_assignment(const Assignment& a) {
  IMP_USAGE_CHECK(a.size()==order_.size(),
                  "Sizes don't match: " << a.size()
                  << " vs " << order_.size());
  Ints save= order_.get_list_ordered(a);
  cache_.insert(cache_.end(), save.begin(), save.end());
  if (cache_.size() > max_cache_) flush();
}




ReadHDF5AssignmentContainer
::ReadHDF5AssignmentContainer(RMF::HDF5::IndexConstDataSet2D dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name):
  AssignmentContainer(name), ds_(dataset),
  order_(s, all_particles),
  max_cache_(10000){
}


unsigned int ReadHDF5AssignmentContainer::get_number_of_assignments() const {
  return ds_.get_size()[0]+cache_.size()/order_.size();
}

Assignment ReadHDF5AssignmentContainer::get_assignment(unsigned int i) const {
  RMF::HDF5::Ints is= ds_.get_row(RMF::HDF5::DataSetIndexD<1>(i));
  IMP_USAGE_CHECK(is.size()== order_.size(), "Wrong size assignment");
  return order_.get_subset_ordered(is.begin(), is.end());
}


void ReadHDF5AssignmentContainer::set_cache_size(unsigned int words) {
  max_cache_=words;
}

void ReadHDF5AssignmentContainer::add_assignment(const Assignment& ) {
  IMP_NOT_IMPLEMENTED;
}

#endif



WriteAssignmentContainer
::WriteAssignmentContainer(std::string dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name):
  AssignmentContainer(name),
  order_(s, all_particles),
  max_cache_(10000) {
  cache_.reserve(max_cache_);
  f_=open(dataset.c_str(), O_WRONLY|O_APPEND|O_CREAT|O_TRUNC
#ifdef _MSC_VER
          |O_BINARY, _S_IREAD|_S_IWRITE);
#else
          , S_IRUSR|S_IWUSR);
#endif
  number_=0;
}


unsigned int WriteAssignmentContainer::get_number_of_assignments() const {
  return number_;
}

Assignment WriteAssignmentContainer::get_assignment(unsigned int) const {
  IMP_NOT_IMPLEMENTED;
}

void WriteAssignmentContainer::flush() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Flushing " << cache_.size() << " entries" << std::endl);
  set_was_used(true);
  if (cache_.empty()) return;
  int ret=write(f_, &cache_[0], cache_.size()*sizeof(int));

  IMP_INTERNAL_CHECK(ret == static_cast<int>(cache_.size()*sizeof(int)),
                  "Not everything written: " << ret
                  << " of " << cache_.size()*sizeof(int));

  IMP_CHECK_VARIABLE(ret);
  cache_.clear();
  cache_.reserve(max_cache_);
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  size_t size = lseek(f_, 0, SEEK_CUR);
  IMP_INTERNAL_CHECK(size== number_*order_.size()*sizeof(int),
                     "Wrong number of bytes in file: got "
                     << size << " expected "
                     << number_*order_.size()*sizeof(int));
#endif
}

void WriteAssignmentContainer::set_cache_size(unsigned int words) {
  max_cache_=words;
  if (cache_.size()>max_cache_) flush();
}

void WriteAssignmentContainer::add_assignment(const Assignment& a) {
  IMP_USAGE_CHECK(a.size()==order_.size(),
                  "Sizes don't match: " << a.size()
                  << " vs " << order_.size());
  Ints ret= order_.get_list_ordered(a);
  cache_.insert(cache_.end(), ret.begin(), ret.end());
  ++number_;
  IMP_LOG_VERBOSE( "Added " << a << " cache is now " << cache_
          << std::endl);
  if (cache_.size() > max_cache_) flush();
}


/// Capped writer
CappedAssignmentContainer
::CappedAssignmentContainer(AssignmentContainer *c,
                                 int max,
                          std::string name):
  P(name),
  contained_(c),
  max_(max){
}

void CappedAssignmentContainer::check_number() const {
  if (get_number_of_assignments() > max_) {
    IMP_THROW("Too many assignments", ValueException);
  }
}



unsigned int CappedAssignmentContainer::get_number_of_assignments() const {
  return contained_->get_number_of_assignments();
}

Assignment CappedAssignmentContainer::get_assignment(unsigned int i) const {
  return contained_->get_assignment(i);
}


void CappedAssignmentContainer::add_assignment(const Assignment& a) {
  contained_->add_assignment(a);
  check_number();
}


///

ReadAssignmentContainer
::ReadAssignmentContainer(std::string dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name):
  AssignmentContainer(name),
  order_(s, all_particles) {
  // must be done first to initialize max_cache_
  set_cache_size(100000);
  struct stat data;
  stat(dataset.c_str(), &data);
  size_=data.st_size/sizeof(int)/s.size();
  IMP_LOG_TERSE( "Opened binary file with " << size_ << "assignments"
          << std::endl);
#ifdef _MSC_VER
  f_=open(dataset.c_str(), O_RDONLY|O_BINARY, 0);
#else
  f_=open(dataset.c_str(), O_RDONLY, 0);
#endif
  offset_=-1;
}


unsigned int ReadAssignmentContainer::get_number_of_assignments() const {
  return size_;
}

Assignment ReadAssignmentContainer::get_assignment(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Not enough assignments: " << i);
  if ( static_cast< int>(i) < offset_
      || (i+1-offset_)*order_.size() > cache_.size()) {
    cache_.resize(max_cache_);
    lseek(f_, i*sizeof(int)*order_.size(), SEEK_SET);
    int rd= read(f_, &cache_[0], max_cache_*sizeof(int));
    cache_.resize(rd/sizeof(int));
    offset_=i;
    IMP_LOG_TERSE( "Cache is of size " << cache_.size() << " at " << offset_
            << " when reading " << i << " with assignments of size "
            << order_.size() << std::endl);
  }
  return order_.get_subset_ordered(cache_.begin()+(i-offset_)*order_.size(),
                                   cache_.begin()+(i+1-offset_)*order_.size());
}

void ReadAssignmentContainer::set_cache_size(unsigned int words) {
  // make sure it is a whole number of assignments
  max_cache_=(words/order_.size() +1)*order_.size();
}

void ReadAssignmentContainer::add_assignment(const Assignment&) {
  IMP_NOT_IMPLEMENTED;
}





////////////////////////// RangeViewAssignmentContainer

inline unsigned int
RangeViewAssignmentContainer::get_number_of_assignments() const {
  return end_-begin_;
}

inline Assignment
RangeViewAssignmentContainer::get_assignment(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Invalid assignment requested: " << i);
  return inner_->get_assignment(i+begin_);
}
RangeViewAssignmentContainer
::RangeViewAssignmentContainer(AssignmentContainer *inner,
                               unsigned int begin, unsigned int end):
  AssignmentContainer("RangeViewAssignmentContainer%1%"),
  inner_(inner), begin_(begin),
  end_(std::min<unsigned int>(end, inner->get_number_of_assignments())) {}

void RangeViewAssignmentContainer::do_show(std::ostream &out) const {
  out << "inner: " << inner_->get_name() << std::endl;
  out << "range: " << begin_ << "..." << end_ << std::endl;
}



void RangeViewAssignmentContainer::add_assignment(const Assignment&) {
  IMP_NOT_IMPLEMENTED;
}


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
    Subset subset,
    unsigned int k,
    RestraintCache *rssf,
    std::string name):
    AssignmentContainer(name), subset_(subset), k_(k), rssf_(rssf) {
  rs_= get_as<Restraints>(rssf_->get_restraints(subset_, Subsets()));
  for (unsigned int i=0; i< rs_.size(); ++i) {
    slices_.push_back(rssf_->get_slice(rs_[i], subset_));
  }
}

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
  double score=std::numeric_limits<double>::max();
  if (rssf_){
    score=0;
    for (unsigned int i=0; i< rs_.size(); ++i) {
      score+=rssf_->get_score(rs_[i], slices_[i].get_sliced(a));
    }
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



IMPDOMINO_END_NAMESPACE
