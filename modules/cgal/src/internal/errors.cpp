/**
 *  \file  SphereD<3>.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#include <IMP/exception.h>
#include <CGAL/assertions_behaviour.h>
#include <CGAL/assertions.h>

namespace {
  void my_failure_function (const char *type,
                            const char *expression,
                            const char *file,
                            int line,
                            const char *explanation) {
    std::ostringstream oss;
    oss << type << " error in CGAL " << expression << " at " << file << ": "
        << line << ": " << explanation;
    IMP::internal::assert_fail(oss.str().c_str());
    IMP_THROW(oss.str(), IMP::ValueException);
  }
  struct Registrar {
    Registrar() {
      CGAL::set_error_behaviour(CGAL::CONTINUE);
      CGAL::set_error_handler(my_failure_function);
    }
  } registrar;
}
