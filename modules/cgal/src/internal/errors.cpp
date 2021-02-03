/**
 *  \file  Sphere3D.cpp
 *  \brief simple implementation of spheres in 3D
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */
#include <IMP/exception.h>
#include <CGAL/assertions_behaviour.h>
#include <CGAL/assertions.h>

namespace {
void my_failure_function(const char *type, const char *expression,
                         const char *file, int line, const char *explanation) {
  std::ostringstream oss;
  oss << type << " error in CGAL " << expression << " at " << file << ": "
      << line << ": " << explanation << std::endl;
  IMP::handle_error(oss.str().c_str());
  throw IMP::UsageException(oss.str().c_str());
}
struct Registrar {
  Registrar() {
    CGAL::set_error_behaviour(CGAL::CONTINUE);
    CGAL::set_error_handler(my_failure_function);
  }
} registrar;
}
