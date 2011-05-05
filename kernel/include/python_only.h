/**
 *  \file IMP/python_only.h   \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_PYTHON_ONLY_H
#define IMP_PYTHON_ONLY_H

#include "kernel_config.h"
#include "Model.h"
#include <boost/graph/adjacency_list.hpp>

IMP_BEGIN_NAMESPACE

#ifdef IMP_DOXYGEN
/** \name Python Only
    The following functions are only available in python.
    @{
*/
/** Use the python \external{packages.python.org/altgraph/core.html,altgraph}
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
/** Export an IMP graph to a \external{networkx.lanl.gov/,networkx} graph.
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
/** @} */
#endif

IMP_END_NAMESPACE

#endif  /* IMP_PYTHON_ONLY_H */
