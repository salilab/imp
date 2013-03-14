/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/domino.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP/core.h>
#ifdef IMP_DOMINO_USE_IMP_RMF
#include <RMF/HDF5.h>
#endif

namespace {

#if IMP_BUILD == IMP_DEBUG
static const int number_of_values = 1000;
#else
static const int number_of_values = 100000;
#endif

using namespace IMP;
using namespace IMP::domino;
using namespace IMP::algebra;
using namespace IMP::core;
using namespace IMP::container;

template <class Table>
void benchmark_table(AssignmentContainer *ac, std::string name) {
  double runtime, num=0;
  ac->set_was_used(true);
  IMP_TIME(
           {
             int vals[10];
             for (int i=0; i< number_of_values; ++i) {
               for (unsigned int j=0; j< 10; ++j) {
                 vals[j]=i+j;
               }
               Assignment cur(vals, vals+10);
               ac->add_assignment(cur);
             }
             num+=ac->get_number_of_assignments();
           }, runtime);
  IMP::benchmark::report("save assignments", name, runtime, num);
}
}

int main(int , char *[]) {
  IMP_NEW(Model, m, ());
  for (unsigned int i=0; i< 10; ++i) {
    IMP_NEW(Particle, p, (m));
  }
  Subset s(m->get_particles());
#ifdef IMP_DOMINO_USE_IMP_RMF
  {
    RMF::HDF5::File file
      = RMF::HDF5::create_file(create_temporary_file_name("benchmark",
                                                          ".hdf5"));
    RMF::HDF5::IndexDataSet2D ds=file.add_child_index_data_set_2d("data");
    IMP_NEW(WriteHDF5AssignmentContainer, ac,(ds, s, m->get_particles(), "c"));
    ac->set_cache_size(1);
    benchmark_table<WriteHDF5AssignmentContainer>(ac, "hdf5 no cache");
  }
  {
    RMF::HDF5::File file
      = RMF::HDF5::create_file(create_temporary_file_name("benchmark",
                                                          ".hdf5"));
    RMF::HDF5::IndexDataSet2D ds=file.add_child_index_data_set_2d("data");
    IMP_NEW(WriteHDF5AssignmentContainer, ac,(ds, s, m->get_particles(), "c"));
    ac->set_cache_size(1000000);
    benchmark_table<WriteHDF5AssignmentContainer>(ac, "hdf5");
  }
#endif
  {
    std::string name=create_temporary_file_name("benchmark", ".assignments");
    IMP_NEW(WriteAssignmentContainer, ac,(name, s, m->get_particles(), "c"));
    ac->set_cache_size(1000000);
    benchmark_table<WriteAssignmentContainer>(ac, "direct");
  }
  {
    IMP_NEW(PackedAssignmentContainer, ac,("hi"));
    benchmark_table<PackedAssignmentContainer>(ac, "packed container");
  }
  {
    IMP_NEW(ListAssignmentContainer, ac,("hi"));
    benchmark_table<ListAssignmentContainer>(ac, "list container");
  }
  return IMP::benchmark::get_return_value();
}
