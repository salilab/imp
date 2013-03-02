import IMP
import IMP.test
import IMP.domino
import IMP.core
import StringIO


class Tests(IMP.test.TestCase):
    def test_global_min0(self):
        """Testing writing and reading a merge tree"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for p in range(10)]
        jt= IMP.domino.SubsetGraph()
        for p in ps:
            jt.add_vertex(IMP.domino.Subset([p]))
        vs= jt.get_vertices()
        for i in range(1,10):
            jt.add_edge(i-1,i)
        #IMP.show_graphviz(jt)
        mbt= IMP.domino.get_balanced_merge_tree(jt)
        #IMP.show_graphviz(mbt)
        print mbt.get_graphviz_string()
        buf= StringIO.StringIO()
        IMP.domino.write_merge_tree(mbt, ps, buf)
        print "graph begin"
        print buf.getvalue()
        print "graph end"
        mbt2= IMP.domino.read_merge_tree(StringIO.StringIO(buf.getvalue()), ps)
        #IMP.show_graphviz(mbt)
        #IMP.show_graphviz(mbt2)
        self.assertEqual(mbt2.get_vertices(),
                         mbt.get_vertices())
        self.assertEqual(mbt2.get_vertex_name(0),
                         mbt.get_vertex_name(0))
if __name__ == '__main__':
    IMP.test.main()
