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
SubsetGraphTable::SubsetGraphTable(std::string name): Object(name){}

JunctionTreeTable::JunctionTreeTable(RestraintSet *rs): rs_(rs){}

SubsetGraph
JunctionTreeTable::get_subset_graph(ParticleStatesTable *pst) const {
  IMP_LOG(TERSE, "Computing junction tree\n");
  InteractionGraph ig= get_interaction_graph(pst->get_particles(),
                                             get_restraints(rs_));
  return get_junction_tree(ig);
}

void JunctionTreeTable::do_show(std::ostream &out) const {
  RestraintsTemp rst=get_restraints(rs_);
  for (unsigned int i=0; i< rst.size(); ++i) {
    out << "\"" << rst[i]->get_name() << "\"\n";
  }
}

Subsets get_subsets(const SubsetGraph &g){
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, g);
    Subsets output;
    for (unsigned int vi = 0;vi < boost::num_vertices(g);vi++) {
      output.push_back(boost::get(subset_map,vi));
  }
    return output;
  }



StoredSubsetGraphTable::StoredSubsetGraphTable(const SubsetGraph &sg,
                                               std::string name):
  SubsetGraphTable(name),
  sg_(sg){}

SubsetGraph
StoredSubsetGraphTable::get_subset_graph(ParticleStatesTable *) const {
  return sg_;
}

void StoredSubsetGraphTable::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
