import IMP
import IMP.atom
import IMP.test
from io import BytesIO

class Tests(IMP.test.TestCase):

    def test_hierarchy_tree(self):
        """Test the HierarchyTree class"""
        m = IMP.Model()
        p0 = IMP.Particle(m)
        p0.set_name("HT_P0")
        h0 = IMP.atom.Hierarchy.setup_particle(p0)
        p1 = IMP.Particle(m)
        p1.set_name("HT_P1")
        h1 = IMP.atom.Hierarchy.setup_particle(p1)

        t = IMP.atom.HierarchyTree()
        desc0 = t.add_vertex(h0)
        desc1 = t.add_vertex(h1)
        t.add_edge(desc0, desc1)
        self.assertIsInstance(t.get_graph(), IMP.atom.HierarchyTree)
        self.assertEqual(sorted(t.get_vertices()), sorted([desc0, desc1]))
        n = t.get_vertex_name(desc0)
        self.assertIsInstance(n, IMP.atom.Hierarchy)
        self.assertEqual(n.get_name(), "HT_P0")
        self.assertEqual(list(t.get_in_neighbors(desc0)), [])
        self.assertEqual(t.get_out_neighbors(desc0), [desc1])
        self.assertEqual(t.get_in_neighbors(desc1), [desc0])
        self.assertEqual(list(t.get_out_neighbors(desc1)), [])
        sio = BytesIO()
        t.show_graphviz(sio)
        self.assertEqual(sio.getvalue(),
                         t.get_graphviz_string().encode('ascii'))
        t.remove_vertex(desc1)
        self.assertEqual(t.get_vertices(), [desc0])
        self.assertRaisesUsageException(t.remove_vertex, desc1)

if __name__ == '__main__':
    IMP.test.main()
