import unittest
import IMP
import IMP.test
import IMP.core

class TestREFCover(IMP.test.TestCase):
    """Tests copying derivatives to and from refined"""

    def _create_particle(self, m, ks):
        p= IMP.Particle(m)
        for k in ks:
            p.add_attribute(k, 0.0)
        return p
    def _setup(self):
        k0= IMP.FloatKey("Key0")
        k1= IMP.FloatKey("Key1")
        ks= IMP.FloatKeys()
        ks.append(k0)
        ks.append(k1)
        m= IMP.Model()
        p0= self._create_particle(m, ks)
        p1= self._create_particle(m, ks)
        p2= self._create_particle(m, ks)
        h0= IMP.core.HierarchyDecorator.create(p0)
        h1= IMP.core.HierarchyDecorator.create(p1)
        h2= IMP.core.HierarchyDecorator.create(p2)
        h0.add_child(h1)
        h0.add_child(h2)
        r= IMP.core.ChildrenRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        return h0, h1, h2, p0, p1, p2, r, ks, m
    def test_to(self):
        """Test copying derivatives to refined"""
        h0, h1, h2, p0, p1, p2, r, ks, m= self._setup()
        da= IMP.DerivativeAccumulator()
        p0.add_to_derivative(ks[0], 1, da)
        p0.add_to_derivative(ks[1], 2, da)
        prop= IMP.core.DerivativesToRefined(r, ks)
        da= IMP.DerivativeAccumulator()
        prop.apply(p0, da)
        self.assertEqual(p1.get_derivative(ks[0]), 1)
        self.assertEqual(p1.get_derivative(ks[1]), 2)
        self.assertEqual(p2.get_derivative(ks[0]), 1)
        self.assertEqual(p2.get_derivative(ks[1]), 2)
    def test_from(self):
        """Test copying derivatives from refined"""
        h0, h1, h2, p0, p1, p2, r, ks, m= self._setup()
        da= IMP.DerivativeAccumulator()
        p1.add_to_derivative(ks[0], 1, da)
        p1.add_to_derivative(ks[1], 2, da)
        p2.add_to_derivative(ks[0], 4, da)
        p2.add_to_derivative(ks[1], 8, da)
        prop= IMP.core.DerivativesFromRefined(r, ks)
        da= IMP.DerivativeAccumulator()
        prop.apply(p0, da)
        self.assertEqual(p0.get_derivative(ks[0]), 5)
        self.assertEqual(p0.get_derivative(ks[1]), 10)

if __name__ == '__main__':
    unittest.main()
