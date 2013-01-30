
def get_networkx_graph(ig):
    import networkx
    g= networkx.DiGraph()
    if len(ig.get_vertices())==0:
        return g
    class NodeWrapper:
        def __init__(self, p):
            self.p=p
        def __str__(self):
            return self.p.get_name()
        def __call__(self, name):
            return self.p.__call__(name)

    for vi in ig.get_vertices():
        n= ig.get_vertex_name(vi)
        g.add_node(NodeWrapper(n))
    for vi in ig.get_vertices():
        n= ig.get_vertex_name(vi)
        for ni in ig.get_out_neighbors(vi):
            nn= ig.get_vertex_name(ni)
            g.add_edge(NodeWrapper(n), NodeWrapper(nn))
    return g


def show_altgraph(g):
    def clean(name):
        try:
            n0=name.get_name()
        except:
            n0=str(name)
        n1= str(n0).replace('"','')
        n2= n1.replace("\n",'')
        return n2
    import altgraph
    from altgraph import Graph, Dot
    graph= Graph.Graph()
    for i,v in enumerate(g.get_vertices()):
        graph.add_node(i) #, node_data=g.get_vertex_name(v)
    for i,v in enumerate(g.get_vertices()):
        for n in g.get_out_neighbors(v):
            graph.add_edge(v, n)
    dot = Dot.Dot(graph) #, graph_type="digraph"
    for i,v in enumerate(g.get_vertices()):
        dot.node_style(i, label=clean(g.get_vertex_name(v)))
    dot.display()

def show_graphviz(g):
    import IMP.base
    tfn= IMP.base.create_temporary_file_name("graph", ".dot")
    tfon= IMP.base.create_temporary_file_name("graph", ".pdf")
    st= g.get_graphviz_string()
    open(tfn, "w").write(st)
    try:
        import subprocess
    except ImportError:
        import sys
        print >> sys.stderr, "Cannot run dot on Python 2.3 systems."
        return
    try:
        print "running dot"
        sp=subprocess.Popen(["dot", "-Tpdf", tfn, "-o"+tfon])
        sp.wait()
    except:
        import sys
        print >> sys.stderr, "The dot command from the graphviz package was not found. Please make sure it is in the PATH passed to IMP."
        return
    try:
        import platform
        if platform.system() =="Darwin":
            cmd="open"
        else:
            cmd="acroread"
        print "launching viewer", cmd
        subprocess.Popen([cmd, tfon])
    except:
        print "Could not display file. It is saved at", tfon
    return tfon
