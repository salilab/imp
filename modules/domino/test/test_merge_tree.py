import IMP
import IMP.test
import IMP.domino
import IMP.core


class Tests(IMP.test.TestCase):
    def test_global_min0(self):
        """Testing balanced merge tree algorithm"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for p in range(10)]
        jt= IMP.domino.SubsetGraph()
        for p in ps:
            jt.add_vertex(IMP.domino.Subset([p]))
        vs= jt.get_vertices()
        for i in range(1,10):
            jt.add_edge(i-1,i)
        #IMP.show_graphviz(jt)
        print jt
        print type(jt)
        mbt= IMP.domino.get_balanced_merge_tree(jt)
        #IMP.show_graphviz(mbt)

if __name__ == '__main__':
    IMP.test.main()
