/**
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/benchmark/command_line_macros.h>
/** This benchmark does nothing and is here to experiment with
    benchmarking support.*/
int main(int argc, char** argv) {
  IMP_BENCHMARK( );
  {
    double sum=0, time;
    IMP_TIME({
        for (unsigned int i=0; i< 100; ++i) {
          double *d= new double(i);
          std::size_t iv= reinterpret_cast<std::size_t>(d);
          sum+=iv;
          delete d;
        }
      }, time);
      IMP::benchmark::report("alloc and free", time, sum);
  }
  return IMP::benchmark::get_return_value();
}
