/**
 *  \file interaction_graph.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_INTERACTION_GRAPH_H
#define IMPMISC_INTERACTION_GRAPH_H

#include "config.h"
#include <IMP/config.h>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPMISC_BEGIN_NAMESPACE


typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              boost::property<boost::vertex_name_t, Particle*>,
                              boost::property<boost::edge_name_t,
                                              Object*> > InteractionGraph;

IMPMISCEXPORT InteractionGraph get_interaction_graph(Model *m);


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_INTERACTION_GRAPH_H */
