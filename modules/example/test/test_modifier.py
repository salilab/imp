from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example
import pickle


def make_modifier():
    m = IMP.Model()
    bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                   IMP.algebra.Vector3D(10, 10, 10))
    p = m.add_particle("p1")
    d = IMP.core.XYZ.setup_particle(m, p, IMP.algebra.Vector3D(-4, 13, 28))
    s = IMP.example.ExampleSingletonModifier(bb)
    return m, p, d, s


class Tests(IMP.test.TestCase):

    def test_modifier(self):
        """Test example SingletonModifier"""
        for typ in (IMP.example.ExampleSingletonModifier,
                    IMP.example.PythonExampleSingletonModifier):
            m = IMP.Model()
            bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                           IMP.algebra.Vector3D(10, 10, 10))
            p = m.add_particle("p1")
            d = IMP.core.XYZ.setup_particle(m, p,
                                            IMP.algebra.Vector3D(-4, 13, 28))
            s = typ(bb)
            s.apply_index(m, p)
            self.assertLess(IMP.algebra.get_distance(d.get_coordinates(),
                                           IMP.algebra.Vector3D(6,3,8)), 1e-4)
            self.assertIn("SingletonModifier", str(s))
            self.assertIn("SingletonModifier", repr(s))
            self.assertIn("example", s.get_version_info().get_module())
            self.assertEqual(len(s.get_inputs(m, [p])), 1)
            self.assertEqual(len(s.get_outputs(m, [p])), 1)

    def test_combine(self):
        """Test combining example SingletonModifier with IMP classes"""
        for typ in (IMP.example.ExampleSingletonModifier,
                    IMP.example.PythonExampleSingletonModifier):
            m = IMP.Model()
            bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                           IMP.algebra.Vector3D(10, 10, 10))
            p = m.add_particle("p")
            d = IMP.core.XYZ.setup_particle(m, p,
                                            IMP.algebra.Vector3D(-4, 13, 28))
            c = IMP.core.SingletonConstraint(typ(bb), None, m, p)
            m.add_score_state(c)
            m.update()
            self.assertLess(IMP.algebra.get_distance(d.get_coordinates(),
                                           IMP.algebra.Vector3D(6,3,8)), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of ExampleSingletonModifier"""
        m, p, d, s = make_modifier()
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        news.apply_index(m, p)
        self.assertLess(IMP.algebra.get_distance(
            d.get_coordinates(), IMP.algebra.Vector3D(6,3,8)), 1e-4)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of ExampleSingletonModifier via polymorphic ptr"""
        m, p, d, s = make_modifier()
        c = IMP.core.SingletonConstraint(s, None, m, p)
        dump = pickle.dumps(c)
        newc = pickle.loads(dump)
        newc.before_evaluate()
        self.assertLess(IMP.algebra.get_distance(
            d.get_coordinates(), IMP.algebra.Vector3D(6,3,8)), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
