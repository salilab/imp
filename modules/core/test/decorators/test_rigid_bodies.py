import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra



class WLCTests(IMP.test.TestCase):
    """Tests for WLC unary function"""

    def test_rigid(self):
        """Test rigid bodies"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        rbp= IMP.Particle(m)
        rbxyz= IMP.core.XYZDecorator.create(rbp)
        ps= IMP.Particles()
        xyzs= []
        for i in range(0, 4):
            mp= IMP.Particle(m)
            ps.append(mp)
            mxyz= IMP.core.XYZDecorator.create(mp, IMP.Vector3D(0, 0, i))
            xyzs.append(mxyz)
        rbd= IMP.core.RigidBodyDecorator.create(rbp, ps)
        rbd.show()
        r= IMP.algebra.random_rotation()
        print "transformed"
        tvs= IMP.Vector3Ds()
        for d in xyzs:
            d.set_coordinates(r.rotate(d.get_coordinates()))
            d.show()
            print
            tvs.append(d.get_coordinates())
        rbd.set_transformation(ps)
        print "final coordinates"
        rbd.show()
        print "at end"
        for i in range(0, len(xyzs)):
            d= xyzs[i]
            v=tvs[i]
            d.show()
            print
            self.assertInTolerance((d.get_coordinates()-v).get_squared_magnitude(),
                                   0, .1)

if __name__ == '__main__':
    unittest.main()
