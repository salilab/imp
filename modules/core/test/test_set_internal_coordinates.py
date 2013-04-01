import IMP
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Test the symmetry restraint"""
    def test_symmetry(self):
        """Test setting internal coordinates through float keys"""
        m= IMP.Model()
        p= IMP.Particle(m)
        rb=IMP.core.RigidBody.setup_particle(p, IMP.algebra.ReferenceFrame3D())
        pm= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(pm)
        rb.add_member(pm)

        keys=IMP.core.RigidMember.get_internal_coordinate_keys()
        print keys
        for k in keys:
            pm.set_is_optimized(k, True)
        mv= IMP.core.BallMover([pm],
                               keys, 1)

        mv.set_log_level(IMP.base.VERBOSE)
        print IMP.core.XYZ(pm).get_coordinates(),
        print IMP.core.RigidMember(pm).get_internal_coordinates()
        old = IMP.core.RigidMember(pm).get_internal_coordinates()
        mv.propose()
        print IMP.core.XYZ(pm).get_coordinates(),
        print IMP.core.RigidMember(pm).get_internal_coordinates()

        # update the rigid body
        m.update()
        new= IMP.core.RigidMember(pm).get_internal_coordinates()
        print IMP.core.XYZ(pm).get_coordinates(),
        print IMP.core.RigidMember(pm).get_internal_coordinates()
        self.assert_((new-old).get_magnitude() > 0)



if __name__ == '__main__':
    IMP.test.main()
