/** \example flags.cpp
    Show simple usage of the command line flags support in IMP in a
    C++ executable. Run with "--help" to see all available flags.
*/
#include <IMP/flags.h>
#include <IMP/Flag.h>

int main(int argc, char *argv[]) {
  IMP::Flag<std::string> hello("hello", "How to say hello.",
                                     "Hello world");

  IMP::setup_from_argv(argc, argv, "Example on C++ use of flags.");

  std::cout << hello << std::endl;

  return 0;
}
