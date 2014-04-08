/** \example base/flags.cpp
    Show simple usage of the command line flags support in IMP.base in a
    C++ executable. Run with "--help" to see all available flags.
*/
#include <IMP/base/flags.h>
#include <IMP/base/Flag.h>

int main(int argc, char *argv[]) {
  IMP::base::Flag<std::string> hello("hello", "How to say hello.",
                                     "Hello world");

  IMP::base::setup_from_argv(argc, argv, "Example on C++ use of flags.");

  std::cout << hello << std::endl;

  return 0;
}
