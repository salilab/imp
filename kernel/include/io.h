/**
 *  \file IMP/io.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_IO_H
#define IMP_IO_H

#include "config.h"
#include "Model.h"
#include "file.h"
#include "FailureHandler.h"

IMP_BEGIN_NAMESPACE
/** \name Model IO Models can be written to files in an easy-to-read
    file format based on "name: value" pairs on separate lines with
    indentation used to indicated nesting (as in python).

    If you are just interested in saving the state of a model during
    runtime, set IMP::ConfigurationSet instead. It should be
    substantially faster.

    The functions take a list of the particles to write. The particles
    need to be provided in the same order for reading and writing.
    This is not checked.

    If an passed particle has a particle attribute which is not
    in the list, an IOException is thrown. This can change to
    silently ignoring such attributes if desired.

    \note The versions taking std::map arguments are not exported
    to python.
    @{
*/
IMPEXPORT void write_model(Model *m,
                           const ParticlesTemp &particles,
                           TextOutput out);
IMPEXPORT void read_model(TextInput in,
                          const ParticlesTemp &particles,
                          Model *m);
#ifndef IMP_DOXYGEN
IMPEXPORT void write_model(Model *m,
                           TextOutput out);

IMPEXPORT void read_model(TextInput in, Model *m);
#endif
/** @} */

/** \brief Dump the state of the model to a file on an error and then
    go on the the other handlers.

    When an error (check or assertion failure) occurs, the model is
    dumped to the specified file.

    \verbinclude dump_on_error.py
 */
class IMPEXPORT DumpModelOnFailure: public FailureHandler {
  Model *m_;
  TextOutput file_name_;
 public:
  DumpModelOnFailure(Model *m, TextOutput out);
  IMP_FAILURE_HANDLER(DumpModelOnFailure);
};

IMP_END_NAMESPACE

#endif /* IMP_IO_H */
