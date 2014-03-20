/**
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 */
#include <IMP/base/flags.h>
#include <IMP/domino.h>
#include <IMP/container.h>
#include <IMP/benchmark.h>
#include <IMP/core.h>
#ifdef IMP_DOMINO_USE_IMP_RMF
#include <RMF/HDF5.h>
#endif

namespace {

using namespace IMP;
using namespace IMP::domino;
using namespace IMP::algebra;
using namespace IMP::core;
using namespace IMP::container;

template <class Table>
void benchmark_table(AssignmentContainer *ac, std::string name,
                     int number_of_values) {
  double runtime, num = 0;
  ac->set_was_used(true);
  IMP_TIME({
             int vals[10];
             for (int i = 0; i < number_of_values; ++i) {
               for (unsigned int j = 0; j < 10; ++j) {
                 vals[j] = i + j;
               }
               Assignment cur(vals, vals + 10);
               ac->add_assignment(cur);
             }
             num += ac->get_number_of_assignments();
           },
           runtime);
  IMP::benchmark::report("save assignments", name, runtime, num);
}
}

int main(int argc, char *argv[]) {
  IMP::base::setup_from_argv(argc, argv, "benchmark domino containers");
  IMP_NEW(kernel::Model, m, ());
  kernel::ParticlesTemp ps;
  for (unsigned int i = 0; i < 10; ++i) {
    IMP_NEW(kernel::Particle, p, (m));
    ps.push_back(p);
  }
  int number_of_values;
  if (IMP_BUILD == IMP_DEBUG || IMP::base::run_quick_test) {
    number_of_values = 1000;
  } else {
    number_of_values = 100000;
  }
  Subset s(ps);
#ifdef IMP_DOMINO_USE_IMP_RMF
  {
    RMF::HDF5::File file = RMF::HDF5::create_file(
        base::create_temporary_file_name("benchmark", ".hdf5"));
    RMF::HDF5::IndexDataSet2D ds = file.add_child_index_data_set_2d("data");
    IMP_NEW(WriteHDF5AssignmentContainer, ac, (ds, s, ps, "c"));
    ac->set_cache_size(1);
    benchmark_table<WriteHDF5AssignmentContainer>(ac, "hdf5 no cache",
                                                  number_of_values);
  }
  {
    RMF::HDF5::File file = RMF::HDF5::create_file(
        base::create_temporary_file_name("benchmark", ".hdf5"));
    RMF::HDF5::IndexDataSet2D ds = file.add_child_index_data_set_2d("data");
    IMP_NEW(WriteHDF5AssignmentContainer, ac, (ds, s, ps, "c"));
    ac->set_cache_size(1000000);
    benchmark_table<WriteHDF5AssignmentContainer>(ac, "hdf5", number_of_values);
  }
#endif
  {
    std::string name =
        base::create_temporary_file_name("benchmark", ".assignments");
    IMP_NEW(WriteAssignmentContainer, ac, (name, s, ps, "c"));
    ac->set_cache_size(1000000);
    benchmark_table<WriteAssignmentContainer>(ac, "direct", number_of_values);
  }
  {
    IMP_NEW(PackedAssignmentContainer, ac, ("hi"));
    benchmark_table<PackedAssignmentContainer>(ac, "packed container",
                                               number_of_values);
  }
  {
    IMP_NEW(ListAssignmentContainer, ac, ("hi"));
    benchmark_table<ListAssignmentContainer>(ac, "list container",
                                             number_of_values);
  }
  return IMP::benchmark::get_return_value();
}
