## \example kernel/graph.py
# A simple example showing how to use the graph interface for in Python.

from __future__ import print_function
import IMP
import sys

IMP.setup_from_argv(sys.argv, "Graph interface")

m = IMP.Model()
# An undirected graph with an IMP::Object for each node
g = IMP.DependencyGraph()
vs = []
ps = []
for i in range(0, 10):
    ps.append(IMP.Particle(m))
    vs.append(g.add_vertex(ps[-1]))
g.add_edge(vs[0], vs[1])
g.add_edge(vs[1], vs[2])

# try to use the altgraph package to visualize
IMP.show_graphviz(g)
try:
    IMP.show_graphviz(g)
except:
    print("Oh well, no altgraph")

try:
    import matplotlib
    # the engine to be used must be selected before pyplot is imported
    matplotlib.use("macosx")
    import matplotlib.pyplot

    # the method below requires the altgraph Python package
    xg = IMP.get_networkx_graph(g)

    # the networkx visualization tools suck, so skip them
    # import networkx
    # networkx.draw(xg)
    # networkx.draw_shell(xg)
    # matplotlib.pyplot.show()
except:
    print("networkx not fully installed")

g.remove_vertex(0)

# we can also try another show method
try:
    IMP.show_graphviz(g)
except:
    print("oh well, something not working with graphviz")

# finally, we can
# in and out neighbors are the same
for n in g.get_in_neighbors(8):
    print(g.get_vertex_name(n).get_name())
