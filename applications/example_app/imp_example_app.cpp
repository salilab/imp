/** This file is in the public domain.

 */

#include <IMP/log.h>
#include <IMP/base/flags.h>

int main(int argc, char* argv[]) {
  IMP::base::setup_from_argv(argc, argv, "An example application");
  IMP_LOG(IMP::SILENT, "Hello world" << std::endl);
  return 0;
}
