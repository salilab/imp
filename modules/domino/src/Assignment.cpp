/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/Assignment.h>
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
  AssignmentContainer(name), width_(-1), k_(k), i_(0), place_(0, k_-1) {}


void SampleAssignmentContainer::do_show(std::ostream &out) const {
  out << "size: " << get_number_of_assignments() << std::endl;
  out << "width: " << width_ << std::endl;
}

void SampleAssignmentContainer::add_assignment(Assignment a) {
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

HDF5AssignmentContainer::HDF5AssignmentContainer(rmf::HDF5Group parent,
                                                 const Subset &s,
                                           const ParticlesTemp &all_particles,
                                                 std::string name):
  AssignmentContainer(name), ds_(parent.add_child_index_data_set(name, 2)),
  init_(false), order_(get_order(s, all_particles)) {}


HDF5AssignmentContainer
::HDF5AssignmentContainer(rmf::HDF5DataSet<rmf::IndexTraits> dataset,
                          const Subset &s,
                          const ParticlesTemp &all_particles,
                          std::string name):
  AssignmentContainer(name), ds_(dataset),
  init_(true), order_(get_order(s, all_particles)) {}


unsigned int HDF5AssignmentContainer::get_number_of_assignments() const {
  return ds_.get_size()[0];
}

Assignment HDF5AssignmentContainer::get_assignment(unsigned int i) const {
  Ints is= ds_.get_row(Ints(1,i));
  Ints ret(is.size());
  IMP_USAGE_CHECK(ret.size()== order_.size(), "Wrong size assignment");
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[order_[i]]=is[i];
  }
  return Assignment(ret);
}

void HDF5AssignmentContainer::add_assignment(Assignment a) {
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
  Ints sz= ds_.get_size();
  ++sz[0];
  ds_.set_size(sz);
  IMP_USAGE_CHECK(ds_.get_size()[1]==static_cast<int>(a.size()),
                  "Sizes don't match: " << a.size() << " vs "
                  << ds_.get_size()[1]);
  ds_.set_row(Ints(1,sz[0]-1), is);
}



void HDF5AssignmentContainer::do_show(std::ostream &) const {
  //out << "dataset: " << ds_.get_name() << std::endl;
}

#endif


IMPDOMINO_END_NAMESPACE
