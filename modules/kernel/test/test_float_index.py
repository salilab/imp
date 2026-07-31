import IMP.test
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_float_index_float(self):
        """Test FloatIndex with Float attributes"""
        m = IMP.Model()
        xkey = IMP.FloatKey("x")
        p1 = IMP.Particle(m)
        m.add_attribute(xkey, p1, 42.0)

        fi = IMP.FloatIndex(p1, xkey)
        self.assertAlmostEqual(fi.get_value(m), 42.0, delta=1e-5)

        fi.set_value(m, 22.0)
        self.assertAlmostEqual(m.get_attribute(xkey, p1), 22.0, delta=1e-5)
        _ = fi.get_derivative(m)

    def test_float_index_vector3d(self):
        """Test FloatIndex with Vector3D attributes"""
        m = IMP.Model()
        xkey = IMP.Vector3DDerivKey("x")
        p1 = IMP.Particle(m)
        m.add_attribute(xkey, p1, IMP.algebra.Vector3D(20., 30., 40.))

        fi = IMP.FloatIndex(p1, xkey, 1)
        self.assertAlmostEqual(fi.get_value(m), 30.0, delta=1e-5)

        fi.set_value(m, 22.0)
        self.assertAlmostEqual(m.get_attribute(xkey, p1)[1], 22.0, delta=1e-5)
        _ = fi.get_derivative(m)

    def test_float_index_vector4d(self):
        """Test FloatIndex with Vector4D attributes"""
        m = IMP.Model()
        xkey = IMP.Vector4DDerivKey("x")
        p1 = IMP.Particle(m)
        m.add_attribute(xkey, p1, IMP.algebra.Vector4D(20., 30., 40., 50.0))

        fi = IMP.FloatIndex(p1, xkey, 1)
        self.assertAlmostEqual(fi.get_value(m), 30.0, delta=1e-5)

        fi.set_value(m, 22.0)
        self.assertAlmostEqual(m.get_attribute(xkey, p1)[1], 22.0, delta=1e-5)
        _ = fi.get_derivative(m)


if __name__ == '__main__':
    IMP.test.main()
