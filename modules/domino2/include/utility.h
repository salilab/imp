/**
 *  \file domino2/utility.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_UTILITY_H
#define IMPDOMINO2_UTILITY_H

#include "domino2_config.h"

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPDOMINO2_BEGIN_NAMESPACE


typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              boost::property<boost::vertex_name_t, Particle*>,
                              boost::property<boost::edge_name_t,
                                              Object*> > InteractionGraph;

IMPDOMINO2EXPORT InteractionGraph get_interaction_graph(Model *m);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_UTILITY_H */
