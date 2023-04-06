import IMP
import IMP.test
import IMP.algebra
import io
import pickle


class Tests(IMP.test.TestCase):
    def test_reflection3d(self):
        """Test Reflection3D class"""
        # Test trivial constructor
        r = IMP.algebra.Reflection3D()
        p = IMP.algebra.Plane3D(5.0, IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        r = IMP.algebra.Reflection3D(p)
        sio = io.BytesIO()
        r.show(sio)
        v = r.get_reflected(IMP.algebra.Vector3D(20., 0., 0.))
        self.assertLess(IMP.algebra.get_distance(
            v, IMP.algebra.Vector3D(20, 10, 0)), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of Reflection3D"""
        p1 = IMP.algebra.Plane3D(5.0, IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        r1 = IMP.algebra.Reflection3D(p1)
        p2 = IMP.algebra.Plane3D(3.0, IMP.algebra.Vector3D(1.0, 0.0, 0.0))
        r2 = IMP.algebra.Reflection3D(p2)
        r2.foo = 'bar'
        rdump = pickle.dumps((r1, r2))

        newr1, newr2 = pickle.loads(rdump)
        v = IMP.algebra.Vector3D(40., 30., 20.)
        self.assertLess(
            IMP.algebra.get_distance(r1.get_reflected(v),
                                     newr1.get_reflected(v)), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(r2.get_reflected(v),
                                     newr2.get_reflected(v)), 1e-4)
        self.assertEqual(newr2.foo, 'bar')


if __name__ == '__main__':
    IMP.test.main()
