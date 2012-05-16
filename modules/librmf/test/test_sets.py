import unittest
import RMF
import random

class GenericTest(RMF.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def _test_set(self, f, Arity):
        ch= f.get_root_node().get_children()
        added=[]
        for i in range(0,5):
            tpl= random.sample(ch, Arity)
            tpl.sort()
            if tpl not in added:
                added.append(tpl)
                print tpl
                t= f.add_node_set(tpl, RMF.CUSTOM_SET)
        ts= f.get_node_sets(Arity)
        cat= f.add_category("hi", Arity)
        k= f.add_float_key(cat, "v", False)
        for t in ts:
            t.set_value(k, float(t.get_node(0).get_name()))
        for t in ts:
            self.assertEqual(t.get_value(k), float(t.get_node(0).get_name()))
    def test_perturbed(self):
        """Test node sets"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file_perturbed.rmf"))
        r= f.get_root_node()
        for i in range(0,10):
            cs= r.add_child(str(i), RMF.GEOMETRY)
        self._test_set(f, 2)
        self._test_set(f, 3)
        self._test_set(f, 4)

    def test_perturbed_pairs(self):
        """Test node pairs"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file_perturbed.rmf"))
        r= f.get_root_node()
        for i in range(0,10):
            cs= r.add_child(str(i), RMF.GEOMETRY)
        ch= r.get_children()
        added=[]
        for i in range(0,5):
            tpl= random.sample(ch, 2)
            tpl.sort()
            if tpl not in added:
                added.append(tpl)
                print tpl
                t= f.add_node_pair(tpl, RMF.BOND)
        ts= f.get_node_pairs()
        cat= f.add_pair_category("hi")
        k= f.add_float_key(cat, "v", False)
        for t in ts:
            t.set_value(k, float(t.get_node(0).get_name()))
        for t in ts:
            self.assertEqual(t.get_value(k), float(t.get_node(0).get_name()))
if __name__ == '__main__':
    unittest.main()
