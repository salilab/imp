/**
 *  \file IMP/rmf/hierarchy_links_helpers.h
 *  \brief Helper functions for HierarchyLoad/SaveLink.h
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INTERNAL_HIERARCHY_LINKS_HELPERS_H
#define IMPRMF_INTERNAL_HIERARCHY_LINKS_HELPERS_H

#include <IMP/rmf/rmf_config.h>
#include <IMP/base_types.h>
#include <IMP/Model.h>
#include <RMF/NodeConstHandle.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

//! return a string that would be used to name h in the rmf file
std::string get_good_name_to_atom_node(Model *m, ParticleIndex h);

RMF::NodeConstHandle get_previous_rmf_provenance(RMF::NodeConstHandle node);

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_INTERNAL_HIERARCHY_LINKS_HELPERS_H */
