/**
 *  \file domino2/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/domino2_config.h>
#include <IMP/domino2/subset_graphs.h>
#include <IMP/domino2/utility.h>



IMPDOMINO2_BEGIN_NAMESPACE
SubsetGraphTable::SubsetGraphTable(): Object("SubsetGraphTable"){}

JunctionTreeTable::JunctionTreeTable(RestraintSet *rs): rs_(rs){}

SubsetGraph
JunctionTreeTable::get_subset_graph(ParticleStatesTable *pst) const {
  IMP_LOG(TERSE, "Computing junction tree\n");
  InteractionGraph ig= get_interaction_graph(pst->get_particles(),
                                             get_restraints(rs_).first);
  return get_junction_tree(ig);
}

void JunctionTreeTable::do_show(std::ostream &out) const {
  RestraintsTemp rst=get_restraints(rs_).first;
  for (unsigned int i=0; i< rst.size(); ++i) {
    out << "\"" << rst[i]->get_name() << "\"\n";
  }
}

IMPDOMINO2_END_NAMESPACE
