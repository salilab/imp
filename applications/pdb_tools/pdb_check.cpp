/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/atom/pdb.h>
#include <IMP/base/flags.h>
#include <IMP/atom/force_fields.h>
#include <IMP/base/exception.h>

int main(int argc, char **argv) {
  try {
    std::vector<std::string> argcs
        = IMP::base::setup_from_argv(argc, argv,
                                     "Print warnings about pdb file",
                                     "input.pdb",
                                     -1);

    IMP::base::set_log_level(IMP::base::WARNING);
    for (unsigned int i=0; i< argcs.size(); ++i) {
      IMP_NEW(IMP::Model, m, ());
      m->set_log_level(IMP::base::SILENT);
      IMP::atom::Hierarchies inhs;
      IMP_CATCH_AND_TERMINATE(inhs= IMP::atom::read_multimodel_pdb(argcs[i],
                                                                   m));
      for (unsigned int i=0; i< inhs.size(); ++i) {
        IMP::atom::add_bonds(inhs[i]);
      }
    }
    return 0;
  } catch (const IMP::base::Exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
