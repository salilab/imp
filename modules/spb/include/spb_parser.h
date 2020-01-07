/**
 *  \file IMP/spb/spb_parser.h
 *  \brief SPB Parser
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_SPB_PARSER_H
#define IMPSPB_SPB_PARSER_H
#include <IMP.h>
#include <IMP/spb.h>
#include <string>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT SPBParameters get_SPBParameters(IMP::TextInput in,
                                                  std::string suffix);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_SPB_PARSER_H */
