/**
 *  \file IMP/kernel/python_only.h   \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PYTHON_ONLY_H
#define IMPKERNEL_PYTHON_ONLY_H

#include <IMP/kernel/kernel_config.h>
#include "Model.h"
#include <boost/graph/adjacency_list.hpp>

IMPKERNEL_BEGIN_NAMESPACE

#ifdef IMP_DOXYGEN
/** \name Python Only
    The following functions are only available in python.
    @{
*/
/** The boost graphs used on the C++ side in \imp cannot easily be
    exported to python. Instead we provide a simple graph wrapper and
    a set of helper functions.

    Each node of the graph has an associated vertex name, the exact type
    of which depends on the graph.

    \pythononlyclass
*/
class PythonDirectedGraph {
public:
  class VertexDescriptor;
  class VertexDescriptors;
  class VertexName;
  Vertices get_vertices() const;
  VertexName get_vertex_name(VertexDescriptor v) const;
  VertexDescriptors get_in_neighbors(VertexDescriptor) const;
  VertexDescriptors get_out_neighbors(VertexDescriptor) const;
  void add_edge(VertexDescriptor, VertexDescriptor);
  VertexDescriptor add_vertex(VertexName);
  void remove_vertex(VertexDescriptor);
  /** Show the graph in the graphviz format.
      \note The vertices are not output in order, but have
      their indices shown.
  */
  void show(std::ostream &out=std::cout) const;
};

/** Use the python \external{http://packages.python.org/altgraph/core.html,altgraph}
    package to graphically display a graph.
    Note that is requires X11 to be running (you can find it in
    your Utilities folder in MacOS) and the window produced is shockingly
    retro.

    An alterntative is to write the graph to a \c dot file
    \code
    g.show_graphviz(open("graph.dot", "w"));
    \endcode
    then turn it to pdf with \c dot
    \command{dot -Tpdf graph.dot > graph.pdf}
    and finally view the pdf.
*/
void show_altgraph(Graph g);
/** Export an IMP graph to a \external{http://networkx.lanl.gov/,networkx} graph.
    The resulting graph can be analyzed and displaye, although the graph
    layout algorithms in networkx seem to be quite poor compared to
    graphviz.

    One a mac, such graphs can be displayed by
\code
import matplotlib
# the engine to be used must be selected before pyplot is imported
matplotlib.use("macosx")
import matplotlib.pyplot as plt

import networkx
networkx.draw_spring(xg)
plt.show()
\endcode
 */
networkx::DiGraph get_networkx_graph(Graph g);


/** Generate a nice pdf of the graph and attempt to open the pdf. The name
    of the pdf will be returned in case the attempt to open it fails.
*/
std::string show_graphviz(Graph g);
/** @} */
#endif

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_PYTHON_ONLY_H */
