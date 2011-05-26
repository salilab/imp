/**
 *  \file domino/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/Assignment.h>



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

#ifdef IMP_DOMINO_USE_IMP_RMF

HDF5AssignmentContainer::HDF5AssignmentContainer(rmf::HDF5Group parent,
                                                 std::string name):
  AssignmentContainer(name), ds_(parent.add_child_index_data_set(name, 2)),
  init_(false) {}


unsigned int HDF5AssignmentContainer::get_number_of_assignments() const {
  return ds_.get_size()[0];
}

Assignment HDF5AssignmentContainer::get_assignment(unsigned int i) const {
  Ints is= ds_.get_row(Ints(1,i));
  return Assignment(is.begin(), is.end());
}

void HDF5AssignmentContainer::add_assignment(Assignment a) {
  if (!init_) {
    Ints sz(2);
    sz[0]=0; sz[1]=a.size();
    ds_.set_size(sz);
    init_=true;
  }
  Ints is(a.begin(), a.end());
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
