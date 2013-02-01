/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/base.h>
#include <IMP/algebra.h>
#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>
using namespace IMP;
using namespace IMP::base;
using namespace IMP::algebra;
using namespace IMP::benchmark;

namespace {
template <class KNN>
void test(std::string name,
          const Vector3Ds &pts,
          const KNN &knn,
          double eps) {
  double result=0;
  double runtime;
  IMP_TIME({
      for (unsigned int i=0; i< pts.size(); ++i) {
        Ints ret(2);
        knn.fill_nearest_neighbors(pts[i], 2U, eps, ret);
        result+=ret[1];
      }
    }, runtime);
  std::ostringstream oss;
  oss << "knn " << name << " " << knn.get_number_of_points() << " " << eps;
  report(oss.str(), runtime, result/pts.size());
}

template <class KNN>
void test_uniform(std::string name,
                  unsigned int n, double eps) {
  set_log_level(SILENT);
  set_check_level(NONE);
  BoundingBox3D bb(Vector3D(0,0,0),
                   Vector3D(10,10,10));
  Vector3Ds pts;
  for (unsigned int i=0; i< n; ++i) {
    pts.push_back(get_random_vector_in(bb));
  }
  KNN knn(pts.begin(), pts.end());
  test(name+" uniform", pts, knn, eps);
}

  template <class Data>
  void test_all(std::string name) {
    test_uniform<Data>(name, 10, 0);
    //test_uniform<Data>(name, 10, .1);
    test_uniform<Data>(name, 10, .5);
    test_uniform<Data>(name, 100, 0);
    //test_uniform<Data>(name, 100, .1);
    test_uniform<Data>(name, 100, .5);
    test_uniform<Data>(name, 1000, 0);
    //test_uniform<Data>(name, 1000, .1;
    test_uniform<Data>(name, 1000, .5);
    test_uniform<Data>(name, 10000, 0);
    //test_uniform<Data>(name, 10000, .1;
    test_uniform<Data>(name, 10000, .5);
  }
}

int main(int argc, char** argv) {
  IMP::base::setup_from_argv(argc, argv, "Benchmark nearest neighbor algos");
#ifdef IMP_BENCHMARK_USE_ANN
  {
    std::string name("ann");
    typedef IMP::algebra::internal::ANNData Data;
    test_all<Data>(name);
}
#endif
#ifdef IMP_ALGEBRA_USE_IMP_CGAL
  {
    std::string name("cgal");
    typedef IMP::cgal::internal::KNNData Data;
    test_all<Data>(name);
}
#endif
  {
    std::string name("linear");
    typedef IMP::algebra::internal::LinearKNNData<3> Data;
    test_all<Data>(name);
}
  return IMP::benchmark::get_return_value();
}
