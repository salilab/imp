/**
 * Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/domino.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP/core.h>
using namespace IMP;
using namespace IMP::domino;
using namespace IMP::atom;
using namespace IMP::algebra;
using namespace IMP::core;
using namespace IMP::container;
typedef IMP::atom::Hierarchy Hierarchy;

template <class Table>
void benchmark_table(AssignmentContainer *ac, std::string name) {
  double runtime, num=0;
  ac->set_was_used(true);
  IMP_TIME(
           {
             int vals[10];
             for (int i=0; i< 100000; ++i) {
               for (unsigned int j=0; j< 10; ++j) {
                 vals[j]=i+j;
               }
               Assignment cur(vals, vals+10);
               ac->add_assignment(cur);
             }
             num+=ac->get_number_of_assignments();
           }, runtime);
  IMP::benchmark::report(name, runtime, num);
}

int main(int argc, char *[]) {
  IMP_NEW(Model, m, ());
  for (unsigned int i=0; i< 10; ++i) {
    IMP_NEW(Particle, p, (m));
  }
  Subset s(m->get_particles());
  {
    RMF::HDF5File file
      = RMF::create_hdf5_file(create_temporary_file_name("benchmark", ".hdf5"));
    RMF::HDF5IndexDataSet2D ds=file.add_child_index_data_set_2d("data");
    IMP_NEW(HDF5AssignmentContainer, ac,(ds, s, m->get_particles(), "c"));
    ac->set_cache_size(1);
    benchmark_table<HDF5AssignmentContainer>(ac, "hdf5 no cache");
  }
  {
    RMF::HDF5File file
      = RMF::create_hdf5_file(create_temporary_file_name("benchmark", ".hdf5"));
    RMF::HDF5IndexDataSet2D ds=file.add_child_index_data_set_2d("data");
    IMP_NEW(HDF5AssignmentContainer, ac,(ds, s, m->get_particles(), "c"));
    ac->set_cache_size(1000000);
    benchmark_table<HDF5AssignmentContainer>(ac, "hdf5");
  }
  {
    IMP_NEW(PackedAssignmentContainer, ac,("hi"));
    benchmark_table<HDF5AssignmentContainer>(ac, "packed container");
  }
  {
    IMP_NEW(ListAssignmentContainer, ac,("hi"));
    benchmark_table<HDF5AssignmentContainer>(ac, "list container");
  }
  return IMP::benchmark::get_return_value();
}
