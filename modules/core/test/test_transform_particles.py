from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.test
import pickle


class Tests(IMP.test.TestCase):

    """Test particle transformations"""

    def test_transformation(self):
        """Test the TransformationFunction class"""
        imp_model = IMP.Model()
        particles = IMP.core.create_xyzr_particles(imp_model, 4, 1.0)

        coords = [x.get_coordinates() for x in particles]
        r = IMP.algebra.get_rotation_from_fixed_xyz(0.2, 0.8, -0.4)
        t = IMP.algebra.Transformation3D(
            r, IMP.algebra.Vector3D(20.0, -12.4, 18.6))
        print("create transform")
        tf = IMP.core.Transform(t)
        tf.set_was_used(True)
        for p in particles:
            print("applying to " + str(p))
            r = tf.apply_index(imp_model, p)
        for i in range(0, len(particles)):
            v = particles[i].get_coordinates()
            self.assertAlmostEqual(
                (v - t.get_transformed(coords[i])).get_magnitude(),
                0,
                delta=0.01)

    def make_system(self):
        m = IMP.Model()
        p1 = m.add_particle("p1")
        xyz = IMP.core.XYZ.setup_particle(
            m, p1, IMP.algebra.Vector3D(1., 2., 3.))
        trans = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(10., 0., 0.))
        t = IMP.core.Transform(trans)
        return m, xyz, t

    def test_pickle(self):
        """Test (un-)pickle of Transform"""
        m, xyz, t = self.make_system()
        t.apply_index(m, xyz)
        self.assertAlmostEqual(xyz.get_coordinates()[0], 11.0, delta=1e-5)
        t.set_name("foo")
        dump = pickle.dumps(t)
        newt = pickle.loads(dump)
        self.assertEqual(newt.get_name(), "foo")
        newt.apply_index(m, xyz)
        self.assertAlmostEqual(xyz.get_coordinates()[0], 21.0, delta=1e-5)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of Transform via polymorphic pointer"""
        m, xyz, t = self.make_system()
        r = IMP.core.SingletonConstraint(t, None, m, xyz)
        r.before_evaluate()
        self.assertAlmostEqual(xyz.get_coordinates()[0], 11.0, delta=1e-5)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        newr.before_evaluate()
        self.assertAlmostEqual(xyz.get_coordinates()[0], 21.0, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
