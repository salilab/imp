/**
 *  \file spb_parser.h
 *  \brief SPB Parser
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_SPB_PARSER_H
#define IMPMEMBRANE_SPB_PARSER_H
#include <IMP.h>
#include <IMP/membrane.h>
#include <string>
#include "membrane_config.h"

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT SPBParameters get_SPBParameters(IMP::TextInput in,
                                                  std::string suffix);

IMPMEMBRANE_END_NAMESPACE

#endif /* IMPMEMBRANE_SPB_PARSER_H */
