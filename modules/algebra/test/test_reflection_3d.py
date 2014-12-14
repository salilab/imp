import IMP
import IMP.test
import IMP.algebra
import io

class Tests(IMP.test.TestCase):
    def test_reflection3d(self):
        """Test Reflection3D class"""
        # Test trivial constructor
        r = IMP.algebra.Reflection3D()
        p = IMP.algebra.Plane3D(5.0, IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        r = IMP.algebra.Reflection3D(p)
        sio = io.StringIO()
        r.show(sio)
        v = r.get_reflected(IMP.algebra.Vector3D(20., 0., 0.))
        self.assertLess(IMP.algebra.get_distance(v,
                                       IMP.algebra.Vector3D(20,10,0)), 1e-4)

if __name__ == '__main__':
    IMP.test.main()
