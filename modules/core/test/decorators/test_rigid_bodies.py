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
        hd= IMP.core.HierarchyDecorator.create(rbp)
        xyzs= []
        for i in range(0, 4):
            mp= IMP.Particle(m)
            mxyz= IMP.core.XYZDecorator.create(mp,
                                               IMP.algebra.random_vector_in_unit_box())
            chd= IMP.core.HierarchyDecorator.create(mp)
            hd.add_child(chd)
            #IMP.algebra.Vector3D(i%2, (i+1)%3, i)
            xyzs.append(mxyz)
            mxyz.show()
            print
        pr= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        rbd= IMP.core.RigidBodyDecorator.create(rbp, pr)
        rbd.show()
        #r= IMP.algebra.Rotation3D(0.437645, -0.422742, -0.778777, 0.152519)
        r=IMP.algebra.random_rotation()
        print "center rotated is "
        r.rotate(rbd.get_coordinates()).show()
        print
        print "transformed"
        tvs= IMP.algebra.Vector3Ds()
        for d in xyzs:
            d.set_coordinates(r.rotate(d.get_coordinates()))
            d.show()
            print
            tvs.append(d.get_coordinates())
        rbd.set_transformation(pr)
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
