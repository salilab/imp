import IMP
import IMP.test
import IMP.core
import pickle


class Tests(IMP.test.TestCase):

    def make_system(self):
        m = IMP.Model()
        p1 = m.add_particle("p1")
        xyz = IMP.core.XYZ.setup_particle(
            m, p1, IMP.algebra.Vector3D(1., 2., 3.))
        trans = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(10., 0., 0.))
        before = IMP.core.Transform(trans)
        r = IMP.core.SingletonConstraint(before, None, m, p1)
        return m, xyz, r

    def test_pickle(self):
        """Test (un-)pickle of SingletonConstraint"""
        m, xyz, r = self.make_system()
        r.before_evaluate()
        self.assertAlmostEqual(xyz.get_coordinates()[0], 11.0, delta=1e-5)
        r.set_name("foo")
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        newr.before_evaluate()
        self.assertAlmostEqual(xyz.get_coordinates()[0], 21.0, delta=1e-5)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SingletonConstraint via polymorphic pointer"""
        m, xyz, r = self.make_system()
        r.before_evaluate()
        self.assertAlmostEqual(xyz.get_coordinates()[0], 11.0, delta=1e-5)
        r.set_name("foo")
        m.score_states.append(r)
        dump = pickle.dumps(m)
        newm = pickle.loads(dump)
        newr, = newm.score_states
        self.assertEqual(newr.get_name(), "foo")
        newr.before_evaluate()
        self.assertAlmostEqual(xyz.get_coordinates()[0], 21.0, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
