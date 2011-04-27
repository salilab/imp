import IMP
import IMP.test
import IMP.bullet
import IMP.core
import IMP.algebra
import IMP.display
import StringIO
import math

class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def test_cleanup(self):
        """Test cleaning up after optimization"""
        m= IMP.Model()
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(5,5,5))
        ps=[]
        for i in range(0,30):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)
            ps.append(p)
            d.set_radius(2)
            print d
        r= IMP.core.ExcludedVolumeRestraint(IMP.container.ListSingletonContainer(ps))
        m.add_restraint(r)
        bo= IMP.bullet.ResolveCollisionsOptimizer(m)
        bo.optimize(1)
        self.assertEqual(len(IMP.get_restraints(m.get_root_restraint_set())), 1)


if __name__ == '__main__':
    IMP.test.main()
