import IMP
import IMP.test
import IMP.core
import pickle

class TestVisitor(IMP.core.HierarchyVisitor):
    def __init__(self, descend=True):
        super().__init__()
        self.descend = descend
        self.seen = []
    def __call__(self, h):
        self.seen.append(h.get_name())
        return self.descend

class Tests(IMP.test.TestCase):

    def _make_test_hierarchy(self, m):
        pp = IMP.Particle(m)
        ppd = IMP.core.Hierarchy.setup_particle(pp)
        pp.set_name("root")
        for i in range(0, 2):
            pc = IMP.Particle(m)
            pcd = IMP.core.Hierarchy.setup_particle(pc)
            ppd.add_child(pcd)
            pc.set_name("child %d" % i)
            for j in range(0, 3):
                gpc = IMP.Particle(m)
                gpcd = IMP.core.Hierarchy.setup_particle(gpc)
                pcd.add_child(gpcd)
                gpc.set_name("grandchild %d-%d" % (i,j))
        return ppd

    def test_hierarchy(self):
        """Check that the hierarchy works fine"""
        m = IMP.Model()
        ppd = self._make_test_hierarchy(m)
        self.assertEqual(ppd.get_number_of_children(), 2)

    def test_trigger(self):
        """Check Hierarchy-changed Model trigger"""
        m = IMP.Model()
        tk = IMP.core.Hierarchy.get_changed_key()
        # No hierarchy created yet, so trigger should be 0
        self.assertEqual(m.get_trigger_last_updated(tk), 0)
        self.assertEqual(m.get_age(), 1)
        ppd = self._make_test_hierarchy(m)
        self.assertEqual(m.get_trigger_last_updated(tk), 1)
        self.assertEqual(m.get_age(), 1)

        # Update should increase model age but not change trigger
        m.update()
        self.assertEqual(m.get_trigger_last_updated(tk), 1)
        self.assertEqual(m.get_age(), 2)

        # Adding a child should update the trigger
        pc = IMP.Particle(m)
        pcd = IMP.core.Hierarchy.setup_particle(pc)
        ppd.add_child(pcd)
        self.assertEqual(m.get_trigger_last_updated(tk), 2)

        m.update()
        pc = IMP.Particle(m)
        pcd = IMP.core.Hierarchy.setup_particle(pc)
        ppd.add_child_at(pcd, 0)
        self.assertEqual(m.get_trigger_last_updated(tk), 3)

        # Removing a child should update the trigger
        m.update()
        ppd.remove_child(pcd)
        self.assertEqual(m.get_trigger_last_updated(tk), 4)

        m.update()
        ppd.remove_child(0)
        self.assertEqual(m.get_trigger_last_updated(tk), 5)

        # Clearing all children should update the trigger
        m.update()
        ppd.clear_children()
        self.assertEqual(m.get_trigger_last_updated(tk), 6)

    def test_depth_visit_count(self):
        """Check depth-first hierarchy traversal"""
        m = IMP.Model()
        ppd = self._make_test_hierarchy(m)
        v = IMP.core.HierarchyCounter()
        IMP.core.visit_depth_first(ppd, v)
        self.assertEqual(v.get_count(), 9)

    def test_hierarchy_counter_pickle(self):
        """Test (un-)pickle of HierarchyCounter"""
        v1 = IMP.core.HierarchyCounter()
        v1(IMP.core.Hierarchy())
        self.assertEqual(v1.get_count(), 1)
        v2 = IMP.core.HierarchyCounter()
        self.assertEqual(v2.get_count(), 0)
        v2.foo = 'bar'
        dump = pickle.dumps((v1, v2))

        newv1, newv2 = pickle.loads(dump)
        self.assertEqual(v1.get_count(), newv1.get_count())
        self.assertEqual(v2.get_count(), newv2.get_count())
        self.assertEqual(newv2.foo, 'bar')

        self.assertRaises(TypeError, v1._set_from_binary, 42)

    def test_breadth_visit_count(self):
        """Check breadth-first hierarchy traversal"""
        m = IMP.Model()
        ppd = self._make_test_hierarchy(m)
        v = IMP.core.HierarchyCounter()
        IMP.core.visit_breadth_first(ppd, v)
        self.assertEqual(v.get_count(), 9)

    def test_depth_visitor_python(self):
        """Check depth-first hierarchy traversal with a Python visitor"""
        m = IMP.Model()
        ppd = self._make_test_hierarchy(m)
        v = TestVisitor(True)
        IMP.core.visit_depth_first(ppd, v)
        self.assertEqual(v.seen,
             ['root', 'child 0', 'grandchild 0-0', 'grandchild 0-1',
              'grandchild 0-2', 'child 1', 'grandchild 1-0', 'grandchild 1-1',
              'grandchild 1-2'])

    def test_breadth_visitor_python(self):
        """Check breadth-first hierarchy traversal with a Python visitor"""
        m = IMP.Model()
        ppd = self._make_test_hierarchy(m)
        v = TestVisitor(True)
        IMP.core.visit_breadth_first(ppd, v)
        self.assertEqual(v.seen,
             ['root', 'child 1', 'child 0', 'grandchild 1-2',
              'grandchild 1-1', 'grandchild 1-0', 'grandchild 0-2',
              'grandchild 0-1', 'grandchild 0-0'])

    def test_alt_hierarchy(self):
        """Check that an alternate hierarchy works fine"""
        t = IMP.core.HierarchyTraits("my_hierarchy")
        m = IMP.Model()
        pp = IMP.Particle(m)
        ppd = IMP.core.Hierarchy.setup_particle(pp, t)
        pp.set_name("root")
        for i in range(0, 10):
            pc = IMP.Particle(m)
            pcd = IMP.core.Hierarchy.setup_particle(pc, t)
            ppd.add_child(pcd)
            pc.set_name(str("child " + str(i)))

        self.assertEqual(ppd.get_number_of_children(), 10,
                         "Number of children is not as expected")

if __name__ == '__main__':
    IMP.test.main()
