import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.kinematics
import math

class Test(IMP.test.TestCase):
    """Tests for kinematic structure"""

    def create_rigidbody(self, model, coords):
        p = IMP.Particle(model)
        tr = IMP.algebra.Transformation3D( coords )
        rf = IMP.algebra.ReferenceFrame3D( tr )
        return IMP.core.RigidBody.setup_particle(p, rf)

    def create_model_with_rbs(self, coords_list):
        m= IMP.Model()
        rbs = []
        for coords in coords_list:
            rbs.append( self.create_rigidbody(m, coords) )
        return (m, rbs)

    def assert_rb_coords_good(self, kf, rbs, coords_list, places=5):
        """
        asserts that the coordinates of each rb are almost
        equal to the corresponding ones in coords, up to
        places precision, where coords is a list of vector3Ds
        kf - the kinematic forest used to control rbs
        """
        self.assertEqual( len(rbs), len(coords_list) )
        for (rb,coords) in zip(rbs, coords_list):
            rb_coords = kf.get_coordinates_safe(rb)
            d = IMP.algebra.get_distance( rb_coords, coords )
            self.assertAlmostEqual(d, 0.0, places)

    def test_two_prismatic_joints(self):
        """Checking prismatic angle joints"""
#        IMP.set_log_level(IMP.VERBOSE)
        (m, rbs) = self.create_model_with_rbs( [ [0,0,0], [0,0,1], [0,0,2] ] )
        # constucrt kinematic forest
        kf = IMP.kinematics.KinematicForest(m)
        kf.set_was_used(True)
        pj01 = IMP.kinematics.PrismaticJoint(rbs[0], rbs[1])
        pj12 = IMP.kinematics.PrismaticJoint(rbs[1], rbs[2])
        kf.add_edge(pj01)
        kf.add_edge(pj12)
        self.assertEqual(pj01.get_length(), 1.0)
        self.assertEqual(pj12.get_length(), 1.0)
        # play with joint
        kf.set_coordinates_safe(rbs[1],[0,0,0.5])
        self.assertEqual(pj01.get_length(), 0.5)
        self.assertEqual(pj12.get_length(), 1.5)
        pj01.set_length(10.0)
        self.assertEqual(pj01.get_length(), 10.0)
        self.assertEqual(pj12.get_length(), 1.5)
        d_rb2 = IMP.algebra.get_distance(kf.get_coordinates_safe(rbs[2]), [0,0,11.5])
        self.assertEqual( d_rb2, 0.0, 1e-12)

    def test_two_dihedral_joints(self):
        """Checking a pair of dihedral joints"""
#        IMP.set_log_level(IMP.VERBOSE)
        (m, rbs) = self.create_model_with_rbs( [ [0,0,0], [1,0,0], [1,1,0], [0,1,0], [0,2,0] ] )
        self.assertAlmostEqual(IMP.algebra.get_distance             \
                               (rbs[0].get_coordinates() ,
                                rbs[1].get_coordinates()) ,
                               1.0 )
        # Construct kinematic forest
        kf = IMP.kinematics.KinematicForest(m)
        kf.set_was_used(True)
        j01 = IMP.kinematics.Joint( rbs[0], rbs[1] )
        dj12 = IMP.kinematics.DihedralAngleRevoluteJoint( rbs[1], rbs[2],rbs[0],
                                                          rbs[1],rbs[2],rbs[3] )
        dj23 = IMP.kinematics.DihedralAngleRevoluteJoint( rbs[2], rbs[3],rbs[1],
                                                          rbs[2],rbs[3],rbs[4] )
        j34 = IMP.kinematics.Joint( rbs[3], rbs[4] )
        kf.add_edge( j01 )
        kf.add_edge( dj12 )
        kf.add_edge( dj23 )
        kf.add_edge( j34 )
        self.assertAlmostEqual( dj12.get_angle(), 0.0, 10)
        self.assertAlmostEqual( dj23.get_angle(), math.pi, 10)
        # Play with joint
        dj12.set_angle(math.pi)
        dj23.set_angle(0)
        right_coords = [ [0, 0, 0],
                         [1, 0, 0],
                         [1, 1, 0],
                         [2, 1, 0],
                         [2, 0, 0] ]
        self.assert_rb_coords_good( kf, rbs, right_coords )
        dj12.set_angle(0)
        dj23.set_angle(math.pi)
        right_coords = [ [0, 0, 0],
                         [1, 0, 0],
                         [1, 1, 0],
                         [0, 1, 0],
                         [0, 2, 0] ]
        self.assert_rb_coords_good( kf, rbs, right_coords )
        kf.set_coordinates_safe(rbs[1],[0,0,1.0])
        deg = math.pi / 180.0
        self.assertAlmostEqual( dj12.get_angle(), 60 * deg, 10 )
        self.assertAlmostEqual( dj23.get_angle(), 135 * deg, 10 )
        dj12.set_angle(0.0)
        dj23.set_angle(math.pi)
        right_coords = [ [0, 0, 0],
                         [0, 0, 1],
                         [1, 1, 0],
                         [0.333333, 0.333333, -0.333333],
                         [0.569036, 0.569036, -1.27614] ]
        self.assert_rb_coords_good( kf, rbs, right_coords )
        dj12.set_angle(math.pi)
        dj23.set_angle(0.0)
        right_coords = [ [0, 0, 0],
                         [0, 0, 1],
                         [1, 1, 0],
                         [1, 1, 1],
                         [0.292893, 0.292893, 1] ]
        self.assert_rb_coords_good( kf, rbs, right_coords )


    def test_composite_joint(self):
        """
        test a joint that composes a prismatic and dihedral joints
        """
#        IMP.set_log_level(IMP.VERBOSE)
        (m, rbs) = self.create_model_with_rbs( [ [0,0,0], [1,0,0],
                                                 [1,1,0], [0,1,0] ] )
        # Construct kinematic forest
        kf = IMP.kinematics.KinematicForest(m)
        kf.set_was_used(True)
        j01 = IMP.kinematics.Joint( rbs[0], rbs[1] )
        pj12 = IMP.kinematics.PrismaticJoint( rbs[1], rbs[2] )
        dj12 = IMP.kinematics.DihedralAngleRevoluteJoint( rbs[1], rbs[2],rbs[0],
                                                          rbs[1],rbs[2],rbs[3] )
        cj12 = IMP.kinematics.CompositeJoint ( rbs[1], rbs[2], [pj12, dj12] )
#, [pj12, dj12] )
        j23 = IMP.kinematics.Joint( rbs[2], rbs[3] )
        kf.add_edge( j01 )
        kf.add_edge( cj12 )
        kf.add_edge( j23 )
        self.assertAlmostEqual( pj12.get_length(), 1.0, 10)
        self.assertAlmostEqual( dj12.get_angle(), 0.0, 10)
        pj12.set_length(10.0);
        print "After set length(10.0):"
        for rb in rbs:
            print kf.get_coordinates_safe(rb)
        print "After set angle(90.0):"
        dj12.set_angle(math.pi/2)
        for rb in rbs:
            print kf.get_coordinates_safe(rb)


if __name__ == '__main__':
    IMP.test.main()
